#include "pin_defs.h"
#include "ultrasonic.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "pwm.h"
#include "adc.h"

#define ADC_THREAD_STACK_SIZE 1024  // Adjust based on usage
#define ADC_THREAD_PRIORITY 5       // Higher number = lower priority
#define ADC_THRESHOLD 100

K_THREAD_STACK_DEFINE(adc_thread_stack, ADC_THREAD_STACK_SIZE);
static struct k_thread adc_thread_data;

void adc_thread(void* p1, void* p2, void* p3) {
	while (1) {
		int val = adcread();
		if (val > ADC_THRESHOLD) {
			// set duty to something
			printf("Hello world!\n");
			pwm_set_duty((float)val / 4096.0);
		} else {
			// set duty to 0
			pwm_set_duty(0);
		}
		k_sleep(K_MSEC(10));
	}
}


int main(void) {
	fprintf(stderr, "Hello, there!\n");
	k_sleep(K_MSEC(10)); // let the RTOS do its thing

	// Initialize the ADC
	if (adc_begin() != 0) {
		fprintf(stderr, "Error initializing ADC\n");
		return -1;
	}
	fprintf(stderr, "ADC initialized\n");

	// Initialize the PWM
	pwm_init();
	fprintf(stderr, "PWM initialized\n");

	k_thread_create(&adc_thread_data, adc_thread_stack,
		K_THREAD_STACK_SIZEOF(adc_thread_stack),
		adc_thread, NULL, NULL, NULL,
		ADC_THREAD_PRIORITY, 0, K_NO_WAIT);

	// while (1) {
	// 	pwm_set_duty(0);
	// 	k_sleep(K_SECONDS(1));
	// 	pwm_set_duty(0.5);
	// 	k_sleep(K_SECONDS(1));
	// }


	// while (1) {
	// 	int reading = adcread();
	// 	fprintf(stderr, "ADC reading: %d\n", reading);
	// 	k_sleep(K_SECONDS(1));
	// }

	// fprintf(stderr, "Initializing ultrasonic sensors\n");
	// ultrasonic_init();
	// fprintf(stderr, "Ultrasonic sensors initialized\n");
	// k_sleep(K_MSEC(1000));

	// while (1) {
	// 	double distance = sample_sensors_distance_cm();
	// 	fprintf(stderr, "Distance: %f cm\n", distance);
	// 	k_sleep(K_MSEC(1000));
	// }

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return -1;
}