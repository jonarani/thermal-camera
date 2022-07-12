/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include <stdio.h>
#include "MLX90640_API.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern UART_HandleTypeDef huart2;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
				if (hi2c == &hi2c1)
				{
								printf("data\r\n");
								HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				}
}

void StartDefaultTask(void *argument)
{
//				HAL_I2C_Mem_Read(&hi2c1, mlx90640Address << 1, 0x800D, I2C_MEMADD_SIZE_16BIT, refreshRate, 2, 3000);
//				printf ("Refresh rate from 0x800D: 0x%X\r\n", swap_uint16(*(uint16_t*)refreshRate));

				// for testing
				MLX90640_GetFrameData(1, NULL);
				for (;;)
				{
								printf ("test\r\n");
								osDelay(10000);
				}
}

void StartCameraTask(void *argument)
{
				for (;;)
				{
								osDelay(10000);
				}
}

/* USER CODE END Application */

