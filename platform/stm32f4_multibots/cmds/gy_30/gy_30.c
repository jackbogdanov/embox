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
#include <libs/gy_30.h>

static void init_leds(void) {
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
}

static void stm32f4_delay(int i) {
	while(--i > 0)
		;
}

int main(int argc, char *argv[]) {
	int res;
	uint16_t level = 0;

	printf("GY-30 running\n");

	init_leds();
	res = gy_30_enable();
	if (res < 0) {
		return -1;
	}
	BSP_LED_Toggle(LED3);

	while (1) {
		level = gy_30_read_light_level();
		printf("level: %d\n", (level * 5) / 6);
		stm32f4_delay(5000000);
	}

	return 0;
}
