#include "pin_defs.h"
#include "ultrasonic.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

int main(void) {
	fprintf(stderr, "Hello, there!\n");

	fprintf(stderr, "Initializing ultrasonic sensors\n");
	ultrasonic_init();
	fprintf(stderr, "Ultrasonic sensors initialized\n");
	k_sleep(K_MSEC(1000));


	while (1) {
		double distance = sample_sensors_distance_cm();
		fprintf(stderr, "Distance: %d mm\n", (int)(distance * 10));
		k_sleep(K_MSEC(1000));
	}

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return -1;
}