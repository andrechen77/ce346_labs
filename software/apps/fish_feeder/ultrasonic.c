#include "ultrasonic.h"
#include "nrf_delay.h"

#define SOUND_SPEED 0.034 // (cm / us)

#define ULTRASONIC_TIMEOUT_DISTANCE 100 // cm
#define ULTRASONIC_TIMEOUT_US (ULTRASONIC_TIMEOUT_DISTANCE / SOUND_SPEED * 2)

#define NUM_MEASUREMENT_SAMPLES_PER_SENSOR 10

ultrasonic_conf_t ultrasonic_sensors[] = {
	{.trig_pin = EDGE_P7, .echo_pin = EDGE_P8, .timeout_us = ULTRASONIC_TIMEOUT_US},
	{.trig_pin = EDGE_P9, .echo_pin = EDGE_P10, .timeout_us = ULTRASONIC_TIMEOUT_US},
};
int num_sensors = sizeof(ultrasonic_sensors) / sizeof(ultrasonic_sensors[0]);

void ultrasonic_init() {
	// init gpio
	for (int i = 0; i < num_sensors; i++) {
		ultrasonic_conf_t* conf = &ultrasonic_sensors[i];
		nrf_gpio_pin_dir_set(conf->trig_pin, NRF_GPIO_PIN_DIR_OUTPUT);
		nrf_gpio_pin_dir_set(conf->echo_pin, NRF_GPIO_PIN_DIR_INPUT);
	}
}

double sample_distance_cm(ultrasonic_conf_t* conf) {
	// trigger a pulse
	nrf_gpio_pin_write(conf->trig_pin, 0);
	nrf_delay_us(2);
	nrf_gpio_pin_write(conf->trig_pin, 1);
	nrf_delay_us(10);
	nrf_gpio_pin_write(conf->trig_pin, 0);

	// wait for echo pin to go high
	while (!nrf_gpio_pin_read(conf->echo_pin));
	uint32_t start = read_timer_us();

	// poll echo
	while (nrf_gpio_pin_read(conf->echo_pin)) {
		if (read_timer_us() - start > conf->timeout_us) { // TODO consider overflow
			return INFINITY;
		}
	}

	uint32_t end = read_timer_us();

	// calculate distance
	double duration_us = end - start; // TODO consider overflow
	double distance_cm = duration_us * SOUND_SPEED / 2;

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