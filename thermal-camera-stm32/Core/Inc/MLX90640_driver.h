/*
 * MLX90640-driver.h
 *
 *  Created on: Jul 6, 2022
 *      Author: Jonathan
 */

#ifndef INC_MLX90640_DRIVER_H_
#define INC_MLX90640_DRIVER_H_

#include <stdint.h>


typedef enum MLX90640_Statuses {
	MLX_OK = 0,
	MLX_INVALID_PARAM,
	MLX_HAL_ERROR
} MLX90640_StatusTypedef;

typedef enum MLX90640_RefreshRates {
  MLX_0_5_HZ,
  MLX_1_HZ,
  MLX_2_HZ,
  MLX_4_HZ,
  MLX_8_HZ,
  MLX_16_HZ,
  MLX_32_HZ,
  MLX_64_HZ,
} MLX90640_RefreshRateTypedef;


#define MLX90640_PIXEL_DATA_START_ADDR 		0x2440
#define MLX90640_PIXEL_DATA_END_ADDR		0x273F

#define MLX90640_EEPROM_REFRESH_RATE_ADDR 	0x240C
#define MLX90640_CONTROL_REGISTER_1 		0x800D

#define FRAME_ROWS 24
#define FRAME_COLS 32
#define NUM_OF_PIXELS (FRAME_ROWS * FRAME_COLS)


extern uint8_t mlx90640Address;


// Default refresh rate is 2Hz.
// All available options: 0.5, 1, 2, 4, 8, 16, 32 and 64Hz
// Since the argument is uint8_t then 0.5 is not possible in this case.
MLX90640_StatusTypedef setRefreshRate(MLX90640_RefreshRateTypedef refreshRate);


#endif /* INC_MLX90640_DRIVER_H_ */
