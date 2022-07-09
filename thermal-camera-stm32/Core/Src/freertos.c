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

extern MLX_CalibrationData caliData;
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

static uint16_t swap_uint16(uint16_t val)
{
				return (val << 8) | (val >> 8 );
}

void StartDefaultTask(void *argument)
{
				uint8_t refreshRate[2] = {0};

				// Wait 80ms + delay determined by the refresh rate
				osDelay(80 + 1000); // Given that refresh rate is set 2Hz

				// Extract calibration data from EEPROM and store in RAM
				extractCalibrationData();


				//HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, mlx90640Address << 1, 100, osWaitForever);
				//printf ("Device Ready status: %d\r\n", status);
				//status = HAL_I2C_Mem_Read_DMA(&hi2c1, mlx90640Address << 1, 0x2440, I2C_MEMADD_SIZE_16BIT, cam_frames, NUM_OF_PIXELS);
				//printf ("Master Receive status: %d\r\n", status);

				// TODO: Move EEPROM extractions to separate functions and then brake down the calcu
				printf("kvdd: %d\r\n", caliData.eepromData.kVdd);
				printf("vdd25: %d\r\n\n", caliData.eepromData.Vdd25);

				printf("Kvptat: %.5f\r\n", caliData.eepromData.KvPtat);
				printf("Ktptat: %.5f\r\n", caliData.eepromData.KtPtat);
				printf("deltaV: %.5f\r\n", caliData.taSensorParams.deltaV);
				printf("Vdd: %.5f\r\n", caliData.taSensorParams.Vdd);
				printf("Vptat25: %d\r\n", caliData.eepromData.Vptat25);
				printf("Vptat: %d\r\n", caliData.taSensorParams.Vptat);
				printf("Vbe: %d\r\n", caliData.taSensorParams.Vbe);
				printf("AlphaptatEe: %d\r\n", caliData.eepromData.AlphaPtatEe);
				printf("Alphaptat: %d\r\n", caliData.eepromData.AlphaPtat);
				printf("Vptatart: %.5f\r\n", caliData.taSensorParams.Vptatart);
				printf("Ta: %.5f\r\n\n", caliData.taSensorParams.Ta);

				printf("offsetAvg: %d\r\n\n", caliData.eepromData.offsetAvg);
				uint8_t i = 0;
				for (i = 0; i < 6; i++)
								printf ("offsetRows[%d]: 0x%X\r\n", i, caliData.eepromData.offsetRows[i]);

				printf ("occRows: ");
				for (i = 0; i < 24; i++)
								printf ("[%d: %d] ", i + 1, caliData.eepromData.occRows[i]);
				printf ("\r\n");
				printf("occScaleRow: %d\r\n", caliData.eepromData.occScaleRow);

				for (i = 0; i < 8; i++)
								printf("offsetCols[%d]: 0x%X\r\n", i, caliData.eepromData.offsetCols[i]);

				printf("occCols: ");
				for (i = 0; i < 32; i++)
								printf("[%d: %d] ", i + 1, caliData.eepromData.occCols[i]);
				printf("\r\n");

				printf("occScaleCol: %d\r\n", caliData.eepromData.occScaleCol);
				printf("occScaleRemnant: %d\r\n", caliData.eepromData.occScaleRemnant);

				printf ("pix(12,16) offset: %d\r\n", getPixelOffset(12, 16));

				printf("\r\n");
				printf("gain: %d\r\n", caliData.eepromData.gain);
				printf("Kgain: %.5f\r\n", caliData.kGain);

				printf("ResolutionEE: %d\r\n", caliData.eepromData.resolutionEe);
				printf("ResolutionReg: %d\r\n", caliData.resolutionsparams.resolutionReg);
				printf("ResolutionCorr: %.2f\r\n\n", caliData.resolutionsparams.resolutionCorr);

//				setRefreshRate(MLX_16_HZ);
//				HAL_I2C_Mem_Read(&hi2c1, mlx90640Address << 1, 0x800D, I2C_MEMADD_SIZE_16BIT, refreshRate, 2, 3000);
//				printf ("Refresh rate from 0x800D: 0x%X\r\n", swap_uint16(*(uint16_t*)refreshRate));

				// Get raw IR data
				// Gain compensation

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

