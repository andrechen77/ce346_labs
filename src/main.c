#include "pin_defs.h"
#include "ultrasonic.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "info.h"
#include "timer.h"
#include "pwm.h"
#include "adc.h"
#include "ble.h"

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

    if (ble_init()) {
        fprintf(stderr, "Error initializing BLE\n");
		return -1;
	}
	fprintf(stderr, "BLE initialized\n");

    info_t* info = get_info();

    uint32_t start_time = read_timer_s();

    while (1) {
        // simulate counting down to feeding
        k_mutex_lock(info->mut, K_FOREVER);

        uint32_t cur_time = read_timer_s();

        long remaining_time = (long)(start_time + info->feed_freq_sec) - (long)cur_time;

        if (remaining_time < 0) {
            start_time = cur_time;
            info->next_feed_sec = info->feed_freq_sec;
        } else {
            info->next_feed_sec = remaining_time;
        }

        info->last_feed_sec = cur_time - start_time;

        k_mutex_unlock(info->mut);

        if (remaining_time < 0) {
            // do the actual feeding here
        }

		k_sleep(K_MSEC(100));
	}

    // TODO: Fix the pins so it doesn't interfere with BLE!!

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

