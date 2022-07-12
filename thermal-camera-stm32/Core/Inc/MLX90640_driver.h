/*
 * MLX90640-driver.h
 *
 *  Created on: Jul 6, 2022
 *      Author: Jonathan
 */

#ifndef INC_MLX90640_DRIVER_H_
#define INC_MLX90640_DRIVER_H_

#include <stdint.h>


typedef enum MLX90640_Statuses
{
  MLX_OK = 0,
  MLX_INVALID_PARAM,
  MLX_HAL_ERROR
} MLX90640_StatusTypedef;

typedef enum MLX90640_RefreshRates
{
  MLX_0_5_HZ,
  MLX_1_HZ,
  MLX_2_HZ,
  MLX_4_HZ,
  MLX_8_HZ,
  MLX_16_HZ,
  MLX_32_HZ,
  MLX_64_HZ,
} MLX90640_RefreshRateTypedef;


typedef struct EepromData
{
				int16_t kVdd;
				int16_t Vdd25;

				double KvPtat;
				double KtPtat;
				int16_t Vptat25;
				uint16_t kPtatAndScales;
				uint16_t AlphaPtatEe;
				uint16_t AlphaPtat;

				int16_t offsetAvg;
				int16_t offsetRows[6];
				int8_t occRows[24];
				int8_t occScaleRow;

				int16_t offsetCols[8];
				int8_t occCols[32];
				int8_t occScaleCol;

				uint8_t occScaleRemnant;

				uint16_t pixelOffsets[768];

				int16_t ilChessC1Ee;
				int16_t ilChessC1;
				int16_t ilChessC2Ee;
				int16_t ilChessC2;
				int16_t ilChessC3Ee;
				int16_t ilChessC3;

				uint16_t alphaRef;
				uint16_t scaleACCReg;
				uint8_t alphaScale;

				uint16_t accRowRegisters[6];
				int8_t accRows[24];
				uint8_t accScaleRow;

				uint16_t accColRegisters[8];
				int8_t accCols[32];
				uint8_t accScaleCol;

				uint8_t accScaleRemnant;

				uint16_t kTaAvg[2];
				uint8_t kTaScale1;
				uint8_t kTaScale2;

				int16_t gain;

				int16_t ct1;
				int16_t ct2;
				int16_t step;
				int16_t ct3;
				int16_t ct4;

				double ksTo1;
				uint16_t ksToScale;
				int16_t ksTo1Ee;

				double ksTo2;
				int16_t ksTo2Ee;

				double ksTo3;
				int16_t ksTo3Ee;

				double ksTo4;
				int16_t ksTo4Ee;

				double alphaCorrRange1;
				double alphaCorrRange2;
				double alphaCorrRange3;
				double alphaCorrRange4;

				uint16_t alphaScaleCp;
				int16_t cp_P1P0_ratio;
				double alphaCpSubpage0;
				double alphaCpSubpage1;

				int16_t offCpSubpage0;
				int16_t offCpSubpage1;
				int16_t offCpSubpage1Delta;

				uint8_t kvScale;
				int16_t kvCpEe;
				double kvCp;

				int16_t kTaCpEe;
				double kTaCp;

				uint16_t kvAvg;

				double tgc;
				uint16_t tgcEe;

				uint16_t resolutionEe;
} EepromData;

typedef struct CalculatedData
{
				uint16_t resolutionReg;
				double resolutionCorr;

				double Vdd;
				double deltaV;
				int16_t vPtat;
				int16_t vBe;
				double vPtatArt;
				double ta; // Actual sensor surrounding temperature is 8deg lower

				double kGain;

				double pixGainCpSp0;
				double pixGainCpSp1;

				double pixOsCpSp0;
				double pixOsCpSp1;

} CalculatedData;

typedef struct MLX_Data
{
				EepromData eepromData;
				CalculatedData calcuData;

				uint16_t pixelRawData[768];
				double tempData[768];
} MLX_Data;


#define MLX90640_REFRESH_RATE_ADDR 										0x240C
#define MLX90640_KPTAT_AND_SCALE_ADDR 							0x2410
#define MLX90640_PIX_OS_AVG_ADDR 												0x2411
#define MLX90640_OFFSET_ROW_ADDR 												0x2412
#define MLX90640_OFFSET_COL_ADDR 												0x2418
#define MLX90640_SCALE_ACC_ADDR 													0x2420
#define MLX90640_ALPHA_REF_ADDR 													0x2421
#define MLX90640_ACC_ROW_ADDR 															0x2422
#define MLX90640_ACC_COL_ADDR 															0x2428
#define MLX90640_GAIN_ADDR																			0x2430
#define MLX90640_PTAT_25_ADDR																0x2431
#define MLX90640_KVPTAT_KTPTAT_ADDR 									0x2432
#define MLX90640_KVDD_VDD25_ADDR 												0x2433
#define MLX90640_KV_AVG_ADDR 																0x2434
#define MLX90640_IL_CHESS_ADDR 														0x2435
#define MLX90640_KTA_AVG_ADDR 															0x2436
#define MLX90640_RES_CTRL_AND_SCALE_ADDR 				0x2438
#define MLX90640_CP_P1_P0_RATIO_ADDR 								0x2439
#define MLX90640_OFF_CP_ADDR 																0x243A
#define MLX90640_KV_KTA_CP_ADDR 													0x243B
#define MLX90640_TGC_EE_ADDR 																0x243C
#define MLX90640_KS_TO_1_2_ADDR 													0x243D
#define MLX90640_KS_TO_3_4_ADDR 													0x243E
#define MLX90640_CORNER_TEMP_ADDR 											0x243F

#define MLX90640_PIXEL_CALI_DATA_START_ADDR		0x2440

// Pixel raw data begin addr 0x0400
#define MLX90640_RAM_0400																				0x0400
#define MLX90640_RAM_0700																				0x0700
#define MLX90640_RAM_0708																				0x0708
#define MLX90640_RAM_070A																				0x070A
#define MLX90640_RAM_0720																				0x0720
#define MLX90640_RAM_0728																				0x0728
#define MLX90640_RAM_072A																				0x072A

#define MLX90640_CONTROL_REGISTER_1 									0x800D

#define NUM_ROWS_IN_FRAME	24
#define NUM_COLS_IN_FRAME 32
#define NUM_OF_PIXELS (NUM_ROWS_IN_FRAME * NUM_COLS_IN_FRAME)

#define PAT1(pixNum) ((int)((pixNum - 1) / 32))
#define PAT2(pixNum) ((int)((pixNum - 1) / 2))


// pixels from 1..768
#define PATTERN(pixNum)	(((PAT1(pixNum) - (int)(PAT1(pixNum) / 2) * 2)) ^ \
																								((pixNum - 1) - ((int)(PAT2(pixNum)) * 2)))

extern uint8_t mlx90640Address;

MLX90640_StatusTypedef extractCalibrationData();

MLX90640_StatusTypedef resolutionCalculations();
MLX90640_StatusTypedef supplyVoltageCalculations();
MLX90640_StatusTypedef ambientTempCalculations();
MLX90640_StatusTypedef gainParameterCalculation();
MLX90640_StatusTypedef getPixelDataFromRam();
MLX90640_StatusTypedef calculateGainCompensation();
int16_t getPixOsref(uint8_t pixelRow, uint8_t pixelCol);
MLX90640_StatusTypedef calculateIrDataCompensation();
MLX90640_StatusTypedef compensateGainOfCpPixel();
MLX90640_StatusTypedef compensateOffsetTaAndVddOfCpPixel();
MLX90640_StatusTypedef iRDataGradientCompensation();

// Default refresh rate is 2Hz.
// All available options: 0.5, 1, 2, 4, 8, 16, 32 and 64Hz
// Since the argument is uint8_t then 0.5 is not possible in this case.
// NB! this function is blocking
MLX90640_StatusTypedef setRefreshRate(MLX90640_RefreshRateTypedef refreshRate);
int16_t getPixelOffset(uint8_t pixelRow, uint8_t pixelCol);
int16_t getAlphaPixel(uint8_t pixelRow, uint8_t pixelCol);
int8_t getKTaEe(uint8_t pixelRow, uint8_t pixelCol);
uint16_t getKTaRcEe(uint8_t pixelRow, uint8_t pixelCol);
double getAlphaij(uint8_t pixelRow, uint8_t pixelCol);
double getKvij(uint8_t pixelRow, uint8_t pixelCol);
double getKTaij(uint8_t pixelRow, uint8_t pixelCol);

#endif /* INC_MLX90640_DRIVER_H_ */
