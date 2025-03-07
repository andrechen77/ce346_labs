#include "pin_defs.h"
#include "ultrasonic.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>


struct gpio_callback gpio_cb;
void interrupt_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
	fprintf(stderr, "echo interrupt\n");
}

int main(void) {
	fprintf(stderr, "Hello, there!\n");
	k_sleep(K_MSEC(10)); // let the RTOS do its thing

	// gpio_pin_configure(EDGE_P8_DEV, EDGE_P8_IDX, GPIO_OUTPUT_LOW);

	// while (1) {
	// 	gpio_pin_set(EDGE_P8_DEV, EDGE_P8_IDX, 1);
	// 	k_sleep(K_MSEC(500));
	// 	gpio_pin_set(EDGE_P8_DEV, EDGE_P8_IDX, 0);
	// 	k_sleep(K_MSEC(500));
	// }

	// fprintf(stderr, "Configuring GPIO\n");
	// int err = gpio_pin_configure(EDGE_P8_DEV, EDGE_P8_IDX, GPIO_INPUT);
	// fprintf(stderr, "gpio_pin_configure returned %d\n", err);
	// gpio_pin_interrupt_configure(EDGE_P8_DEV, EDGE_P8_IDX, GPIO_INT_EDGE_BOTH);
	// gpio_init_callback(&gpio_cb, interrupt_handler, BIT(EDGE_P8_IDX));
	// gpio_add_callback(EDGE_P8_DEV, &gpio_cb);

	// while (1) {
	// 	k_sleep(K_SECONDS(1));
	// }

	// while (1) {
	// 	while (!gpio_pin_get(EDGE_P8_DEV, EDGE_P8_IDX)) {
	// 		fprintf(stderr, "waiting for pin to go high\n");
	// 	}
	// 	fprintf(stderr, "pin went high\n");

	// 	while (gpio_pin_get(EDGE_P8_DEV, EDGE_P8_IDX)) {
	// 		fprintf(stderr, "waiting for pin to go low\n");
	// 	}
	// 	fprintf(stderr, "pin went low\n");
	// }

	// testing stuff sstops here


	fprintf(stderr, "Initializing ultrasonic sensors\n");
	ultrasonic_init();
	fprintf(stderr, "Ultrasonic sensors initialized\n");
	k_sleep(K_MSEC(1000));

	while (1) {
		double distance = sample_sensors_distance_cm();
		fprintf(stderr, "Distance: %f cm\n", distance);
		k_sleep(K_MSEC(1000));
	}

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return -1;
}