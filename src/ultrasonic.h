#include <stdint.h>

typedef struct {
	uint32_t trig_pin;
	uint32_t echo_pin;
	uint32_t timeout_us;
} ultrasonic_conf_t;

void ultrasonic_init();
double sample_distance_cm(ultrasonic_conf_t* conf);
int double_cmp(const void* a, const void* b);
double sample_sensors_distance_cm();
