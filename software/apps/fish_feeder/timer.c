#include "timer.h"

void timer_init() {
	// high-resolution timer
	NRF_TIMER4->MODE = 0; // timer mode
	NRF_TIMER4->BITMODE = 3; // 32 bit, overflows after ~1 hour
	NRF_TIMER4->PRESCALER = 4; // each tick is 1 microsecond

	NRF_TIMER4->TASKS_CLEAR = 1;
	NRF_TIMER4->TASKS_START = 1;

	// rtc timer
	NRF_RTC1->PRESCALER = 4095; // each tick is 125 ms
	NRF_RTC1->TASKS_CLEAR = 1;
	NRF_RTC1->TASKS_START = 1;
}

uint32_t read_timer_us(void) {
	NRF_TIMER4->TASKS_CAPTURE[1] = 1;
	return NRF_TIMER4->CC[1];
}

uint32_t read_timer_ms(void) {
	return NRF_RTC1->COUNTER * 125;
}

void execute_event(uint32_t after_ms, callback_t cb) {
	if (read_timer_ms() > after_ms) {
		cb();
		NRF_RTC1->TASKS_STOP = 1;
		NRF_RTC1->TASKS_CLEAR = 1;
		NRF_RTC1->TASKS_START = 1;
	}
}