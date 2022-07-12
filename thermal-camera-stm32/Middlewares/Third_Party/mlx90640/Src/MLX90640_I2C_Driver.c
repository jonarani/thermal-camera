/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "MLX90640_I2C_Driver.h"

static I2C_HandleTypeDef *hi2c = NULL;
static uint8_t i2cData[1664] = {0};

void MLX90640_I2CInit(I2C_HandleTypeDef *i2cHandle)
{
	hi2c = i2cHandle;
    return;
}

uint8_t MLX90640_I2CGeneralReset(void)
{
    uint8_t cmd[2] = {0x06, 0x00};

    HAL_I2C_Slave_Transmit(hi2c, &cmd[0], sizeof(cmd), 3000);

    HAL_Delay(1);

    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t sa = 0;
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t cnt = 0;
    uint16_t i = 0;
    uint16_t *p;
    
    p = data;
    sa = (slaveAddr << 1);
    
    status = HAL_I2C_Mem_Read(hi2c, sa, startAddress, I2C_MEMADD_SIZE_16BIT, i2cData, nMemAddressRead * 2, 3000);

    if (status != HAL_OK)
    {
		return -status;
    }

    for (cnt = 0; cnt < nMemAddressRead; cnt++)
    {
        i = cnt << 1;
        *p++ = ((uint16_t)i2cData[i] << 8) + (uint16_t)i2cData[i+1];
    }
    
    return 0;   
} 

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t sa;

    sa = (slaveAddr << 1);

    data = (data << 8) | (data >> 8);

	HAL_I2C_Mem_Write(hi2c, sa, writeAddress, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&data, sizeof(data), 3000);

	// TODO: can check, but too much overhead
//    static uint16_t dataCheck;
//    MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);
//
//    if ( dataCheck != data)
//    {
//        return -2;
//    }
    
    return 0;
}

