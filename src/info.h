#include <stdint.h>
#include <zephyr/kernel.h>

typedef struct {
    uint32_t feed_freq_sec; // how often to feed (sec)
    uint32_t feed_duration_sec; // how long to feed for (sec)
    uint32_t last_feed_sec; // how many seconds ago the last feeding was
    uint32_t next_feed_sec; // in how many seconds will the next feeding occur
    uint8_t is_food_low; // whether food is low (boolean)
    struct k_mutex* mut; // kms this is needed for race conditions
} info_t;

void info_to_buf_str(char* str_buf, uint32_t buf_size);

info_t* get_info(void);