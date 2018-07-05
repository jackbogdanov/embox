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

#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/printk.h>

#include "stm32f4_discovery.h"

#define IR_GPIO               GPIOD
#define IR_CLK_ENABLE()       __GPIOD_CLK_ENABLE()

#define IR_LED_PIN            GPIO_PIN_3
#define IR_RECV_OUT           GPIO_PIN_1

static int ir_signals_cnt;

static void stm32_delay(uint32_t delay) {
	while (--delay > 0)
		;
}

static void init_leds(void) {
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
}

static irq_return_t exti1_handler(unsigned int irq_nr, void *data) {
	if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) {
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
	}

	printk("IR signal received (total count = %d)\n", ir_signals_cnt++);

	return IRQ_HANDLED;
}

static void ir_init(void) {
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

static void ir_test(void) {
	stm32_delay(100000);
	HAL_GPIO_WritePin(IR_GPIO, IR_LED_PIN, GPIO_PIN_SET);

	while (1) {
	}
}

int main(int argc, char *argv[]) {
	printf("IR sensors test start!\n");

	//HAL_Init();

	init_leds();
    BSP_LED_Toggle(LED5);

	ir_init();
	ir_test();

	return 0;
}
