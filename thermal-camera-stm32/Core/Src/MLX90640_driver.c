/*
 * MLX90640_driver.c
 *
 *  Created on: Jul 6, 2022
 *      Author: Jonathan
 */


#include <MLX90640_driver.h>
#include "main.h"
#include <stdio.h>
#include <math.h>

// Default address is 0x33
uint8_t mlx90640Address = 0x33;
MLX_CalibrationData caliData; // Calibration Data

extern I2C_HandleTypeDef hi2c1;

#define TIMEOUT_MS (3000)

static uint16_t swap_uint16(uint16_t val)
{
				return (val << 8) | (val >> 8 );
}

// Row 1 - 24, Col 1 - 32
int16_t getPixelOffset(uint8_t pixelRow, uint8_t pixelCol)
{
				int16_t pixelOffset =
												(caliData.eepromData.pixelOffsets[(pixelRow - 1) * 32 + (pixelCol - 1)]
												& 0xFC00)
												>> 10;

				if (pixelOffset > 31)
								pixelOffset -= 64;

				return pixelOffset;
}

static MLX90640_StatusTypedef restoreVddSensorParams()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t vddParamRegister = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KVDD_VDD25_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&vddParamRegister,
																														sizeof(vddParamRegister),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreVddSensorParams HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				vddParamRegister = swap_uint16(vddParamRegister);

				caliData.eepromData.kVdd = (vddParamRegister & 0xFF00) >> 8;
				if (caliData.eepromData.kVdd > 127)
								caliData.eepromData.kVdd -= 256;
				caliData.eepromData.kVdd <<= 5;

				caliData.eepromData.Vdd25 = ((vddParamRegister & 0x00FF) - 256) * 32 - 8192;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreTaSensorParams()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t kvKtPtatRegister = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KVPTAT_KTPTAT_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&kvKtPtatRegister,
																														sizeof(kvKtPtatRegister),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreKvKtPtats HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				kvKtPtatRegister = swap_uint16(kvKtPtatRegister);

				caliData.eepromData.KvPtat = (kvKtPtatRegister & 0xFC00) >> 10;
				if (caliData.eepromData.KvPtat > 31)
								caliData.eepromData.KvPtat -= 64;
				caliData.eepromData.KvPtat /= 4096;

				caliData.eepromData.KtPtat = kvKtPtatRegister & 0x03FF;
				if (caliData.eepromData.KtPtat > 511)
								caliData.eepromData.KtPtat -= 1024;
				caliData.eepromData.KtPtat /= 8;



				return MLX_OK;
}

static MLX90640_StatusTypedef restoreOffsets()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint8_t offsetAvgReg[2] = {0};
				uint8_t offsetRows[12] = {0};
				uint8_t kptatAndScales[2] = {0};
				uint8_t offsetCols[16] = {0};

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_PIX_OS_AVG_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&offsetAvgReg,
																														sizeof(offsetAvgReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				caliData.eepromData.offsetAvg = (offsetAvgReg[0] << 8) | offsetAvgReg[1];

				// Restore rows
				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_OFFSET_ROW_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&offsetRows,
																														sizeof(offsetRows),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				uint16_t i = 0;
				for (i = 0; i < 6; i++)
								caliData.eepromData.offsetRows[i] = (offsetRows[i*2] << 8) | offsetRows[i*2 + 1];

				printf ("OccRow12: %d\r\n", (caliData.eepromData.offsetRows[2] & 0xF000) >> 12);

				for (i = 0; i < NUM_ROWS_IN_FRAME; i++)
				{
								uint8_t maskShift = (i % 4) * 4;
								uint16_t mask = 0x000F << maskShift;
								uint8_t offsetRowsElement = (double)i / 4.;

								caliData.eepromData.occRows[i] =
																(caliData.eepromData.offsetRows[offsetRowsElement] & mask) >> maskShift;

								if (caliData.eepromData.occRows[i] > 7)
												caliData.eepromData.occRows[i] -= 16;
				}

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KPTAT_AND_SCALE_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&kptatAndScales,
																														sizeof(kptatAndScales),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK3!\r\n");
								return MLX_HAL_ERROR;
				}

				caliData.eepromData.kPtatAndScales = (kptatAndScales[0] << 8) | kptatAndScales[1];

				caliData.eepromData.occScaleRow = (caliData.eepromData.kPtatAndScales & 0x0F00) >> 8;


				// Restore cols
				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_OFFSET_COL_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&offsetCols,
																														sizeof(offsetCols),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK4!\r\n");
								return MLX_HAL_ERROR;
				}

				for (i = 0; i < 8; i++)
								caliData.eepromData.offsetCols[i] = (offsetCols[i*2] << 8) | offsetCols[i*2 + 1];

				for (i = 0; i < NUM_COLS_IN_FRAME; i++)
				{
								uint8_t maskShift = (i % 4) * 4;
								uint16_t mask = 0x000F << maskShift;
								uint8_t offsetColsElement = (double)i / 4.;

								caliData.eepromData.occCols[i] =
																(caliData.eepromData.offsetCols[offsetColsElement] & mask) >> maskShift;

								if (caliData.eepromData.occCols[i] > 7)
												caliData.eepromData.occCols[i] -= 16;
				}

				caliData.eepromData.occScaleCol = (caliData.eepromData.kPtatAndScales & 0x00F0) >> 4;
				caliData.eepromData.occScaleRemnant = (caliData.eepromData.kPtatAndScales & 0x000F);


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_PIXEL_CALI_DATA_START_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&caliData.eepromData.pixelOffsets,
																														sizeof(caliData.eepromData.pixelOffsets),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK5!\r\n");
								return MLX_HAL_ERROR;
				}

				for (i = 0; i < NUM_ROWS_IN_FRAME * NUM_COLS_IN_FRAME; i++)
								caliData.eepromData.pixelOffsets[i] = swap_uint16(caliData.eepromData.pixelOffsets[i]);


				return MLX_OK;
}

static MLX90640_StatusTypedef restoreGain()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t gainRegister = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_GAIN_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&gainRegister,
																														sizeof(gainRegister),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreGain HAL NOT OK 2!\r\n");
								return MLX_HAL_ERROR;
				}

				gainRegister = swap_uint16(gainRegister);
				caliData.eepromData.gain = gainRegister;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreResolutionEe()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t resolutionReg = 0;
				uint16_t ptat25Register = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RES_CTRL_AND_SCALE_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&resolutionReg,
																														sizeof(resolutionReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreResolution HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				resolutionReg = swap_uint16(resolutionReg);
				caliData.eepromData.resolutionEe = (resolutionReg & 0x3000) >> 12;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_PTAT_25_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ptat25Register,
																														sizeof(ptat25Register),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreVptat25 HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				ptat25Register = swap_uint16(ptat25Register);

				caliData.eepromData.Vptat25 = ptat25Register;

				caliData.eepromData.AlphaPtatEe =
												(caliData.eepromData.kPtatAndScales & 0xF000) >> 12;

				caliData.eepromData.AlphaPtat =
												(caliData.eepromData.AlphaPtatEe >> 2) + 8;

				return MLX_OK;
}

static MLX90640_StatusTypedef getEepromData()
{
				restoreVddSensorParams();
				restoreTaSensorParams();
				restoreOffsets();
				restoreGain();
				restoreResolutionEe();

				return MLX_OK;
}

MLX90640_StatusTypedef extractCalibrationData()
{
				getEepromData();

				// TODO: keep for now as testing purposes
				resolutionCalculations();
				taCalculations();	// Ta - ambient temperature
				gainParameterCalculation();

				return MLX_OK;
}

MLX90640_StatusTypedef setRefreshRate(MLX90640_RefreshRateTypedef refreshRate)
{
				if (refreshRate < MLX_0_5_HZ || refreshRate > MLX_64_HZ)
				{
								printf("setRefreshRate INVALID PARAMS!\r\n");
								return MLX_INVALID_PARAM;
				}

				HAL_StatusTypeDef status = HAL_OK;

				uint16_t controlRegister = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_CONTROL_REGISTER_1,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&controlRegister,
																														sizeof(controlRegister),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("setRefreshRate HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				controlRegister = swap_uint16(controlRegister);

				controlRegister = (controlRegister & 0xFC7F) | (refreshRate << 0x7);

				controlRegister = swap_uint16(controlRegister);

				status = HAL_I2C_Mem_Write(&hi2c1,
																															mlx90640Address << 1,
																															// Can write to MLX90640_EEPROM_REFRESH_RATE_ADDR
																															// to set it permanently, however reboot it required
																															MLX90640_CONTROL_REGISTER_1,
																															I2C_MEMADD_SIZE_16BIT,
																															(uint8_t*)&controlRegister,
																															sizeof(controlRegister),
																															TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("setRefreshRate HAL NOT OK 2!\r\n");
								return MLX_HAL_ERROR;
				}

				return MLX_OK;
}

MLX90640_StatusTypedef resolutionCalculations()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t controlRegister = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_CONTROL_REGISTER_1,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&controlRegister,
																														sizeof(controlRegister),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreResolution HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				controlRegister = swap_uint16(controlRegister);
				caliData.resolutionsparams.resolutionReg = (controlRegister & 0x0C00) >> 10;

				caliData.resolutionsparams.resolutionCorr =
																				pow(2, caliData.eepromData.resolutionEe) /
																				pow(2, caliData.resolutionsparams.resolutionReg);

				return MLX_OK;
}

MLX90640_StatusTypedef taCalculations()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t tempRam072a = 0;
				int16_t ram072a = 0;
				uint16_t ram0720 = 0;
				uint16_t ram0700 = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_072A,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&tempRam072a,
																														sizeof(tempRam072a),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreTaSensorParams HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				tempRam072a = swap_uint16(tempRam072a);
				ram072a = tempRam072a;

				caliData.taSensorParams.deltaV = (double)(ram072a -
																																														caliData.eepromData.Vdd25) /
																																														caliData.eepromData.kVdd;

				caliData.taSensorParams.Vdd = (caliData.resolutionsparams.resolutionCorr *
																																	 (double)(ram072a - caliData.eepromData.Vdd25)) /
																																	 (double)caliData.eepromData.kVdd + 3.3;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0720,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0720,
																														sizeof(ram0720),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreTaSensorParams HAL NOT OK 4!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0720 = swap_uint16(ram0720);
				caliData.taSensorParams.Vptat = ram0720;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0700,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0700,
																														sizeof(ram0700),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreTaSensorParams HAL NOT OK 5!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0700 = swap_uint16(ram0700);
				caliData.taSensorParams.Vbe = ram0700;

				caliData.taSensorParams.Vptatart =
												(double)((double)caliData.taSensorParams.Vptat /
												((double)caliData.taSensorParams.Vptat *
												 (double)caliData.eepromData.AlphaPtat +
												 (double)caliData.taSensorParams.Vbe))
													* 262144.;

				caliData.taSensorParams.Ta =
								((caliData.taSensorParams.Vptatart /
								 (1. + caliData.eepromData.KvPtat *
									 caliData.taSensorParams.deltaV)) -
												caliData.eepromData.Vptat25) /
																caliData.eepromData.KtPtat + 25.;

				return MLX_OK;
}

MLX90640_StatusTypedef gainParameterCalculation()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t tempRam070a = 0;
				int16_t ram070a;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_070A,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&tempRam070a,
																														sizeof(tempRam070a),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("gainParameterCalculation HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				tempRam070a = swap_uint16(tempRam070a);
				ram070a = tempRam070a;

				// This value is updated every frame and it is same for all pixels
				// including CP regardless the subpage number
				caliData.kGain = (double)caliData.eepromData.gain / (double)ram070a;

				return MLX_OK;
}
