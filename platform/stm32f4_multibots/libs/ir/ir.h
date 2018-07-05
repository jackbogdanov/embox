/**
 * @file
 * @brief
 *
 * @date 11.01.2017
 * @author Alex Kalmuk
 */

#ifndef IR_H_
#define IR_H_

#include "stm32f4_discovery.h"

#define IR_GPIO               GPIOD
#define IR_CLK_ENABLE()       __GPIOD_CLK_ENABLE()

#define IR_LED_PIN            GPIO_PIN_3
#define IR_RECV_OUT           GPIO_PIN_1

extern void ir_init(void);

static inline void ir_enable(void) {
	HAL_GPIO_WritePin(IR_GPIO, IR_LED_PIN, GPIO_PIN_SET);
}

#endif /* IR_H_ */
