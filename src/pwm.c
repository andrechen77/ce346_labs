#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#define PWM_DEVICE_NAME "PWM_0"
#define PWM_PERIOD_US 1000 // 1 ms period

const struct device* pwm_dev;

void pwm_init(void) {
	pwm_dev = device_get_binding(PWM_DEVICE_NAME);
	if (!pwm_dev) {
		printk("PWM device not found\n");
		return;
	}
}

void pwm_set_duty(float duty_percent) {
	fprintf(stderr, "Setting PWM duty cycle to %f\n", duty_percent);
	pwm_set(pwm_dev, 0, PWM_PERIOD_US, (float)PWM_PERIOD_US * duty_percent, 0);
}
