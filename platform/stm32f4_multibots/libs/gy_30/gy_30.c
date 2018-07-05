/**
 * @file
 * @brief
 *
 * @date    29.03.2017
 * @author  Alex Kalmuk
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <kernel/irq.h>
#include <embox/unit.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4_discovery.h"

#include <drivers/i2c/stm32f4/stm32f4_i2c.h>
#include "gy_30.h"

#define GY30_ADDR       0x46

#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

static I2C_HandleTypeDef I2cHandle;

static void stm32f4_delay(int i) {
	while(--i > 0)
		;
}

uint16_t gy_30_read_light_level(void) {
	uint16_t level = 0;

	do {
		while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
			;
		while (HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)GY30_ADDR, (void *) &level, 2, 10000) != HAL_OK)
			;
		while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
			;
	} while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);

	return (level * 5) / 6;
}

int gy_30_config(uint8_t mode) {
	do {
		while (HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)GY30_ADDR, (void *) &mode, 1, 10000) != HAL_OK)
			;
		while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
			;
	} while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);
	stm32f4_delay(5000000);
	printf("%s\n", "> gy_30_config OK\n");
	return 0;
}

int gy_30_enable(void) {
	int res;
	res = stm32f4_i2c_init(&I2cHandle, GY30_ADDR);
	if (res < 0) {
		return -1;
	}
	stm32f4_delay(5000000);
	return gy_30_config(BH1750_CONTINUOUS_HIGH_RES_MODE);
}
