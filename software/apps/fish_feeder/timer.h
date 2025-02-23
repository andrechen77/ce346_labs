#include "nrf.h"

#include "app_error.h"
#include "app_timer.h"

#include <stdint.h>

typedef void (*callback_t)(void);

void timer_init();
uint32_t read_timer_us();
void execute_event(uint32_t after_ms, callback_t cb);
