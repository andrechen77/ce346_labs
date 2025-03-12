void pwm_init(void);

void pwm_set_duty(float duty_percent);

int pwm_lock(int wait_ms);

void pwm_unlock();
