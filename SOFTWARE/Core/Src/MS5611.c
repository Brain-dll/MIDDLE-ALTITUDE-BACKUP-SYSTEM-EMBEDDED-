/*
 * MS5611.c
 *
 *  Created on: Jun 22, 2021
 *      Author: BRAIN.DLL
 */
#include "MS5611.h"
#include "math.h"
#include "stdio.h"

float BASE_P[3] = {0}, BASE = 0;

uint8_t fab_data[16] = {0};
uint8_t ADC[6] = {0};
uint16_t C[6] = {0};
uint32_t D1 = 0, D2 = 0;

uint8_t D1_4096 = 0x48;
uint8_t D2_4096 = 0x58;
uint8_t Read_ADC = 0x00;
uint8_t Read_PROMFAB = 0xA0;
uint8_t Read_PROMC1 = 0xA2;
uint8_t Read_PROMC2 = 0xA4;
uint8_t Read_PROMC3 = 0xA6;
uint8_t Read_PROMC4 = 0xA8;
uint8_t Read_PROMC5 = 0xAA;
uint8_t Read_PROMC6 = 0xAB;
uint8_t Read_PROMCRC = 0xAD;

int32_t dT = 0, dT2 = 0;
int64_t OFF = 0, SENS = 0, OFF2 = 0, SENS2 = 0;
float TEMP = 0.0, PRESS = 0.0, ALT = 0.0;
float P0 = 1013.25;		// sea level air pressure
//float P0 = 1012.0;	// ADANA air pressure
float T = 0;
float* pte = &T;

void MS5611_Init(void) {

	if (HAL_I2C_IsDeviceReady(&hi2c1, MS5611_WRITE_ADRESS, 1, 100) != HAL_OK) {
		for (uint8_t i = 0; i < 6; i++) {
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
			HAL_Delay(500);
		}
	}

	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMFAB, 1,
			1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[0], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC1, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[2], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC2, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[4], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC3, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[6], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC4, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[8], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC5, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[10], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMC6, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[12], 2, 1000);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_PROMCRC, 1,
			1000);
	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &fab_data[14], 2, 1000);

	for (uint8_t x = 2; x < 14; x += 2) {
		C[(x / 2) - 1] = fab_data[x] << 8 | fab_data[x + 1];
	}

	BASE = 0;
	for(uint8_t i = 0 ; i < sizeof(BASE_P) / sizeof(BASE_P[0]) ; i++)
	{
		BASE_P[i] = MS5611_ReadMedian_Altitude();
	}
	BASE = Array_sort_MS(BASE_P, sizeof(BASE_P) / sizeof(BASE_P[0]));
}

float MS5611_ReadTemperature(void)
{
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &D2_4096, 1, 1000);
	HAL_Delay(9);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_ADC, 1, 1000);

	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, ADC, 3, 1000);
	D2 = ADC[0] << 16 | ADC[1] << 8 | ADC[2];

	dT = D2 - C[4] * pow(2, 8);
	TEMP = (2000 + dT * C[5] / pow(2, 23)) / 100.0;
	return TEMP;
}

float MS5611_ReadPressure(void) {
	/*
	 HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &D2_4096, 1, 1000);
	 HAL_Delay(9);
	 HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_ADC, 1, 1000);

	 HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, ADC, 3, 1000);
	 D2 = ADC[0] << 16 | ADC[1] << 8 | ADC[2];

	 dT = D2 - ((int32_t) C[4] << 8);
	 float T = (2000 + ((int32_t) (dT * C[5]) >> 23)) / 100.0;
	 */

	T = MS5611_ReadTemperature();

	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &D1_4096, 1, 1000);
	HAL_Delay(9);
	HAL_I2C_Master_Transmit(&hi2c1, MS5611_WRITE_ADRESS, &Read_ADC, 1, 1000);

	HAL_I2C_Master_Receive(&hi2c1, MS5611_READ_ADRESS, &ADC[3], 3, 1000);
	D1 = ADC[3] << 16 | ADC[4] << 8 | ADC[5];

	if (T < 20 && T > -15) {
		dT2 = (dT * dT) >> 31;
		OFF2 = 5 * (T - 2000) * (T - 2000) / 2;
		SENS2 = 5 * (T - 2000) * (T - 2000) / 4;
	}
	OFF = (((int64_t) (C[1])) << 16) + ((C[3] * dT) >> 7);
	SENS = (((int64_t) (C[0])) << 15) + ((C[2] * dT) >> 8);
	PRESS = ((((D1 * SENS) >> 21) - OFF) >> 15) / 100.0;
	/*
	 dT = D2 - C[4] * pow(2, 8);
	 OFF = C[1] * pow(2, 16) + (C[3] * dT) / pow(2, 7);
	 SENS = C[0] * pow(2, 15) + (C[2] * dT) / pow(2, 8);

	 TEMP = (2000 + dT * C[5] / pow(2, 23)) / 100.0;

	 if (TEMP < 20) {
	 dT2 = pow(dT, 2) / pow(2, 31);
	 OFF2 = 5 * pow((TEMP - 2000), 2) / 2;
	 SENS2 = 5 * pow((TEMP - 2000), 2) / 4;
	 if (TEMP < -15) {
	 OFF2 = OFF2 + 7 * pow((TEMP + 1500), 2);
	 SENS2 = SENS2 + 11 * pow((TEMP + 1500), 2) / 2;
	 }
	 } else {
	 dT2 = 0;
	 OFF2 = 0;
	 SENS2 = 0;
	 }
	 TEMP = TEMP - dT2;
	 OFF = OFF - OFF2;
	 SENS = SENS - SENS2;


	 PRESS = ((D1 * SENS / pow(2, 21) - OFF) / pow(2, 15)) / 100.0;
	 */
	return PRESS;
}

float MS5611_ReadAltitude(void)
{
	float P = MS5611_ReadPressure();
	//float T0 = MS5611_ReadTemperature();
	//ALT = (float) (44330.0 * (1 - pow((PRESS / P0), (1 / 5.255))));
	ALT = ((pow((P0/P), (1/5.257))-1) * (*pte + 273.15)) / 0.0065;  // Altitude with temperature and pressure
	//ALT = (pow(10,log(PRESS/P0)/5.2558797)-1) / (-6.8755856 * pow(10,-6));
	return (ALT - BASE);
}

float MS5611_ReadMedian_Altitude(void)
{
	float H[5] = {0};
	for (uint8_t i = 0 ; i < 5 ; i++)
		H[i] = MS5611_ReadAltitude();
	return Array_sort_MS(H,5);
}

float Array_sort_MS(float *array, int n) {
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




