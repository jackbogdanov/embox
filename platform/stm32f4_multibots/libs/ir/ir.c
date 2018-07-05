/**
 * @file
 * @brief
 *
 * @date 10.01.2017
 * @author Alex Kalmuk
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <kernel/irq.h>
#include <kernel/printk.h>

#include "stm32f4_discovery.h"
#include "ir.h"

static int ir_signals_cnt;

static irq_return_t exti1_handler(unsigned int irq_nr, void *data) {
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) {
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
	}

	printk("IR signal received (total count = %d)\n", ir_signals_cnt++);

	return IRQ_HANDLED;
}

void ir_init(void) {
	GPIO_InitTypeDef  GPIO_InitStruct;

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));

	IR_CLK_ENABLE();

	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Pin       = IR_LED_PIN;
	HAL_GPIO_Init(IR_GPIO, &GPIO_InitStruct);
	
	/* Configure PD1 pin as input floating */
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pin  = IR_RECV_OUT;
	HAL_GPIO_Init(IR_GPIO, &GPIO_InitStruct);

	HAL_GPIO_WritePin(IR_GPIO, IR_LED_PIN, GPIO_PIN_RESET);
	
	irq_attach(EXTI1_IRQn + 16, exti1_handler, 0, NULL, "ir handler");

	printf("IR pins inited\n");
}
