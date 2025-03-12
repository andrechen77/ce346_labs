#include "info.h"
#include <string.h>

K_MUTEX_DEFINE(my_mutex);

static info_t fish_info = {
    5, // once every 5 seconds
    2, // spin for 5 seconds
    0, // haven't fed yet
    0, // haven't fed yet
    5, // will feed in 5 secocnds
    5, // will feed in 5 seconds
    0, // assume food isn't low for now
    &my_mutex
};

void info_to_buf_str(char* str_buf, uint32_t buf_size) {
    k_mutex_lock(fish_info.mut, K_FOREVER);

    // TODO: change the %d formatter to appropriate value for uint32_t
    const char* format_str =
        "Feed frequency (sec): %d\n"
        "Feed duration (sec): %d\n"
        "Last feed (sec): %d\n"
        "Next feed (sec): %d\n"
        "Is food low?: %s\n";

    char* food_level_str = fish_info.is_food_low ? "Yes" : "No";
    snprintf(str_buf, buf_size, format_str, fish_info.feed_freq_sec, fish_info.feed_duration_sec, fish_info.last_feed_sec, fish_info.next_feed_sec, food_level_str);

    k_mutex_unlock(fish_info.mut);
}

// i'm pretty sure we are on a single core system, so
info_t* get_info(void) {
    return &fish_info;
}