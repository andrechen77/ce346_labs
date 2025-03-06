#include <stdint.h>
#include <stdbool.h>
#include <zephyr/drivers/gpio.h>

typedef struct {
	// configuration
	uint32_t trig_pin;
	uint32_t echo_pin;
	uint32_t timeout_us;

	// status during operation
	uint32_t echo_start_us;
	uint32_t echo_end_us;
	uint8_t ready; // 0 if waiting for echo start, 1 if waiting for echo end, 2 if ready
	struct gpio_callback gpio_cb;
} ultrasonic_t;

void ultrasonic_init();
double sample_distance_cm(ultrasonic_t* sensor);
int double_cmp(const void* a, const void* b);
double sample_sensors_distance_cm();
