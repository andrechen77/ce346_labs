#include "ultrasonic.h"
#include "timer.h"
#include "pin_defs.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define SOUND_SPEED 0.034 // (cm / us)

#define ULTRASONIC_TIMEOUT_DISTANCE 100 // cm
#define ULTRASONIC_TIMEOUT_US (ULTRASONIC_TIMEOUT_DISTANCE / SOUND_SPEED * 2)

#define NUM_MEASUREMENT_SAMPLES_PER_SENSOR 1

ultrasonic_t ultrasonic_sensors[] = {
	{.trig_pin = EDGE_P12_IDX, .echo_pin = EDGE_P13_IDX, .timeout_us = ULTRASONIC_TIMEOUT_US},
	// {.trig_pin = EDGE_P9_IDX, .echo_pin = EDGE_P10_IDX, .timeout_us = ULTRASONIC_TIMEOUT_US},
};
int num_sensors = sizeof(ultrasonic_sensors) / sizeof(ultrasonic_sensors[0]);

const struct device* gpio_dev = DEVICE_DT_GET(GPIO_PORT_0);

void echo_interrupt_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
	ultrasonic_t* sensor = CONTAINER_OF(cb, ultrasonic_t, gpio_cb);

	if (sensor->ready == 0) {
		sensor->echo_start_us = read_timer_us();
		sensor->ready = 1;
	} else if (sensor->ready == 1) {
		sensor->echo_end_us = read_timer_us();
		sensor->ready = 2;
	}
}

void ultrasonic_init() {
	// Initialize GPIO
	if (!gpio_dev) {
		fprintf(stderr, "Error: GPIO device not found\n");
		return;
	}

	for (int i = 0; i < num_sensors; i++) {
		fprintf(stderr, "Configuring ultrasonic sensor %d\n", i);

		ultrasonic_t* conf = &ultrasonic_sensors[i];

		gpio_pin_configure(gpio_dev, conf->trig_pin, GPIO_OUTPUT_LOW);
		gpio_pin_configure(gpio_dev, conf->echo_pin, GPIO_INPUT);
		gpio_pin_interrupt_configure(gpio_dev, conf->echo_pin, GPIO_INT_EDGE_BOTH);
		gpio_init_callback(&conf->gpio_cb, echo_interrupt_handler, BIT(conf->echo_pin));
		gpio_add_callback(gpio_dev, &conf->gpio_cb);
	}
}

double sample_distance_cm(ultrasonic_t* sensor) {
	fprintf(stderr, "sampling distance\n");
	// uint32_t start = read_timer_us();

	// prepare the data structure
	sensor->ready = 0;

	// trigger a pulse
	gpio_pin_set(gpio_dev, sensor->trig_pin, 0);
	k_usleep(2);
	gpio_pin_set(gpio_dev, sensor->trig_pin, 1);
	k_usleep(10);
	gpio_pin_set(gpio_dev, sensor->trig_pin, 0);

	while (sensor->ready != 2) {
		k_yield();
	}

	// calculate distance
	double duration_us = sensor->echo_end_us - sensor->echo_start_us;
	double distance_cm = duration_us * SOUND_SPEED / 2;
	// fprintf(stderr, "distance as int: %d cm\n", (int)(distance_cm));

	return distance_cm;
}

int double_cmp(const void* a, const void* b) {
	double da = *(double*)a;
	double db = *(double*)b;

	if (da < db) {
		return -1;
	} else if (db > da) {
		return 1;
	}

	return 0;
}

double sample_sensors_distance_cm() {
	int num_measurements = NUM_MEASUREMENT_SAMPLES_PER_SENSOR * num_sensors;
	double measurements[num_measurements];

	for (int i = 0; i < num_sensors; i++) {
		for (int j = 0; j < NUM_MEASUREMENT_SAMPLES_PER_SENSOR; j++) {
			measurements[i * NUM_MEASUREMENT_SAMPLES_PER_SENSOR + j] = sample_distance_cm(&ultrasonic_sensors[i]);
		}
	}

	qsort(measurements, num_measurements, sizeof(double), double_cmp);

	if (num_measurements % 2) {
		return measurements[num_measurements / 2];
	} else {
		return (measurements[num_measurements / 2] + measurements[num_measurements / 2 - 1]) / 2;
	}
}
