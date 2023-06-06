/* channel_score_config.h */
#ifndef _CHANNEL_SCORE_CONFIG_H_
#define _CHANNEL_SCORE_CONFIG_H_

#define POPEN_CMD_ENABLE
#define BRIDGE_PLATFORM
#define UDP_FUNCTION

#define MAX_POPEN_BUFFER_SIZE   4096


#define FAIL       -1
#define SUCCESS    0

#define MAX_BAND_5G_CHANNEL_NUM 36

#define PLATFORM_5G     5
#define PLATFORM_2G     2
#define PLATFORM_BOTH   0

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

