/*
 * ADXL345.h
 *
 *  Created on: Apr 24, 2021
 *      Author: Osman ÇİÇEK
 */

#include "ADXL345.h"

uint8_t ADXL345_ID = 0x00;
uint8_t ADXL_OFFSET_ADD = 0x20;
uint8_t ID_RESPONS = 0x00;
uint8_t POWER_CTL = 0x2D;
uint8_t POWER_RESET = 0x00;
uint8_t POWER_SET = 0x08;

uint8_t DATA_FORMAT = 0x31;
uint8_t DATA_FORMAT_RATE = 0x03;

uint8_t ADXL_DATA[6] = {0};
uint8_t ADXL_DATA_ADRESS = 0x32;
uint8_t ADXL_DATA_ADRESS_LENGTH = 6;
uint8_t ADXL_OFFSET = 0x01;
uint8_t control = 0x00;

int16_t Ax, Ay, Az;

extern uint8_t ACC_FAIL;

void ADXL345_Init(void)
{
	HAL_I2C_Mem_Read(&hi2c1, ADXL345_READ_ADRESS, ADXL345_ID, 1, &ID_RESPONS, 1, ADXL345_TIMEOUT);
	ACC_FAIL = 0;
	if(HAL_I2C_IsDeviceReady(&hi2c1, ADXL345_WRITE_ADRESS, 1, ADXL345_TIMEOUT) != HAL_OK  && ID_RESPONS != 0xE5)
	{
		for (uint8_t i = 0 ; i < 4 ; i++)
		    {
		  	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
		  	  HAL_Delay(500);
		    }
		ACC_FAIL = 1;
	}
	ADXL345_SetCalibration();
}

void ADXL345_SetCalibration(void)
{
	// RESET POWER CONTROL REGISTER
	HAL_I2C_Mem_Write(&hi2c1, ADXL345_WRITE_ADRESS, POWER_CTL, 1, &POWER_RESET, 1, ADXL345_TIMEOUT);

	HAL_I2C_Mem_Read(&hi2c1, ADXL345_WRITE_ADRESS, POWER_CTL, 1, &control, 1, ADXL345_TIMEOUT);

	// MEASUREMENT MODE
	HAL_I2C_Mem_Write(&hi2c1, ADXL345_WRITE_ADRESS, POWER_CTL, 1, &POWER_SET, 1, ADXL345_TIMEOUT);

	HAL_I2C_Mem_Read(&hi2c1, ADXL345_WRITE_ADRESS, POWER_CTL, 1, &control, 1, ADXL345_TIMEOUT);

	// G RANGE SELECTION
	HAL_I2C_Mem_Write(&hi2c1, ADXL345_WRITE_ADRESS, DATA_FORMAT, 1, &DATA_FORMAT_RATE, 1, ADXL345_TIMEOUT);

	HAL_I2C_Mem_Read(&hi2c1, ADXL345_WRITE_ADRESS, DATA_FORMAT, 1, &control, 1, ADXL345_TIMEOUT);

	//HAL_I2C_Mem_Write(&hi2c1, ADXL345_WRITE_ADRESS, ADXL_OFFSET_ADD, 1, &ADXL_OFFSET, 1, ADXL345_TIMEOUT);
}

void ADXL345_GetUnCompanseted_Accelerations(void)
{
	HAL_I2C_Mem_Read(&hi2c1, ADXL345_READ_ADRESS, ADXL_DATA_ADRESS, 1, ADXL_DATA, ADXL_DATA_ADRESS_LENGTH, ADXL345_TIMEOUT);

	Ax = /*(int16_t)*/(ADXL_DATA[1] << 8) | ADXL_DATA[0];//  x values in Data[1] and Data[0]
	Ay = /*(int16_t)*/(ADXL_DATA[3] << 8) | ADXL_DATA[2];//  y values in Data[3] and Data[2]
	Az = /*(int16_t)*/(ADXL_DATA[5] << 8) | ADXL_DATA[4];//  z values in Data[5] and Data[4]
}

float ADXL345_GetXaxis(void)
{
	ADXL345_GetUnCompanseted_Accelerations();
	return Ax * .0312;
}
float ADXL345_GetYaxis(void)
{
	ADXL345_GetUnCompanseted_Accelerations();
	return Ay * .0312;
}
float ADXL345_GetZaxis(void)
{
	ADXL345_GetUnCompanseted_Accelerations();
	return Az * .0312;
}
float ADXL345_Zaxiss_filter(void)
{
	float Zf[5] = {0};
		for (uint8_t i = 0 ; i < 5 ; i++)
			Zf[i] = ADXL345_GetZaxis();
		return Array_sort_ADXL(Zf,5);
}
float Array_sort_ADXL(float *array, int n) {
	int i = 0, j = 0;
	float temp = 0.0;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n - 1; j++) {
			if (array[j] > array[j + 1]) {
				temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
			}
		}
	}
	//return array[2];
	return array[n / 2];
}






