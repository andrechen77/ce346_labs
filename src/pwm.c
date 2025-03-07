#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#define PWM_DEVICE_NAME "PWM_0"
#define PWM_PIN 3 // P0.3
#define PWM_PERIOD_US 1000 // 1 ms period

void pwm(void) {
	const struct device* pwm_dev;
	pwm_dev = device_get_binding(PWM_DEVICE_NAME);
	if (!pwm_dev) {
		printk("PWM device not found\n");
		return;
	}

	while (1) {
		fprintf(stderr, "Setting PWM duty cycle to 25%%\n");
		// Set PWM duty cycle to 50%
		pwm_set(pwm_dev, 0, PWM_PERIOD_US, PWM_PERIOD_US / 2, 0);
		k_sleep(K_MSEC(1000));

		// Set PWM duty cycle to 75%
		fprintf(stderr, "Setting PWM duty cycle to 75%%\n");
		pwm_set(pwm_dev, 0, PWM_PERIOD_US, 3 * PWM_PERIOD_US / 8, 0);
		k_sleep(K_MSEC(1000));
	}
}
