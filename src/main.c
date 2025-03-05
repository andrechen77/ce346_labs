#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "pin_defs.h"
#include "ultrasonic.h"

int main(void) {
	printk("Hello, world!\n");

	const struct device* gpio_dev = EDGE_P11_DEV;

	if (!device_is_ready(gpio_dev)) {
		printk("Error: GPIO device not ready\n");
		return -1;
	}

	// Configure the GPIO pin as output
	gpio_pin_configure(gpio_dev, EDGE_P11_IDX, GPIO_OUTPUT_ACTIVE);

	while (1) {
		gpio_pin_toggle(gpio_dev, EDGE_P11_IDX);  // Toggle P1 (GPIO0, Pin 3)
		k_sleep(K_MSEC(500));  // Delay for 500ms
	}


	// ultrasonic_init();

	// while (1) {
	// 	double distance = sample_sensors_distance_cm();
	// 	printk("Distance: %f cm\n", distance);
	// 	k_sleep(K_MSEC(1000));
	// }

	while (1);

	return -1;
}