/**
 * @file
 *
 * @data 29.03.2017
 * @author Alex Kalmuk
 */

#ifndef PLATFORM_STM32F4_MULTIBOTS_CMDS_GY_30_H_
#define PLATFORM_STM32F4_MULTIBOTS_CMDS_GY_30_H_

#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"

// Initialize and config
extern int gy_30_enable(void);
// Config only
extern int gy_30_config(uint8_t mode);
extern uint16_t gy_30_read_light_level(void);

#endif /* PLATFORM_STM32F4_MULTIBOTS_CMDS_GY_30_H_ */
