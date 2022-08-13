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
#include <string.h>
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

extern osThreadId_t EspTxTaskHandle;

static uint8_t receiveBuf[150] = {0};
static char expectedStr[50] = {0};


typedef enum
{
	ESP_INIT = 0,
	RECEIVED_EXPECTED,
	RECEIVED_UNEXPECTED,
	BUSY_PROCESSING,
	ESP_ERROR,
	ESP_UNKNOWN,

} EspRxStatus;

static EspRxStatus espRxStatus = ESP_INIT;

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
	if (huart == &huart1)
	{
		__HAL_UART_CLEAR_OREFLAG(&huart1);
		receiveBuf[Size] = 0;

		if (strstr((char *)receiveBuf, expectedStr))
		{
			espRxStatus = RECEIVED_EXPECTED;
		}
		else if (strstr((char *)receiveBuf, "ERROR"))
		{
			espRxStatus = ESP_ERROR;
		}
		else if (strstr((char *)receiveBuf, "busy p.."))
		{
			espRxStatus = BUSY_PROCESSING;
		}
		else
		{
			espRxStatus = RECEIVED_UNEXPECTED;
			HAL_UARTEx_ReceiveToIdle_IT(&huart1, &receiveBuf[0], 50);
		}

		printf("-- %d ", Size);
		uint16_t i = 0;
		for (i = 0; receiveBuf[i] != 0; i++)
		{
			printf ("%c", receiveBuf[i]);
		}
		printf ("--\r\n");

	}
}

void StartCamTask(void *argument)
{
	paramsMLX90640 mlxParams;
	uint16_t mlx90640Data[834] = {0};

	float image[770] = {0};
	uint8_t crAndLf[2] = {'\r', '\n'};
	memcpy((uint8_t*)&image[768], crAndLf, sizeof(crAndLf));


	float tr = 0;
	float emissivity = 0.95;

	int16_t status = -1;

	MLX90640_I2CInit(&hi2c1);
//
	MLX90640_SetRefreshRate(MLX90640_ADDR, MLX_16_HZ);
//
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
		printf ("Test2\r\n");
//		uint32_t start = osKernelGetTickCount();
		status = MLX90640_SynchFrame(MLX90640_ADDR);
		if (status != 0)
		{
			printf("SynchFrame failed\r\n");
			osDelay(1000);
			continue;
		}

		MLX90640_GetFrameData(MLX90640_ADDR, mlx90640Data);
//
		tr = MLX90640_GetTa(mlx90640Data, &mlxParams) - 8;
//
		MLX90640_CalculateTo(mlx90640Data, &mlxParams, emissivity, tr, image);

		// Not needed at the moment
//		MLX90640_BadPixelsCorrection(mlxParams.brokenPixels, image, 1, &mlxParams);
//		MLX90640_BadPixelsCorrection(mlxParams.outlierPixels, image, 1, &mlxParams);

		if ((sampledFrames + 1) % 2 == 0)
		{
			//HAL_UART_Transmit(&huart1, (uint8_t*)&image, sizeof(image), osWaitForever);
			//printf ("%.2f %.2f %.2f %.2f\r\n", tr, image[100], image[200], image[300]);
		}
		sampledFrames++;

		osDelay(5000);
//
//		uint32_t end = osKernelGetTickCount();
//		printf ("%lu\r\n", end - start);
	}
}


static void sendAtAndWaitForResponse(const char *command, const char *exptectedMsg)
{
	__HAL_UART_CLEAR_OREFLAG(&huart1);

	espRxStatus = ESP_INIT;
	printf ("Command: %s\r\n", command);
	strcpy(expectedStr, exptectedMsg);
	printf ("expectedStr: %s\r\n", expectedStr);

	HAL_UARTEx_ReceiveToIdle_IT(&huart1, receiveBuf, 50);
	HAL_UART_Transmit(&huart1, (uint8_t*)command, strlen(command), 5000);

	while (espRxStatus != RECEIVED_EXPECTED)
	{
		if (espRxStatus == BUSY_PROCESSING)
		{
			osDelay(1000);
			espRxStatus = ESP_UNKNOWN;
			HAL_UARTEx_ReceiveToIdle_IT(&huart1, receiveBuf, 50);
			HAL_UART_Transmit(&huart1, (uint8_t*)command, strlen(command), 5000);
		}
		else if (espRxStatus == ESP_ERROR || espRxStatus == RECEIVED_UNEXPECTED)
		{
			printf ("Error looping: %u\r\n", espRxStatus);
			osDelay(2000);
		}
	}
}

void StartEspTxTask(void *argument)
{
	// Test command
	const char AT_test[] = "AT\r\n";
	// Switch echo off
	const char AT_echo[] = "ATE0\r\n";
	// Query Wifi state and info
	const char AT_query[] = "AT+CWMODE?\r\n";
	const char AT_setStationMode[] = "AT+CWMODE=1\r\n";
	const char AT_dc[] = "AT+CWQAP\r\n";
	const char AT_connectStationToAp[] = "AT+CWJAP=\"WIFI NAME\",\"WIFI PASSWORD\"\r\n";
	// Establish TCP connection
	const char AT_establishTCP[] = "AT+CIPSTART=\"TCP\",\"192.168.1.174\",65432\r\n";
	const char AT_sendTestData[] = "AT+CIPSEND=5\r\n";
	const char testData[] = "data\n";

	osDelay(2000);

	printf ("Setting echo off...\r\n");
	sendAtAndWaitForResponse(AT_echo, "OK");

	printf ("Connecting to AP...\r\n");
	sendAtAndWaitForResponse(AT_connectStationToAp, "OK");

	printf ("Connecting to the server...\r\n");
	sendAtAndWaitForResponse(AT_establishTCP, "OK");

	for (;;)
	{

		sendAtAndWaitForResponse(AT_sendTestData, "OK");
		sendAtAndWaitForResponse(testData, "SEND OK");

		printf("Esp tx\r\n");
		osDelay(3000);
	}
}

/* USER CODE END Application */

