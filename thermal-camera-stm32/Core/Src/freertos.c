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
#include "MLX90640_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_OF_FRAMES_TO_BUFFER 8
#define BYTES_IN_HALFWORD 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern UART_HandleTypeDef huart2;

uint8_t cam_frames[FRAME_ROWS * FRAME_COLS * BYTES_IN_HALFWORD];
uint8_t gotData = 0;
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
		gotData = 1;
	}
}

static uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8 );
}

void StartDefaultTask(void *argument)
{
	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, mlx90640Address << 1, 100, osWaitForever);
	printf ("Device Ready status: %d\r\n", status);

	uint8_t refreshRate[2] = {0};
	HAL_I2C_Mem_Read(&hi2c1, mlx90640Address << 1, 0x800D, I2C_MEMADD_SIZE_16BIT, refreshRate, 2, 3000);
	//HAL_I2C_Master_Receive_DMA(&hi2c1, DEVICE_ADDR << 1, cam_frames, FRAME_ROWS * FRAME_COLS * 2);
	//status = HAL_I2C_Mem_Read_DMA(&hi2c1, mlx90640Address << 1, 0x2440, I2C_MEMADD_SIZE_16BIT, cam_frames, NUM_OF_PIXELS);
	//printf ("Master Receive status: %d\r\n", status);

	printf ("Refresh rate from 0x800D: 0x%X\r\n", swap_uint16(*(uint16_t*)refreshRate));

	setRefreshRate(MLX_16_HZ);

	osDelay(100);

	HAL_I2C_Mem_Read(&hi2c1, mlx90640Address << 1, 0x800D, I2C_MEMADD_SIZE_16BIT, refreshRate, 2, 3000);
	printf ("Refresh rate from 0x800D: 0x%X\r\n", swap_uint16(*(uint16_t*)refreshRate));

	int i = 0;
	for (;;)
	{
		printf ("test\r\n");
		if (gotData == 1)
		{
			int16_t *ptr = (int16_t*)cam_frames;
			for (i = 0; i < FRAME_ROWS * FRAME_COLS; i++)
			{
				if ((i + 1) % FRAME_COLS == 0)
				{
					printf ("\r\n");
				}

				printf ("%d ", ptr[i]);

			}
			gotData = 0;
		}
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

