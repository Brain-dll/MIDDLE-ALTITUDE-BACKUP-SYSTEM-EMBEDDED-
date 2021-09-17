/*
 * ADXL345.h
 *
 *  Created on: Apr 24, 2021
 *      Author: Osman ÇİÇEK
 */

#ifndef INC_ADXL345_H_
#define INC_ADXL345_H_

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define ADXL345_WRITE_ADRESS 0xA6
#define ADXL345_READ_ADRESS 0xA7
/*
#define ADXL345_ID 0x00
#define ADXL_OFFSET_ADD 0x20
*/

#define ADXL345_TIMEOUT 1000

void ADXL345_Init(void);
void ADXL345_SetCalibration(void);
void ADXL345_GetUnCompanseted_Accelerations(void);
float ADXL345_GetXaxis(void);
float ADXL345_GetYaxis(void);
float ADXL345_GetZaxis(void);
float Array_sort_ADXL(float *array, int n);
float ADXL345_Zaxiss_filter(void);


#endif /* INC_ADXL345_H_ */
