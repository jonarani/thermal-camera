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
#include <stdbool.h>
#include <memory.h>
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
extern UART_HandleTypeDef huart1;

static uint8_t espBuf[50] = {0};
static bool received = false;

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

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	__HAL_UART_CLEAR_OREFLAG(&huart1);
	printf("Callback\r\n");
	uint16_t i = 0;
	for (i = 0; i < Size; i++)
	{
		printf ("%d ", espBuf[i]);
	}
	printf ("\r\n");
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, espBuf, 50);
}

void StartCamTask(void *argument)
{
	paramsMLX90640 mlxParams;
	uint16_t mlx90640Data[834] = {0};

	float image[768] = {0};
	//uint8_t crAndLf[2] = {'\r', '\n'};
	//memcpy((uint8_t*)&image[768], crAndLf, sizeof(crAndLf));

	float tr = 0;
	float emissivity = 0.95;

	uint16_t i = 0;
	int16_t status = -1;

	MLX90640_I2CInit(&hi2c1);

	MLX90640_SetRefreshRate(MLX90640_ADDR, MLX_16_HZ);

	MLX90640_DumpEE(MLX90640_ADDR, mlx90640Data);
	MLX90640_ExtractParameters(mlx90640Data, &mlxParams);

	osDelay(500);

//	printf("Broken pixels: \r\n");
//	for (i = 0; i < 5; i++)
//	{
//		printf("0x%X ", mlxParams.brokenPixels[i]);
//	}
//
//	printf("\r\n");
//	printf("Outlier pixels: \r\n");
//	for (i = 0; i < 5; i++)
//	{
//		printf("0x%X ", mlxParams.outlierPixels[i]);
//	}
//
//	printf("\r\n");

	uint32_t sampledFrames = 0;
	for (;;)
	{
		uint32_t start = osKernelGetTickCount();
		status = MLX90640_SynchFrame(MLX90640_ADDR);
		if (status != 0)
		{
			printf("SynchFrame failed\r\n");
			osDelay(20);
			continue;
		}

		MLX90640_GetFrameData(MLX90640_ADDR, mlx90640Data);

		tr = MLX90640_GetTa(mlx90640Data, &mlxParams) - 8;

		MLX90640_CalculateTo(mlx90640Data, &mlxParams, emissivity, tr, image);

//		MLX90640_BadPixelsCorrection(mlxParams.brokenPixels, image, 1, &mlxParams);
//		MLX90640_BadPixelsCorrection(mlxParams.outlierPixels, image, 1, &mlxParams);

		if ((sampledFrames + 1) % 2 == 0)
		{
			HAL_UART_Transmit(&huart1, (uint8_t*)&image, sizeof(image), osWaitForever);
			//printf("Sent\r\n");
		}
		sampledFrames++;

		uint32_t end = osKernelGetTickCount();
		printf ("%lu\r\n", end - start);

	}
}

//void StartWifiTask(void *argument)
//{
//	uint8_t test[5] = {'A', 'T', '\r', '\n'};
//	float num = 7.7;
//	float otherNum = 123.45;
//	char cmd[] = "AT\r\n";
//
//	osDelay(1000);
//	uint16_t rxLen;
//	uint16_t i = 0;
//
//	printf("sizeof f: %d\r\n", sizeof(num));
//
//	float floatTest[3];
//	floatTest[0] = num;
//	floatTest[1] = otherNum;
//
//	uint8_t crAndLf[2] = {'\r', '\n'};
//	memcpy((uint8_t*)&floatTest[2], crAndLf, sizeof(crAndLf));
//
//	printf("size of floatTest: %d\r\n", sizeof(floatTest));
//	printf("floatTest[0]: %.2f\r\n", floatTest[0]);
//	printf("floatTest[0]: %.2f\r\n", floatTest[1]);
//
//	uint8_t *p = &floatTest[2];
//	for (i = 0; i < 4; i++)
//	{
//		printf ("%d ", *p++);
//	}
//	printf("\r\n");
//
//	osDelay(50);
//
//	//__HAL_UART_CLEAR_OREFLAG(&huart1);
//	//HAL_UARTEx_ReceiveToIdle_IT(&huart1, espBuf, 50);
//
////
////	printf("Made it!\r\n");
//
//	for (;;)
//	{
//		//HAL_UART_Transmit(&huart1, (uint8_t*)&floatTest, sizeof(floatTest) - 2, 5000);
//		//HAL_UARTEx_ReceiveToIdle(&huart1, espBuf, 10, &rxLen, osWaitForever);
////		for (i = 0; i < rxLen; i++)
////		{
////			printf ("%d ", espBuf[i]);
////		}
////		printf("\r\n");
//		//printf("Transmitted!\r\n");
//		printf("test\r\n");
//		osThreadYield();
//		osDelay(1000);
//	}
//}

/* USER CODE END Application */

