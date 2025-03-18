#include <stdint.h>

extern uint8_t display_buf[5];

int display_update(void);

int display_init(void);

void write_ring_pattern(uint8_t outer_ring, uint8_t inner_ring, bool center);

void write_row_pattern(uint8_t n);
