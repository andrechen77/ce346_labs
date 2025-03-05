#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>
#include "timer.h"

uint32_t read_timer_us(void) {
	uint32_t cycles = k_cycle_get_32();
	uint32_t cycles_per_sec = sys_clock_hw_cycles_per_sec();

	return (cycles * 1000000) / cycles_per_sec;
}
