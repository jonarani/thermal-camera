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

				int16_t gain;

				uint16_t resolutionEe;
} EepromData;

typedef struct TaSensorParams {
				double deltaV;
				double Vdd;
				int16_t Vptat;
				int16_t Vbe;
				double Vptatart;
				double Ta; // Actual sensor surrounding temperature is 8deg lower
} TaSensorParams;

typedef struct ResolutionParams {
				uint16_t resolutionReg;
				double resolutionCorr;
} ResolutionParams;

typedef struct CalibrationData
{
				EepromData eepromData;
				TaSensorParams taSensorParams;
				ResolutionParams resolutionsparams;
				double kGain;
} MLX_CalibrationData;


#define MLX90640_REFRESH_RATE_ADDR 										0x240C
#define MLX90640_KPTAT_AND_SCALE_ADDR 							0x2410
#define MLX90640_PIX_OS_AVG_ADDR 												0x2411
#define MLX90640_OFFSET_ROW_ADDR 												0x2412
#define MLX90640_OFFSET_COL_ADDR 												0x2418
#define MLX90640_GAIN_ADDR																			0x2430
#define MLX90640_PTAT_25_ADDR																0x2431
#define MLX90640_KVPTAT_KTPTAT_ADDR 									0x2432
#define MLX90640_KVDD_VDD25_ADDR 												0x2433
#define MLX90640_RES_CTRL_AND_SCALE_ADDR 				0x2438

#define MLX90640_PIXEL_CALI_DATA_START_ADDR		0x2440


#define MLX90640_RAM_0700																				0x0700
#define MLX90640_RAM_070A																				0x070A
#define MLX90640_RAM_0720																				0x0720
#define MLX90640_RAM_072A																				0x072A
#define MLX90640_CONTROL_REGISTER_1 									0x800D

#define NUM_ROWS_IN_FRAME	24
#define NUM_COLS_IN_FRAME 32

extern uint8_t mlx90640Address;

MLX90640_StatusTypedef extractCalibrationData();

MLX90640_StatusTypedef resolutionCalculations();
MLX90640_StatusTypedef taCalculations();
MLX90640_StatusTypedef gainParameterCalculation();

// Default refresh rate is 2Hz.
// All available options: 0.5, 1, 2, 4, 8, 16, 32 and 64Hz
// Since the argument is uint8_t then 0.5 is not possible in this case.
// NB! this function is blocking
MLX90640_StatusTypedef setRefreshRate(MLX90640_RefreshRateTypedef refreshRate);
int16_t getPixelOffset(uint8_t pixelRow, uint8_t pixelCol);



#endif /* INC_MLX90640_DRIVER_H_ */
