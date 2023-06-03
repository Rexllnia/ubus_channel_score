/* channel_score_config.h */
#ifndef _CHANNEL_SCORE_CONFIG_H_
#define _CHANNEL_SCORE_CONFIG_H_

#define FAIL       -1
#define SUCCESS    0

struct user_input {
    int code;
    int band;
    long channel_bitmap;
    int channel_num;
    int scan_time;
};

struct channel_info {
    int channel;
    int floornoise;
    int utilization;
    int bw;
    int obss_util;
    int tx_util;
    int rx_util;
    int score;
};

#endif

