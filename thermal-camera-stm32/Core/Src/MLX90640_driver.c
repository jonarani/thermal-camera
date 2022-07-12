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
MLX_Data mlxData; // Calibration Data

extern I2C_HandleTypeDef hi2c1;

#define TIMEOUT_MS (3000)

static uint16_t swap_uint16(uint16_t val)
{
				return (val << 8) | (val >> 8 );
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

				mlxData.eepromData.kVdd = (vddParamRegister & 0xFF00) >> 8;
				if (mlxData.eepromData.kVdd > 127)
								mlxData.eepromData.kVdd -= 256;
				mlxData.eepromData.kVdd <<= 5;

				mlxData.eepromData.Vdd25 = ((vddParamRegister & 0x00FF) - 256) * 32 - 8192;

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

				mlxData.eepromData.KvPtat = (kvKtPtatRegister & 0xFC00) >> 10;
				if (mlxData.eepromData.KvPtat > 31)
								mlxData.eepromData.KvPtat -= 64;
				mlxData.eepromData.KvPtat /= 4096;

				mlxData.eepromData.KtPtat = kvKtPtatRegister & 0x03FF;
				if (mlxData.eepromData.KtPtat > 511)
								mlxData.eepromData.KtPtat -= 1024;
				mlxData.eepromData.KtPtat /= 8;



				return MLX_OK;
}

static MLX90640_StatusTypedef restoreOffsets()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint8_t offsetAvgReg[2] = {0};
				uint8_t offsetRows[12] = {0};
				uint8_t kptatAndScales[2] = {0};
				uint8_t offsetCols[16] = {0};
				uint16_t tempOffsetAvg = 0;

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

				tempOffsetAvg = (offsetAvgReg[0] << 8) | offsetAvgReg[1];
				mlxData.eepromData.offsetAvg = tempOffsetAvg;

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
								mlxData.eepromData.offsetRows[i] = (offsetRows[i*2] << 8) | offsetRows[i*2 + 1];

				for (i = 0; i < NUM_ROWS_IN_FRAME; i++)
				{
								uint8_t maskShift = (i % 4) * 4;
								uint16_t mask = 0x000F << maskShift;
								uint8_t offsetRowsElement = (double)i / 4.;

								mlxData.eepromData.occRows[i] =
																(mlxData.eepromData.offsetRows[offsetRowsElement] & mask) >> maskShift;

								if (mlxData.eepromData.occRows[i] > 7)
												mlxData.eepromData.occRows[i] -= 16;
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

				mlxData.eepromData.kPtatAndScales = (kptatAndScales[0] << 8) | kptatAndScales[1];

				mlxData.eepromData.occScaleRow = (mlxData.eepromData.kPtatAndScales & 0x0F00) >> 8;


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
								mlxData.eepromData.offsetCols[i] = (offsetCols[i*2] << 8) | offsetCols[i*2 + 1];

				for (i = 0; i < NUM_COLS_IN_FRAME; i++)
				{
								uint8_t maskShift = (i % 4) * 4;
								uint16_t mask = 0x000F << maskShift;
								uint8_t offsetColsElement = (double)i / 4.;

								mlxData.eepromData.occCols[i] =
																(mlxData.eepromData.offsetCols[offsetColsElement] & mask) >> maskShift;

								if (mlxData.eepromData.occCols[i] > 7)
												mlxData.eepromData.occCols[i] -= 16;
				}

				mlxData.eepromData.occScaleCol = (mlxData.eepromData.kPtatAndScales & 0x00F0) >> 4;
				mlxData.eepromData.occScaleRemnant = (mlxData.eepromData.kPtatAndScales & 0x000F);


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_PIXEL_CALI_DATA_START_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&mlxData.eepromData.pixelOffsets,
																														sizeof(mlxData.eepromData.pixelOffsets),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreOffsets HAL NOT OK5!\r\n");
								return MLX_HAL_ERROR;
				}

				for (i = 0; i < NUM_ROWS_IN_FRAME * NUM_COLS_IN_FRAME; i++)
								mlxData.eepromData.pixelOffsets[i] = swap_uint16(mlxData.eepromData.pixelOffsets[i]);


				return MLX_OK;
}

static MLX90640_StatusTypedef restoreIlChess()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t ilChessReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_IL_CHESS_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ilChessReg,
																														sizeof(ilChessReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreIlChess HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				ilChessReg = swap_uint16(ilChessReg);

				mlxData.eepromData.ilChessC1Ee = ilChessReg & 0x003F;
				if (mlxData.eepromData.ilChessC1Ee > 31)
								mlxData.eepromData.ilChessC1Ee -= 64;

				mlxData.eepromData.ilChessC1 /= pow(2,4);


				mlxData.eepromData.ilChessC2Ee = (ilChessReg & 0x07C0) / pow(2,6);
				if (mlxData.eepromData.ilChessC2Ee > 15)
								mlxData.eepromData.ilChessC2Ee -= 32;

				mlxData.eepromData.ilChessC2 /= 2;


				mlxData.eepromData.ilChessC3Ee = (ilChessReg & 0xF800) / pow(2,11);
				if (mlxData.eepromData.ilChessC3Ee > 15)
								mlxData.eepromData.ilChessC3Ee -= 32;

				mlxData.eepromData.ilChessC3 /= pow(2,3);


				return MLX_OK;
}

static MLX90640_StatusTypedef restoreSensitivityAlphaij()
{
				uint16_t i = 0;
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t kvAvgReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																																		mlx90640Address << 1,
																																		MLX90640_ALPHA_REF_ADDR,
																																		I2C_MEMADD_SIZE_16BIT,
																																		(uint8_t*)&mlxData.eepromData.alphaRef,
																																		sizeof(mlxData.eepromData.alphaRef),
																																		TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreSensitivity HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				mlxData.eepromData.alphaRef = swap_uint16(mlxData.eepromData.alphaRef);

				status = HAL_I2C_Mem_Read(&hi2c1,
																																		mlx90640Address << 1,
																																		MLX90640_SCALE_ACC_ADDR,
																																		I2C_MEMADD_SIZE_16BIT,
																																		(uint8_t*)&mlxData.eepromData.scaleACCReg,
																																		sizeof(mlxData.eepromData.scaleACCReg),
																																		TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreSensitivity HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				mlxData.eepromData.scaleACCReg = swap_uint16(mlxData.eepromData.scaleACCReg);
				mlxData.eepromData.alphaScale = ((mlxData.eepromData.scaleACCReg & 0xF000) >> 12) + 30;


				// restore acc rows
				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_ACC_ROW_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&mlxData.eepromData.accRowRegisters,
																														sizeof(mlxData.eepromData.accRowRegisters),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreSensitivity HAL NOT OK3!\r\n");
								return MLX_HAL_ERROR;
				}

				for (i = 0; i < 6; i++)
								mlxData.eepromData.accRowRegisters[i] =
																swap_uint16(mlxData.eepromData.accRowRegisters[i]);

				for (i = 0; i < NUM_ROWS_IN_FRAME; i++)
				{
								uint8_t maskShift = (i % 4) * 4;
								uint16_t mask = 0x000F << maskShift;
								uint8_t accRowsElement = (double)i / 4.;

								mlxData.eepromData.accRows[i] =
																(mlxData.eepromData.accRowRegisters[accRowsElement] & mask) >> maskShift;

								if (mlxData.eepromData.accRows[i] > 7)
												mlxData.eepromData.accRows[i] -= 16;
				}

				mlxData.eepromData.accScaleRow = (mlxData.eepromData.scaleACCReg & 0x0F00) >> 8;


				// restore acc cols
				status = HAL_I2C_Mem_Read(&hi2c1,
																																	mlx90640Address << 1,
																																	MLX90640_ACC_COL_ADDR,
																																	I2C_MEMADD_SIZE_16BIT,
																																	(uint8_t*)&mlxData.eepromData.accColRegisters,
																																	sizeof(mlxData.eepromData.accColRegisters),
																																	TIMEOUT_MS);

			if (status != HAL_OK)
			{
							printf("restoreSensitivity HAL NOT OK4!\r\n");
							return MLX_HAL_ERROR;
			}

			for (i = 0; i < 8; i++)
							mlxData.eepromData.accColRegisters[i] =
															swap_uint16(mlxData.eepromData.accColRegisters[i]);

			for (i = 0; i < NUM_COLS_IN_FRAME; i++)
			{
							uint8_t maskShift = (i % 4) * 4;
							uint16_t mask = 0x000F << maskShift;
							uint8_t accColsElement = (double)i / 4.;

							mlxData.eepromData.accCols[i] =
															(mlxData.eepromData.accColRegisters[accColsElement] & mask) >> maskShift;

							if (mlxData.eepromData.accCols[i] > 7)
											mlxData.eepromData.accCols[i] -= 16;
			}

			mlxData.eepromData.accScaleCol = (mlxData.eepromData.scaleACCReg & 0x00F0) >> 4;

			mlxData.eepromData.accScaleRemnant = mlxData.eepromData.scaleACCReg & 0x000F;


			// restore kv avg
			status = HAL_I2C_Mem_Read(&hi2c1,
																													mlx90640Address << 1,
																													MLX90640_KV_AVG_ADDR,
																													I2C_MEMADD_SIZE_16BIT,
																													(uint8_t*)&kvAvgReg,
																													sizeof(kvAvgReg),
																													TIMEOUT_MS);

			if (status != HAL_OK)
			{
							printf("restoreKsToCoefficient HAL NOT OK4!\r\n");
							return MLX_HAL_ERROR;
			}

			kvAvgReg = swap_uint16(kvAvgReg);

			mlxData.eepromData.kvAvg = kvAvgReg;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreKtaCoefficient()
{
				uint16_t kTaScaleReg = 0;
				HAL_StatusTypeDef status = HAL_OK;

				status = HAL_I2C_Mem_Read(&hi2c1,
																																		mlx90640Address << 1,
																																		MLX90640_KTA_AVG_ADDR,
																																		I2C_MEMADD_SIZE_16BIT,
																																		(uint8_t*)&mlxData.eepromData.kTaAvg,
																																		sizeof(mlxData.eepromData.kTaAvg),
																																		TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreKtaCoefficient HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				mlxData.eepromData.kTaAvg[0] = swap_uint16(mlxData.eepromData.kTaAvg[0]);
				mlxData.eepromData.kTaAvg[1] = swap_uint16(mlxData.eepromData.kTaAvg[1]);

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RES_CTRL_AND_SCALE_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&kTaScaleReg,
																														sizeof(kTaScaleReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreKtaCoefficient HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				kTaScaleReg = swap_uint16(kTaScaleReg);

				mlxData.eepromData.kTaScale1 = ((kTaScaleReg & 0x00F0) >> 4) + 8;
				mlxData.eepromData.kTaScale2 = kTaScaleReg & 0x000F;

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
								printf("restoreGain HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				gainRegister = swap_uint16(gainRegister);
				mlxData.eepromData.gain = gainRegister;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreCornerTemperatures()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t cornerTempReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_CORNER_TEMP_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&cornerTempReg,
																														sizeof(cornerTempReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreCornerTemperatures HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				cornerTempReg = swap_uint16(cornerTempReg);

				mlxData.eepromData.ct1 = -40;
				mlxData.eepromData.ct2 = 0;
				mlxData.eepromData.step = ((cornerTempReg & 0x3000) >> 12) * 10;
				mlxData.eepromData.ct3 = ((cornerTempReg & 0x00F0) >> 4) * mlxData.eepromData.step;
				mlxData.eepromData.ct4 = ((cornerTempReg & 0x0F00) >> 8) *
												mlxData.eepromData.step + mlxData.eepromData.ct3;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreKsToCoefficient()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t ksToScaleReg = 0;
				uint16_t ksToReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_CORNER_TEMP_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ksToScaleReg,
																														sizeof(ksToScaleReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreCornerTemperatures HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				ksToScaleReg = swap_uint16(ksToScaleReg);

				mlxData.eepromData.ksToScale = (ksToScaleReg & 0x000F) + 8;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KS_TO_1_2_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ksToReg,
																														sizeof(ksToReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreCornerTemperatures HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				ksToReg = swap_uint16(ksToReg);

				mlxData.eepromData.ksTo1Ee = ksToReg & 0x00FF;
				if (mlxData.eepromData.ksTo1Ee > 127)
								mlxData.eepromData.ksTo1Ee -= 256;

				mlxData.eepromData.ksTo1 = (double)mlxData.eepromData.ksTo1Ee
												/ pow(2, mlxData.eepromData.ksToScale);


				mlxData.eepromData.ksTo2Ee = (ksToReg & 0xFF00) >> 8;
				if (mlxData.eepromData.ksTo2Ee > 127)
								mlxData.eepromData.ksTo2Ee -= 256;

				mlxData.eepromData.ksTo2 = (double)mlxData.eepromData.ksTo2Ee
												/ pow(2, mlxData.eepromData.ksToScale);



				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KS_TO_3_4_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ksToReg,
																														sizeof(ksToReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreCornerTemperatures HAL NOT OK3!\r\n");
								return MLX_HAL_ERROR;
				}

				ksToReg = swap_uint16(ksToReg);

				mlxData.eepromData.ksTo3Ee = (ksToReg & 0x00FF);
				if (mlxData.eepromData.ksTo3Ee > 127)
								mlxData.eepromData.ksTo3Ee -= 256;

				mlxData.eepromData.ksTo3 = mlxData.eepromData.ksTo3Ee
												/ pow(2, mlxData.eepromData.ksToScale);


				mlxData.eepromData.ksTo4Ee = (ksToReg & 0xFF00) >> 8;
				if (mlxData.eepromData.ksTo4Ee > 127)
								mlxData.eepromData.ksTo4Ee -= 256;

				mlxData.eepromData.ksTo4 = mlxData.eepromData.ksTo4Ee
												/ pow(2, mlxData.eepromData.ksToScale);

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreSensitivityCorrectionCoeffients()
{

				mlxData.eepromData.alphaCorrRange1 =
												1. / (1. + mlxData.eepromData.ksTo1 * (0 - (-40)));

				mlxData.eepromData.alphaCorrRange2 = 1;

				mlxData.eepromData.alphaCorrRange3 =
												1. + mlxData.eepromData.ksTo2 * (mlxData.eepromData.ct3 - 0);

				mlxData.eepromData.alphaCorrRange4 =
												(1 + mlxData.eepromData.ksTo2 * (mlxData.eepromData.ct3 - 0))
												* (1 + mlxData.eepromData.ksTo3 *
																				(mlxData.eepromData.ct4 - mlxData.eepromData.ct3));

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreSensitivityAlphaCp()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t p1p0RatioReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																																	mlx90640Address << 1,
																																	MLX90640_CP_P1_P0_RATIO_ADDR,
																																	I2C_MEMADD_SIZE_16BIT,
																																	(uint8_t*)&p1p0RatioReg,
																																	sizeof(p1p0RatioReg),
																																	TIMEOUT_MS);

			if (status != HAL_OK)
			{
							printf("restoreSensitivityAlphaCp HAL NOT OK!\r\n");
							return MLX_HAL_ERROR;
			}

			p1p0RatioReg = swap_uint16(p1p0RatioReg);

			mlxData.eepromData.alphaScaleCp =
											((mlxData.eepromData.scaleACCReg & 0xF000) >> 12) + 27;

				mlxData.eepromData.cp_P1P0_ratio = (p1p0RatioReg &0xFC00) >> 10;
				if (mlxData.eepromData.cp_P1P0_ratio > 31)
								mlxData.eepromData.cp_P1P0_ratio -= 64;

				mlxData.eepromData.alphaCpSubpage0 = (double)(p1p0RatioReg & 0x03FF)
												/ pow(2, mlxData.eepromData.alphaScaleCp);

				mlxData.eepromData.alphaCpSubpage1 =
												(double)mlxData.eepromData.alphaCpSubpage0 *
												(double)(1. + ((double)mlxData.eepromData.cp_P1P0_ratio / pow(2, 7)));

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreOffsetOfTheCompensationPixel()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t offCpReg = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																																	mlx90640Address << 1,
																																	MLX90640_OFF_CP_ADDR,
																																	I2C_MEMADD_SIZE_16BIT,
																																	(uint8_t*)&offCpReg,
																																	sizeof(offCpReg),
																																	TIMEOUT_MS);

			if (status != HAL_OK)
			{
							printf("restoreOffsetOfTheCompensationPixel HAL NOT OK!\r\n");
							return MLX_HAL_ERROR;
			}

			offCpReg = swap_uint16(offCpReg);

			mlxData.eepromData.offCpSubpage0 = offCpReg & 0x03FF;
			if (mlxData.eepromData.offCpSubpage0 > 511)
							mlxData.eepromData.offCpSubpage0 -= 1024;

			mlxData.eepromData.offCpSubpage1Delta = (offCpReg & 0xFC00) >> 10;
			if (mlxData.eepromData.offCpSubpage1Delta > 31)
							mlxData.eepromData.offCpSubpage1Delta -= 64;

			mlxData.eepromData.offCpSubpage1 =
											mlxData.eepromData.offCpSubpage0 + mlxData.eepromData.offCpSubpage1Delta;

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreKvCpAndKtaCpCoefficient()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t kvCpReg = 0;
				uint16_t kvCpEeReg = 0;

				// Restoring Kv CP coefficient
				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RES_CTRL_AND_SCALE_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&kvCpReg,
																														sizeof(kvCpReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreKvCpAndKtaCpCoefficient HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				kvCpReg = swap_uint16(kvCpReg);
				mlxData.eepromData.kvScale = (kvCpReg & 0x0F00) >> 8;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_KV_KTA_CP_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&kvCpEeReg,
																														sizeof(kvCpEeReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreKvCpAndKtaCpCoefficient HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				kvCpEeReg = swap_uint16(kvCpEeReg);

				mlxData.eepromData.kvCpEe = (kvCpEeReg & 0x0FF00) >> 8;
				if (mlxData.eepromData.kvCpEe > 127)
								mlxData.eepromData.kvCpEe -= 256;

				mlxData.eepromData.kvCp = (double)mlxData.eepromData.kvCpEe /
												pow(2, mlxData.eepromData.kvScale);


				// Restoring Kta CP coefficient
				mlxData.eepromData.kTaCpEe = (kvCpEeReg & 0x00FF);
				if (mlxData.eepromData.kTaCpEe > 127)
								mlxData.eepromData.kTaCpEe -= 256;

				mlxData.eepromData.kTaCp = mlxData.eepromData.kTaCpEe /
												pow(2, mlxData.eepromData.kTaScale1);

				return MLX_OK;
}

static MLX90640_StatusTypedef restoreTGCCoefficient()
{

				HAL_StatusTypeDef status = HAL_OK;
				uint16_t tgcReg = 0;

				// Restoring Kv CP coefficient
				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_TGC_EE_ADDR,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&tgcReg,
																														sizeof(tgcReg),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("restoreTGCCoefficient HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				tgcReg = swap_uint16(tgcReg);

				printf("tgc reg: 0x%x\r\n", tgcReg);

				mlxData.eepromData.tgcEe = (tgcReg & 0x00FF);
				if (mlxData.eepromData.tgcEe > 127)
								mlxData.eepromData.tgcEe -= 256;

				mlxData.eepromData.tgc = (double)mlxData.eepromData.tgcEe /
												pow(2, 5);

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
				mlxData.eepromData.resolutionEe = (resolutionReg & 0x3000) >> 12;


				// TODO: move this to somewhere else
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

				mlxData.eepromData.Vptat25 = ptat25Register;

				mlxData.eepromData.AlphaPtatEe =
												(mlxData.eepromData.kPtatAndScales & 0xF000) >> 12;

				mlxData.eepromData.AlphaPtat =
												(mlxData.eepromData.AlphaPtatEe >> 2) + 8;

				return MLX_OK;
}

static MLX90640_StatusTypedef getEepromData()
{
				restoreVddSensorParams();
				restoreTaSensorParams();
				restoreOffsets();
				restoreIlChess();
				restoreSensitivityAlphaij();
				restoreKtaCoefficient();
				restoreGain();
				restoreCornerTemperatures();
				restoreKsToCoefficient();
				restoreSensitivityCorrectionCoeffients();
				restoreSensitivityAlphaCp();
				restoreOffsetOfTheCompensationPixel();
				restoreKvCpAndKtaCpCoefficient();
				restoreTGCCoefficient();
				restoreResolutionEe();

				return MLX_OK;
}

MLX90640_StatusTypedef extractCalibrationData()
{
				getEepromData();

				// TODO: keep for now as testing purposes
				resolutionCalculations();

				supplyVoltageCalculations();

				ambientTempCalculations();	// Ta - ambient temperature

				gainParameterCalculation();

				getPixelDataFromRam();

				calculateGainCompensation();

				calculateIrDataCompensation();

				compensateGainOfCpPixel();

				compensateOffsetTaAndVddOfCpPixel();

				return MLX_OK;
}


// Row 1 - 24, Col 1 - 32
int16_t getPixelOffset(uint8_t pixelRow, uint8_t pixelCol)
{
				int16_t pixelOffset =
												(mlxData.eepromData.pixelOffsets[(pixelRow - 1) * 32 + (pixelCol - 1)]
												& 0xFC00)
												>> 10;

				if (pixelOffset > 31)
								pixelOffset -= 64;

				return pixelOffset;
}

// Row 1 - 24, Col 1 - 32
int16_t getAlphaPixel(uint8_t pixelRow, uint8_t pixelCol)
{
				int16_t alphaPixel =
												(mlxData.eepromData.pixelOffsets[(pixelRow - 1) * 32 + (pixelCol - 1)]
												& 0x03F0)
												>> 4;

				if (alphaPixel > 31)
								alphaPixel -= 64;

				return alphaPixel;
}

// Row 1 - 24, Col 1 - 32
int8_t getKTaEe(uint8_t pixelRow, uint8_t pixelCol)
{
				int8_t kTaEe =
												(mlxData.eepromData.pixelOffsets[(pixelRow - 1) * 32 + (pixelCol - 1)]
												& 0x000E)
												>> 1;

				if (kTaEe > 3)
								kTaEe -= 8;

				return kTaEe;
}

// Row 1 - 24, Col 1 - 32
uint16_t getKTaRcEe(uint8_t pixelRow, uint8_t pixelCol)
{
				uint16_t kTaRcEe = 0;

				if (pixelRow % 2 != 0 && pixelCol % 2 != 0)
				{
								kTaRcEe = (mlxData.eepromData.kTaAvg[0] & 0xFF00) >> 8;
				}
				else if (pixelRow % 2 == 0 && pixelCol % 2 != 0)
				{
								kTaRcEe = mlxData.eepromData.kTaAvg[0] & 0x00FF;
				}
				else if (pixelRow % 2 != 0 && pixelCol % 2 == 0)
				{
								kTaRcEe = (mlxData.eepromData.kTaAvg[1] & 0xFF00) >> 8;
				}
				else if (pixelRow % 2 == 0 && pixelCol % 2 == 0)
				{
								kTaRcEe = mlxData.eepromData.kTaAvg[1] & 0x00FF;
				}

				if (kTaRcEe > 127)
								kTaRcEe -= 256;

				return kTaRcEe;
}

double getKTaij(uint8_t pixelRow, uint8_t pixelCol)
{
				double kTaij = 0;

				kTaij = ((double)getKTaRcEe(pixelRow, pixelCol) +
												((double)getKTaEe(pixelRow, pixelCol) *
																				pow(2, mlxData.eepromData.kTaScale2))) /
																				pow(2, mlxData.eepromData.kTaScale1);

				return kTaij;
}

double getAlphaij(uint8_t pixelRow, uint8_t pixelCol)
{
				double alphaij = 0;
				double dividend =
												mlxData.eepromData.alphaRef +
												mlxData.eepromData.accRows[pixelRow - 1] *
												pow(2, mlxData.eepromData.accScaleRow) +
												mlxData.eepromData.accCols[pixelCol - 1] *
												pow(2, mlxData.eepromData.accScaleCol) +
												getAlphaPixel(pixelRow, pixelCol) *
												pow(2, mlxData.eepromData.accScaleRemnant);


				double divisor = pow(2, mlxData.eepromData.alphaScale);

				alphaij = dividend / divisor;
				return alphaij;
}

double getKvij(uint8_t pixelRow, uint8_t pixelCol)
{
				int16_t kVij = 0;
				double retKvij = 0;

				if (pixelRow % 2 != 0 && pixelCol % 2 != 0)
				{
								kVij = (mlxData.eepromData.kvAvg & 0xF000) >> 12;
				}
				else if (pixelRow % 2 == 0 && pixelCol % 2 != 0)
				{
								kVij = (mlxData.eepromData.kvAvg & 0x0F00) >> 8;
				}
				else if (pixelRow % 2 != 0 && pixelCol % 2 == 0)
				{
								kVij = (mlxData.eepromData.kvAvg & 0x00F0) >> 4;
				}
				else if (pixelRow % 2 == 0 && pixelCol % 2 == 0)
				{
								kVij = (mlxData.eepromData.kvAvg & 0x00F);
				}

				if (kVij > 7)
								kVij -= 16;

				retKvij = kVij / pow(2, mlxData.eepromData.kvScale);

				return retKvij;
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
				mlxData.calcuData.resolutionReg = (controlRegister & 0x0C00) >> 10;

				mlxData.calcuData.resolutionCorr =
																				pow(2, mlxData.eepromData.resolutionEe) /
																				pow(2, mlxData.calcuData.resolutionReg);

				return MLX_OK;
}

MLX90640_StatusTypedef supplyVoltageCalculations()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t ram072a = 0;
				int16_t tempRam072a = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_072A,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram072a,
																														sizeof(ram072a),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("supplyVoltageCalculations HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				ram072a = swap_uint16(ram072a);
				tempRam072a = ram072a;

				mlxData.calcuData.Vdd =
												(((mlxData.calcuData.resolutionCorr *
																				(double)(tempRam072a)) - mlxData.eepromData.Vdd25) /
																				(double)mlxData.eepromData.kVdd)
																				+ 3.3;

				return MLX_OK;
}

MLX90640_StatusTypedef ambientTempCalculations()
{
				HAL_StatusTypeDef status = HAL_OK;

				uint16_t ram072a = 0;
				int16_t tempRam072a = 0;

				uint16_t ram0720 = 0;
				uint16_t ram0700 = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_072A,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram072a,
																														sizeof(ram072a),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("ambientTempCalculations HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				ram072a = swap_uint16(ram072a);
				tempRam072a = ram072a;

				mlxData.calcuData.deltaV = (double)(tempRam072a -
												mlxData.eepromData.Vdd25) / mlxData.eepromData.kVdd;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0720,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0720,
																														sizeof(ram0720),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("ambientTempCalculations HAL NOT OK 4!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0720 = swap_uint16(ram0720);
				mlxData.calcuData.vPtat = ram0720;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0700,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0700,
																														sizeof(ram0700),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("ambientTempCalculations HAL NOT OK 5!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0700 = swap_uint16(ram0700);
				mlxData.calcuData.vBe = ram0700;

				mlxData.calcuData.vPtatArt =
												(double)((double)mlxData.calcuData.vPtat /
												((double)mlxData.calcuData.vPtat *
												 (double)mlxData.eepromData.AlphaPtat +
												 (double)mlxData.calcuData.vBe))
													* 262144.;

				mlxData.calcuData.ta =
								((mlxData.calcuData.vPtatArt /
								 (1. + mlxData.eepromData.KvPtat *
									 mlxData.calcuData.deltaV)) -
												mlxData.eepromData.Vptat25) /
																mlxData.eepromData.KtPtat + 25.;

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
				mlxData.calcuData.kGain = (double)mlxData.eepromData.gain / (double)ram070a;

				return MLX_OK;
}



MLX90640_StatusTypedef getPixelDataFromRam()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t i = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0400,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&mlxData.pixelRawData,
																														sizeof(mlxData.pixelRawData),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("getPixelDataFromRam HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				for (i = 0; i < 768; i++)
								mlxData.pixelRawData[i] = swap_uint16(mlxData.pixelRawData[i]);


				printf("getPixelDataFromRam: pix(12,16): %d\r\n",
								       (int16_t)mlxData.pixelRawData[11*32 + 15]);

				return MLX_OK;
}

MLX90640_StatusTypedef calculateGainCompensation()
{
				int16_t i = 0;

				for (i = 0; i < 768; i++)
				{
								mlxData.tempData[i] =
																(double)((int16_t)mlxData.pixelRawData[i]) * mlxData.calcuData.kGain;
				}

				printf("calculateGainComp: pix(12,16): %.5f\r\n",
				       mlxData.tempData[11*32 + 15]);

				return MLX_OK;
}

int16_t getPixOsref(uint8_t pixelRow, uint8_t pixelCol)
{
				int16_t pixOsRef = 0;

				pixOsRef =
												mlxData.eepromData.offsetAvg +
												mlxData.eepromData.occRows[pixelRow-1] *
												pow(2, mlxData.eepromData.occScaleRow) +
												mlxData.eepromData.occCols[pixelCol-1] *
												pow(2, mlxData.eepromData.occScaleCol) +
												getPixelOffset(pixelRow, pixelCol) *
												pow(2, mlxData.eepromData.occScaleRemnant);

				return pixOsRef;
}

MLX90640_StatusTypedef calculateIrDataCompensation()
{
				uint16_t i = 0;

				for (i = 0; i < 768; i++)
				{
								uint8_t row = i / 32 + 1;
								uint8_t col = i % 32 + 1;
								// printf("[%d %d] ", row, col);
								// Given that calculateGainCompensation has been done so that
								// tempData contains pixGain
								mlxData.tempData[i] =
																mlxData.tempData[i] - getPixOsref(row, col) *
																(1. + getKTaij(row, col) * (mlxData.calcuData.ta - 25.)) *
																(1. + getKvij(row, col) * (mlxData.calcuData.Vdd - 3.3));
				}

				printf("\r\n");

				printf("calculateIrDataCompensation: pix(12,16): %.7f\r\n",
				       mlxData.tempData[11*32 + 15]);

				return MLX_OK;
}

MLX90640_StatusTypedef compensateGainOfCpPixel()
{
				HAL_StatusTypeDef status = HAL_OK;
				uint16_t ram0708 = 0;
				uint16_t ram0728 = 0;

				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0708,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0708,
																														sizeof(ram0708),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("compensateGainOfCpPixel HAL NOT OK!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0708 = swap_uint16(ram0708);

				mlxData.calcuData.pixGainCpSp0 =
												(int16_t)ram0708 * mlxData.calcuData.kGain;


				status = HAL_I2C_Mem_Read(&hi2c1,
																														mlx90640Address << 1,
																														MLX90640_RAM_0728,
																														I2C_MEMADD_SIZE_16BIT,
																														(uint8_t*)&ram0728,
																														sizeof(ram0728),
																														TIMEOUT_MS);

				if (status != HAL_OK)
				{
								printf("compensateGainOfCpPixel HAL NOT OK2!\r\n");
								return MLX_HAL_ERROR;
				}

				ram0728 = swap_uint16(ram0728);

				mlxData.calcuData.pixGainCpSp1 =
												(int16_t)ram0728 * mlxData.calcuData.kGain;

				return MLX_OK;
}

MLX90640_StatusTypedef compensateOffsetTaAndVddOfCpPixel()
{
				mlxData.calcuData.pixOsCpSp0 =
												mlxData.calcuData.pixGainCpSp0 -
												mlxData.eepromData.offCpSubpage0 *
												(1. + mlxData.eepromData.kTaCp * (mlxData.calcuData.ta - 25.)) *
												(1. + mlxData.eepromData.kvCp * (mlxData.calcuData.Vdd - 3.3));


				mlxData.calcuData.pixOsCpSp1 =
												mlxData.calcuData.pixGainCpSp1 -
												mlxData.eepromData.offCpSubpage1 *
												(1. + mlxData.eepromData.kTaCp * (mlxData.calcuData.ta - 25.)) *
												(1. + mlxData.eepromData.kvCp * (mlxData.calcuData.Vdd - 3.3));

				return MLX_OK;
}

MLX90640_StatusTypedef iRDataGradientCompensation()
{
				uint16_t i = 0;

				for (i = 0; i < 768; i++)
				{
								mlxData.tempData[i] =
																mlxData.tempData[i] - mlxData.eepromData.tgc *
																(1. - PATTERN(i+1) * mlxData.calcuData.pixOsCpSp0 +
																						PATTERN(i+1) * mlxData.calcuData.pixOsCpSp1);
				}

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
