#include "pin_defs.h"
#include "ultrasonic.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "pwm.h"
#include "adc.h"


int main(void) {
	fprintf(stderr, "Hello, there!\n");
	k_sleep(K_MSEC(10)); // let the RTOS do its thing

	// Initialize the ADC
	if (adc_begin() != 0) {
		fprintf(stderr, "Error initializing ADC\n");
		return -1;
	}
	fprintf(stderr, "ADC initialized\n");

	while (1) {
		int reading = adcread();
		fprintf(stderr, "ADC reading: %d\n", reading);
		k_sleep(K_SECONDS(1));
	}

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