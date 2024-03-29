/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ADXL345.h"
#include "MS5611.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define ACC_l 2.5  // kalkisi anlamak icin gerekli ivme eşigi (pozitif olmali)***

float ADXL_Z[6] = {0}, MS_V[6] = {0}, MS_A[6] = {0};
float temp, press, MS[2];
uint8_t ADXL = 0, ADXL_S = 0, MS5611 = 0, MS_S = 0, ACC_FAIL = 0, final = 0, k = 0;
uint32_t tim1 = 0, tim2 = 0, dif = 0, alt_l = 0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  Scan_I2C();
  ADXL345_Init();
  MS5611_Init();
  //HAL_TIM_Base_Start(&htim3);
  enum rocket {Rail, Launch, Burnout, Apogee, Descent, Main, Recovery};
  enum rocket EPHEMERISH;


  for (uint8_t i = 0 ; i < 12 ; i++)
    {
  	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
  	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
  	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
  	  HAL_Delay(50);
    }

	if (ACC_FAIL == 0) {
		EPHEMERISH = Rail;
		HAL_TIM_Base_Start_IT(&htim2);
		//EPHEMERISH = Burnout;
	} else if (ACC_FAIL == 1) {
		EPHEMERISH = Launch;
		HAL_TIM_Base_Stop_IT(&htim2);
		HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start(&htim4);
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		switch (EPHEMERISH) {
		case Rail:
			if (ADXL == 1) {
				ADXL_Z[ADXL_S] = ADXL345_Zaxiss_filter();
				if (ADXL_S == 5) {
					uint8_t C2 = 0;
					for (uint8_t i = 0; i <= ADXL_S; i++) {
						if (ADXL_Z[i] < (-1 * ACC_l) || ADXL_Z[i] > ACC_l)
							C2++;
					}
					if (C2 >= 3) {
						EPHEMERISH = Launch;
						//EPHEMERISH = Apogee;
						HAL_TIM_Base_Stop_IT(&htim2);
						for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
							HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
							HAL_Delay(50);
						}
						HAL_TIM_Base_Start_IT(&htim3);
						HAL_TIM_Base_Start(&htim4);
					}
					ADXL_S = 0;
				}
				ADXL_S++;
				ADXL = 0;
			}
			break;
		case Launch:
			if (MS5611 == 1) {
				if (alt_l == 0) {
					tim1 = __HAL_TIM_GET_COUNTER(&htim4);
					MS[0] = MS5611_ReadMedian_Altitude();
					alt_l = 1;
				} else if (alt_l == 1) {
					tim2 = __HAL_TIM_GET_COUNTER(&htim4);
					MS[1] = MS5611_ReadMedian_Altitude();
					if (tim2 < tim1) {
						tim2 = tim2 + 65535;
						dif = tim2 - tim1;
					} else
						dif = tim2 - tim1;
					MS_V[MS_S] = (MS[1] - MS[0]) / (0.001 * dif);
					if (MS_S == 5) {
						uint8_t M = 0;
						for (uint8_t i = 0; i <= MS_S; i++) {
							if (MS_V[i] > 15)  // TEST BURNOUT VALUE = 15 REAL BURNOUT VALUE = 100
								M++;
						}
						if (M >= 3) {
							//**********************
							EPHEMERISH = Burnout;
							for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
								HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
								HAL_Delay(50);
							}
						}
						for (uint8_t k = 0; k <= 5; k++)
							MS_V[k] = 0;
						MS_S = 0;
					}
					MS_S++;
					alt_l = 0;
				}
				MS5611 = 0;
			}
			break;
		case Burnout:
			if (MS5611 == 1) {
				if (alt_l == 0) {
					tim1 = __HAL_TIM_GET_COUNTER(&htim4);
					MS[0] = MS5611_ReadMedian_Altitude();
					alt_l = 1;
				} else if (alt_l == 1) {
					tim2 = __HAL_TIM_GET_COUNTER(&htim4);
					MS[1] = MS5611_ReadMedian_Altitude();
					if (tim2 < tim1) {
						tim2 = tim2 + 65535;
						dif = tim2 - tim1;
					} else
						dif = tim2 - tim1;
					MS_V[MS_S] = (MS[1] - MS[0]) / (0.001 * dif);
					if (MS_S == 5) {
						uint8_t M = 0;
						for (uint8_t i = 0; i <= MS_S; i++) {
							if (MS_V[i] < 0)   // APOGEE velocity limit
								M++;
						}
						if (M >= 3) {
							EPHEMERISH = Apogee;
							HAL_TIM_Base_Stop_IT(&htim3);
							for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
								HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
								HAL_Delay(50);
							}
						}
						for (uint8_t k = 0 ; k <= 5 ; k++)
							MS_V[k] = 0;
						MS_S = 0;
					}
					MS_S++;
					alt_l = 0;
				}
				MS5611 = 0;
			}
			break;
		case Apogee:
			//EPHEMERISH = Recovery;
			EPHEMERISH = Descent;
			for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
				HAL_Delay(50);
			}
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, SET);  // ***************
			HAL_Delay(600);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, RESET);
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, RESET);  // **************
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
			HAL_TIM_Base_Start_IT(&htim3);
			break;
		case Descent:
			if (MS5611 == 1) {
				MS_A[MS_S] = MS5611_ReadMedian_Altitude();
				if (MS_S == 5) {
					uint8_t M = 0;
					for (uint8_t i = 0; i <= MS_S; i++) {
						if (MS_A[i] < 500)
							M++;
					}
					if (M >= 3) {
						// MAIN FIRE
						EPHEMERISH = Main;
						HAL_TIM_Base_Stop_IT(&htim3);
						for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
							HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
							HAL_Delay(50);
						}
					}
					for (uint8_t k = 0 ; k <= 5 ; k++)
						MS_A[k] = 0;
					MS_S = 0;
				}
				MS_S++;
				MS5611 = 0;
			}
			break;
		case Main:
			EPHEMERISH = Recovery;
			for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
				HAL_Delay(50);
			}
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, SET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, SET);
			HAL_Delay(600);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, RESET);
			break;
		case Recovery:
			for (uint8_t i = 0; i < (EPHEMERISH * 2); i++) {
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
				HAL_Delay(500);
				final++;
			}
			break;
		}
	}
/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 47999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 108;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 47999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BUZZER_Pin|BOTTOM_LED_Pin|APOGEE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MAIN_Pin|TOP_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUZZER_Pin BOTTOM_LED_Pin APOGEE_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin|BOTTOM_LED_Pin|APOGEE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MAIN_Pin TOP_LED_Pin */
  GPIO_InitStruct.Pin = MAIN_Pin|TOP_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void Scan_I2C(void){
	for(uint8_t i = 0; i < 255 ; i++){
		if(HAL_I2C_IsDeviceReady(&hi2c1, i, 1, 10) == HAL_OK){
			k++;
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
