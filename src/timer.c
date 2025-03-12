#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>
#include "timer.h"

uint32_t read_timer_us(void) {
	uint32_t cycles = k_cycle_get_32();
	uint32_t cycles_per_sec = sys_clock_hw_cycles_per_sec();

	// prevent overflow and division by zero by scaling both the numerator and
	// denominator
	return (cycles * 1000) / (cycles_per_sec / 1000);
}

uint32_t read_timer_s(void) {
	uint64_t ms = k_uptime_get();
	return (uint32_t)(ms / 1000);
}