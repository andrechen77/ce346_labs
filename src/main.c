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
#include "display.h"
#include "ble.h"

#define THREAD_STACK_SIZE 1024  // Adjust based on usage
#define THREAD_PRIORITY 5       // Higher number = lower priority
#define ADC_THRESHOLD 100

K_THREAD_STACK_DEFINE(adc_thread_stack, THREAD_STACK_SIZE);
static struct k_thread adc_thread_data;

K_THREAD_STACK_DEFINE(timer_feed_thread_stack, THREAD_STACK_SIZE);
static struct k_thread timer_feed_thread_data;

K_THREAD_STACK_DEFINE(food_level_thread_stack, THREAD_STACK_SIZE);
static struct k_thread food_level_thread_data;

void adc_thread(void* p1, void* p2, void* p3) {
	bool has_pwm_lock = false;
	bool has_disp_lock = false;
	while (1) {
		int val = adcread();
		if (val > ADC_THRESHOLD) {
			// set duty to something
			// if (has_lock) {
			// 	pwm_set_duty((float)val / 4096.0f);
			// } else if (pwm_lock(100) == 0) {
			// 	has_lock = true;
			// 	pwm_set_duty((float)val / 4096.0f);
			// } else {
			// 	has_lock = false;
			// }
			if (!has_pwm_lock) {
				if (pwm_lock(100) == 0) {
					has_pwm_lock = true;
				}
			}
			if (has_pwm_lock) {
				float percentage = (float)val / 4096.0f;
				pwm_set_duty(percentage);

				if (!has_disp_lock) {
					if (display_lock() == 0) {
						has_disp_lock = true;
					}
				}
				if (has_disp_lock) {
					write_row_pattern((int)(percentage * 25));
				}
			}
		} else {
			// set duty to 0
			if (has_pwm_lock) {
				pwm_set_duty(0);
				pwm_unlock();
				has_pwm_lock = false;
			}
			if (has_disp_lock) {
				write_row_pattern(0);
				display_unlock();
				has_disp_lock = false;
			}
		}
		k_sleep(K_MSEC(100));
	}
}

void timer_feed_thread(void* p1, void* p2, void* p3) {
	info_t* info = get_info();
	while (1) {
		k_mutex_lock(info->mut, K_FOREVER);
		uint32_t cur_time = read_timer_s();
		uint32_t next_feed_time = info->_next_feed_timestamp_sec;
		uint32_t feed_dur = info->feed_duration_sec;
		// fprintf(stderr, "next feed time: %d, current time %d\n", next_feed_time, cur_time);
		k_mutex_unlock(info->mut);

		if (cur_time >= next_feed_time) {
			// feed
			if (pwm_lock(1) == 0) {
				fprintf(stderr, "Feeding for %d seconds\n", feed_dur);
				pwm_set_duty(0.5);
				k_sleep(K_SECONDS(feed_dur));
				pwm_set_duty(0);
				pwm_unlock();
			} else {
				fprintf(stderr, "timer feed:Failed to lock PWM\n");
				k_sleep(K_SECONDS(feed_dur));
			}

			k_mutex_lock(info->mut, K_FOREVER);
			info->_last_feed_timestamp_sec = cur_time;
			info->_next_feed_timestamp_sec = cur_time + info->feed_freq_sec;
			k_mutex_unlock(info->mut);
		}

		k_sleep(K_MSEC(100));
	}
}

void food_level_thread(void* p1, void* p2, void* p3) {
	info_t* info = get_info();
	while (1) {
		// check food level
		double distance = sample_sensors_distance_cm();
		fprintf(stderr, "Distance: %f\n", distance);

		k_mutex_lock(info->mut, K_FOREVER);
		info->is_food_low = distance >= 15.0;
		bt_gatt_notify(NULL, &custom_svc.attrs[4], &get_info()->is_food_low, 1);
		k_mutex_unlock(info->mut);

		k_sleep(K_SECONDS(3));
	}
}

int ble_main(void);

int main(void) {
	fprintf(stderr, "Hello, there!\n");
	k_sleep(K_MSEC(10)); // let the RTOS do its thing

	display_init();
	write_row_pattern(25);

	pwm_init();
	pwm_set_duty(0);

	ultrasonic_init();

	if (ble_init()) {
		fprintf(stderr, "Error initializing BLE\n");
		return -1;
	}
	fprintf(stderr, "BLE initialized\n");

	info_t* info = get_info();

	// Initialize the ADC
	if (adc_begin() != 0) {
		fprintf(stderr, "Error initializing ADC\n");
		return -1;
	}
	fprintf(stderr, "ADC initialized\n");

	k_thread_create(&adc_thread_data, adc_thread_stack,
		K_THREAD_STACK_SIZEOF(adc_thread_stack),
		adc_thread, NULL, NULL, NULL,
		THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_create(&timer_feed_thread_data, timer_feed_thread_stack,
		K_THREAD_STACK_SIZEOF(timer_feed_thread_stack),
		timer_feed_thread, NULL, NULL, NULL,
		THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_create(&food_level_thread_data, food_level_thread_stack,
		K_THREAD_STACK_SIZEOF(food_level_thread_stack),
		food_level_thread, NULL, NULL, NULL,
		THREAD_PRIORITY, 0, K_NO_WAIT);

	while (1) {
		// simulate counting down to feeding
		k_mutex_lock(info->mut, K_FOREVER);

		uint32_t cur_time = read_timer_s();
		uint32_t period = info->feed_freq_sec;

		info->next_feed_sec = info->_next_feed_timestamp_sec - cur_time;
		info->last_feed_sec = cur_time - info->_last_feed_timestamp_sec;

		float percentage = (float)info->last_feed_sec / (float)(info->next_feed_sec + info->last_feed_sec);

		k_mutex_unlock(info->mut);

		if (display_lock() == 0) {
			write_ring_pattern((int)(percentage * 16), 0, cur_time % 2);
			display_unlock();
		}
		// write_row_pattern(cur_time);

		k_sleep(K_MSEC(100));
	}

	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return -1;
}

