/**
 * @file
 * @brief
 *
 * @date 11.01.2017
 * @author Alex Kalmuk
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <util/log.h>
#include <embox/unit.h>
#include <kernel/thread.h>
#include <kernel/time/ktime.h>

#include "stm32f4_discovery.h"
#include <libs/stepper_motor.h>
#include <libs/ir.h>

#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

extern void nrf24_enable(void);
extern uint16_t gy_30_read_light_level(void);
extern int gy_30_enable(void);

static void stm32_delay(uint32_t delay) {
	while (--delay > 0)
		;
}

static void init_leds() {
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
}

static void *light_sensor_loop(void *arg) {
	uint16_t level;
	int i;

	while (1) {
		for (i = 0; i < 10; i++) {
			level = gy_30_read_light_level();
			printf("light (lx): %d\n", level);
			stm32_delay(1000);
		}
		ksleep(10000);
	}
	return NULL;
}

static void main_loop(void) {
	struct stepper_motor motor1, motor2;
	struct thread *th;

	__GPIOD_CLK_ENABLE(); /* for motors */
	__GPIOE_CLK_ENABLE(); /* for motors */

	nrf24_enable();
	motor_init(&motor1, GPIO_PIN_0, GPIO_PIN_2, GPIO_PIN_4, GPIO_PIN_6, GPIOD);
	motor_init(&motor2, GPIO_PIN_8, GPIO_PIN_10, GPIO_PIN_12, GPIO_PIN_14, GPIOE);
	motor_set_speed(&motor1, MOTOR_MAX_SPEED);
	motor_set_speed(&motor2, MOTOR_MAX_SPEED);
	ir_init();
	ir_enable();
	gy_30_enable();

	th = thread_create(THREAD_FLAG_SUSPENDED, light_sensor_loop, NULL);
	if (!th) {
		log_error("Failed to create thread\n");
		return;
	}
	if (thread_launch(th) != 0) {
		log_error("Failed to launch thread\n");
		return;
	}
	if (thread_detach(th) != 0) {
		log_error("Failed to detach thread\n");
		return;
	}
	stm32_delay(1000000);
	while (1) {
		motor_do_steps2(&motor1, &motor2, 2 * MOTOR_STEPS_PER_REVOLUTION, MOTOR_RUN_BACKWARD);
		stm32_delay(5000000);
	}
}

int main(int argc, char *argv[]) {
	printf("Robot test start!\n");

	init_leds();
	BSP_LED_Toggle(LED6);
	HAL_GPIO_WritePin(IR_GPIO, IR_LED_PIN, GPIO_PIN_SET);

	main_loop();

	return 0;
}
