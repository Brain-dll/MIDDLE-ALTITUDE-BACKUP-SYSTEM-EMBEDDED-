/*
 * MS5611.h
 *
 *  Created on: Jun 22, 2021
 *      Author: BRAIN.DLL
 */

#ifndef INC_MS5611_H_
#define INC_MS5611_H_

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define MS5611_WRITE_ADRESS 0xEE
#define MS5611_READ_ADRESS 0xEF


void MS5611_Init(void);
float MS5611_ReadTemperature(void);
float MS5611_ReadPressure(void);
float MS5611_ReadAltitude(void);
float MS5611_ReadMedian_Altitude(void);
float Array_sort_MS(float *array, int n);

#endif /* INC_MS5611_H_ */
