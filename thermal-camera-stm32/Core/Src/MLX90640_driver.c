/*
 * MLX90640_driver.c
 *
 *  Created on: Jul 6, 2022
 *      Author: Jonathan
 */


#include <MLX90640_driver.h>
#include "main.h"
#include <stdio.h>

// Default address is 0x33
uint8_t mlx90640Address = 0x33;

extern I2C_HandleTypeDef hi2c1;

#define TIMEOUT_MS (3000)

static uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8 );
}

MLX90640_StatusTypedef setRefreshRate(MLX90640_RefreshRateTypedef refreshRate)
{
	if (refreshRate < MLX_0_5_HZ || refreshRate > MLX_64_HZ)
	{
		return MLX_INVALID_PARAM;
	}

	HAL_StatusTypeDef status = HAL_OK;

	uint16_t controlRegister = 0;

	status = HAL_I2C_Mem_Read(&hi2c1,
							  mlx90640Address << 1,
							  0x240C,
							  I2C_MEMADD_SIZE_16BIT,
							  (uint8_t*)&controlRegister,
							  sizeof(controlRegister),
							  TIMEOUT_MS);

	if (status != HAL_OK)
	{
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
		return MLX_HAL_ERROR;
	}

	return MLX_OK;
}
