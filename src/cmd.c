#include "cmd.h"
#include "info.h"
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 512

void execute_command(char* cmd, size_t cmd_size) {
    char* password;
    info_t* info = get_info();

    password = strsep(&cmd, " ");
    if (!password || strcmp(password, "stephen")) {
        fprintf(stderr, "wrong password!\n");
        return;
    }

    char* action = strsep(&cmd, " ");
    if (!action) {
        fprintf(stderr, "no action!\n");
        return;
    } else if (!strcmp(action, "freq")) {
        char* new_freq_str = strsep(&cmd, " ");
        int new_freq = atoi(new_freq_str);

        if (new_freq <= 0) {
            return;
        }

        k_mutex_lock(info->mut, K_FOREVER);
        info->feed_freq_sec = new_freq;
        info->_next_feed_timestamp_sec = info->_last_feed_timestamp_sec + info->feed_freq_sec;
        k_mutex_unlock(info->mut);
        return;
    } else if (!strcmp(action, "dur")) {
        char* new_dur_str = strsep(&cmd, " ");
        int new_dur = atoi(new_dur_str);

        if (new_dur <= 0 || new_dur >= info->feed_freq_sec) {
            return;
        }

        k_mutex_lock(info->mut, K_FOREVER);
        info->feed_duration_sec = new_dur;
        k_mutex_unlock(info->mut);
        return;
    }
}