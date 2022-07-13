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
#include "MLX90640_I2C_Driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MLX90640_ADDR (0x33)
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
	paramsMLX90640 mlxParams;
	uint16_t mlx90640Data[834] = {0};
	float image[768] = {0};

	float tr = 0;
	float emissivity = 0.95;

	uint16_t i = 0;
	int16_t status = -1;

	MLX90640_I2CInit(&hi2c1);

	MLX90640_SetRefreshRate(MLX90640_ADDR, MLX_16_HZ);

	MLX90640_DumpEE(MLX90640_ADDR, mlx90640Data);
	MLX90640_ExtractParameters(mlx90640Data, &mlxParams);


	osDelay(500);


	printf("Broken pixels: \r\n");
	for (i = 0; i < 5; i++)
	{
		printf("0x%X ", mlxParams.brokenPixels[i]);
	}

	printf("\r\n");
	printf("Outlier pixels: \r\n");
	for (i = 0; i < 5; i++)
	{
		printf("0x%X ", mlxParams.outlierPixels[i]);
	}

	printf("\r\n");

	for (;;)
	{
		status = MLX90640_SynchFrame(MLX90640_ADDR);
		if (status != 0)
		{
			printf("SynchFrame failed\r\n");
			osDelay(1000);
			continue;
		}

		MLX90640_GetFrameData(MLX90640_ADDR, mlx90640Data);

		tr = MLX90640_GetTa(mlx90640Data, &mlxParams) - 8;

		printf("Ambient temp: %.2f\r\n", tr);

		MLX90640_CalculateTo(mlx90640Data, &mlxParams, emissivity, tr, image);

		MLX90640_BadPixelsCorrection(mlxParams.brokenPixels, image, 1, &mlxParams);
		MLX90640_BadPixelsCorrection(mlxParams.outlierPixels, image, 1, &mlxParams);

		for (i = 0; i < 768; i++)
		{
			printf("%4.1f ", image[i]);

			if ((i+1) % 32 == 0)
				printf("\r\n");
		}
		printf("\r\n\r\n");
		osDelay(3000);
	}
}

/* USER CODE END Application */

