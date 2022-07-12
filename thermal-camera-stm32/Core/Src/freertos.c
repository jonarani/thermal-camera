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

extern MLX_Data mlxData;
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

//static uint16_t swap_uint16(uint16_t val)
//{
//				return (val << 8) | (val >> 8 );
//}

void StartDefaultTask(void *argument)
{
				//uint8_t refreshRate[2] = {0};

				// Wait 80ms + delay determined by the refresh rate
				osDelay(80 + 1000); // Given that refresh rate is set 2Hz

				// Extract calibration data from EEPROM and store in RAM
				extractCalibrationData();


				//HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, mlx90640Address << 1, 100, osWaitForever);
				//printf ("Device Ready status: %d\r\n", status);
				//status = HAL_I2C_Mem_Read_DMA(&hi2c1, mlx90640Address << 1, 0x2440, I2C_MEMADD_SIZE_16BIT, cam_frames, NUM_OF_PIXELS);
				//printf ("Master Receive status: %d\r\n", status);

				printf ("-------------------Eeprom data start ----------------\r\n");

				// TODO: Move EEPROM extractions to separate functions and then brake down the calcu
				printf("kvdd: %d\r\n", mlxData.eepromData.kVdd);
				printf("vdd25: %d\r\n\n", mlxData.eepromData.Vdd25);

				printf("Kvptat: %.5f\r\n", mlxData.eepromData.KvPtat);
				printf("Ktptat: %.5f\r\n", mlxData.eepromData.KtPtat);
				printf("Vptat25: %d\r\n", mlxData.eepromData.Vptat25);
				printf("AlphaptatEe: %d\r\n", mlxData.eepromData.AlphaPtatEe);
				printf("Alphaptat: %d\r\n", mlxData.eepromData.AlphaPtat);

				printf("offsetAvg: %d\r\n\n", mlxData.eepromData.offsetAvg);
				uint16_t i = 0;
				for (i = 0; i < 6; i++)
								printf ("offsetRows[%d]: 0x%X\r\n", i, mlxData.eepromData.offsetRows[i]);

				printf ("occRows: ");
				for (i = 0; i < 24; i++)
								printf ("[%d: %d] ", i + 1, mlxData.eepromData.occRows[i]);
				printf ("\r\n");
				printf("occScaleRow: %d\r\n", mlxData.eepromData.occScaleRow);

				for (i = 0; i < 8; i++)
								printf("offsetCols[%d]: 0x%X\r\n", i, mlxData.eepromData.offsetCols[i]);

				printf("occCols: ");
				for (i = 0; i < 32; i++)
								printf("[%d: %d] ", i + 1, mlxData.eepromData.occCols[i]);
				printf("\r\n");

				printf("occScaleCol: %d\r\n", mlxData.eepromData.occScaleCol);
				printf("occScaleRemnant: %d\r\n", mlxData.eepromData.occScaleRemnant);

				printf ("pix(12,16) offset: %d\r\n", getPixelOffset(12, 16));
				printf("\r\n");

				printf("alphaRefernce: 0x%X or %d\r\n", mlxData.eepromData.alphaRef, mlxData.eepromData.alphaRef);
				printf("alphaScale: %d\r\n", mlxData.eepromData.alphaScale);

				for (i = 0; i < 6; i++)
								printf("accRowRegisters[%d]: 0x%X\r\n", i, mlxData.eepromData.accRowRegisters[i]);

				printf("accRows: ");
				for (i = 0; i < 24; i++)
								printf("[%d: %d] ", i + 1, mlxData.eepromData.accRows[i]);

				printf("\r\n");

				for (i = 0; i < 8; i++)
								printf("accColRegisters[%d]: 0x%X\r\n", i, mlxData.eepromData.accColRegisters[i]);

				printf("accCols: ");
				for (i = 0; i < 32; i++)
								printf("[%d: %d] ", i + 1, mlxData.eepromData.accCols[i]);

				printf("\r\n");

				printf("accScaleRow: %d\r\n", mlxData.eepromData.accScaleRow);
				printf("accScaleCol: %d\r\n", mlxData.eepromData.accScaleCol);
				printf("accScaleRemnant: %d\r\n", mlxData.eepromData.accScaleRemnant);

				printf ("alphaPixel(12,16): %d\r\n", getAlphaPixel(12, 16));

				printf("\r\n");

				printf("KTaEe(12,16): %d\r\n", getKTaEe(12,16));
				printf("KTaRcEe(12,16): %d\r\n", getKTaRcEe(12,16));
				printf("KTaScale1: %d\r\n", mlxData.eepromData.kTaScale1);
				printf("KTaScale2: %d\r\n", mlxData.eepromData.kTaScale2);

				printf ("\r\n");

				printf("step: %d\r\n", mlxData.eepromData.step);
				printf("ct3: %d\r\n", mlxData.eepromData.ct3);
				printf("ct4: %d\r\n", mlxData.eepromData.ct4);

				printf("\r\n");
				printf("gain: %d\r\n", mlxData.eepromData.gain);

				printf ("\r\n");

				printf ("ksToScale: %d\r\n", mlxData.eepromData.ksToScale);
				printf ("ksTo1Ee: %d\r\n", mlxData.eepromData.ksTo1Ee);
				printf ("ksTo1: %.5f\r\n", mlxData.eepromData.ksTo1);
				printf ("ksTo2Ee %d\r\n", mlxData.eepromData.ksTo2Ee);
				printf ("ksTo2: %.5f\r\n", mlxData.eepromData.ksTo2);
				printf ("ksTo3Ee %d\r\n", mlxData.eepromData.ksTo3Ee);
				printf ("ksTo3: %.5f\r\n", mlxData.eepromData.ksTo3);
				printf ("ksTo4Ee %d\r\n", mlxData.eepromData.ksTo4Ee);
				printf ("ksTo4: %.5f\r\n", mlxData.eepromData.ksTo4);

				printf ("\r\n");

				printf("alphaCorrRange1: %.5f\r\n", mlxData.eepromData.alphaCorrRange1);
				printf("alphaCorrRange2: %.5f\r\n", mlxData.eepromData.alphaCorrRange2);
				printf("alphaCorrRange3: %.5f\r\n", mlxData.eepromData.alphaCorrRange3);
				printf("alphaCorrRange4: %.5f\r\n", mlxData.eepromData.alphaCorrRange4);

				printf ("\r\n");

				printf("alpha(12,16): %.10f\r\n", getAlphaij(12, 16));
				printf("alphaScaleCp: %d\r\n", mlxData.eepromData.alphaScaleCp);
				printf("cp_P1P0_ratio: %d\r\n", mlxData.eepromData.cp_P1P0_ratio);
				printf("alphaCpSubpage0: %.15f\r\n", mlxData.eepromData.alphaCpSubpage0);
				printf("alphaCpSubpage1: %.15f\r\n", mlxData.eepromData.alphaCpSubpage1);

				printf("\r\n");

				printf ("offCpSubpage0 %d\r\n", mlxData.eepromData.offCpSubpage0);
				printf ("offCpSubpage1Delta %d\r\n", mlxData.eepromData.offCpSubpage1Delta);
				printf ("offCpSubpage1 %d\r\n", mlxData.eepromData.offCpSubpage1);

				printf("\r\n");

				printf("kvScale: %d\r\n", mlxData.eepromData.kvScale);
				printf("kvCpEe: %d\r\n", mlxData.eepromData.kvCpEe);
				printf("kvCp: %.10f\r\n", mlxData.eepromData.kvCp);

				printf("\r\n");

				printf("kTaCpEe: %d\r\n", mlxData.eepromData.kTaCpEe);
				printf("kTaCp: %.15f\r\n", mlxData.eepromData.kTaCp);

				printf("\r\n");

				printf("tgcEe: %d\r\n", mlxData.eepromData.tgcEe);
				printf("tgc: %.7f\r\n", mlxData.eepromData.tgc);

				printf("\r\n");

				printf("ResolutionEE: %d\r\n", mlxData.eepromData.resolutionEe);

				printf("\r\n");
				printf ("-------------------Calculated data start ----------------\r\n");

				printf("ResolutionReg: %d\r\n", mlxData.calcuData.resolutionReg);
				printf("ResolutionCorr: %.2f\r\n\n", mlxData.calcuData.resolutionCorr);
				printf("Vdd: %.5f\r\n", mlxData.calcuData.Vdd);
				printf("deltaV: %.5f\r\n", mlxData.calcuData.deltaV);

				printf("Vptat: %d\r\n", mlxData.calcuData.vPtat);
				printf("Vbe: %d\r\n", mlxData.calcuData.vBe);
				printf("Vptatart: %.5f\r\n", mlxData.calcuData.vPtatArt);
				printf("ta: %.5f\r\n", mlxData.calcuData.ta);

				printf("Kgain: %.5f\r\n", mlxData.calcuData.kGain);

				printf("\r\n");

				printf("Kta(12,16): %.7f\r\n", getKTaij(12,16));
				printf("Kvij(12,16): %.7f\r\n", getKvij(12,16));
				printf("osRef(12,16): %d\r\n", getPixOsref(12, 16));
				printf("tempData: %.7f\r\n", mlxData.tempData[11*32 + 15]);

				printf("\r\n");
				printf("pixGainCpSp0: %.7f\r\n", mlxData.calcuData.pixGainCpSp0);
				printf("pixGainCpSp1: %.7f\r\n", mlxData.calcuData.pixGainCpSp1);

				printf ("\r\n");
				printf("pixOsCpSp0: %.7f\r\n", mlxData.calcuData.pixOsCpSp0);
				printf("pixOsCpSp1: %.7f\r\n", mlxData.calcuData.pixOsCpSp1);

				printf("\r\n");
				printf("PATTERN(1): %d\r\n", PATTERN(1));
				printf("PATTERN(2): %d\r\n", PATTERN(2));
				printf("PATTERN(3): %d\r\n", PATTERN(3));
				printf("PATTERN(4): %d\r\n", PATTERN(4));
				printf("PATTERN(368): %d\r\n", PATTERN(368));

//				setRefreshRate(MLX_16_HZ);
//				HAL_I2C_Mem_Read(&hi2c1, mlx90640Address << 1, 0x800D, I2C_MEMADD_SIZE_16BIT, refreshRate, 2, 3000);
//				printf ("Refresh rate from 0x800D: 0x%X\r\n", swap_uint16(*(uint16_t*)refreshRate));

				// Get raw IR data
				// Gain compensation

				printf ("------------------------Starting for loop-----------------------------------.\r\n");
				for (;;)
				{
								//printf ("test\r\n");
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

