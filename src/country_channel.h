/* country_channel.h */
#ifndef _COUNTRY_CHANNEL_H_
#define _COUNTRY_CHANNEL_H_

#include <json-c/json.h>
#include "lib_unifyframe.h"
#include "list.h"
#include "channel_score_config.h"

#define popen_cmd
#define bridge


#define MAX_CHANNEL_NUM         200 
#define MAX_POPEN_BUFFER_SIZE   4096

#define PLATFORM_5G     5
#define PLATFORM_2G     2
#define PLATFORM_BOTH   0

struct country_channel_info {
    char frequency[8];
    int channel;
    int NopOccupancy;
    struct hlist_node node;
};
int channel_to_bitmap (int channel);
int bitmap_to_channel (int bit_set);
int get_country_channel_bitmap(int bw,long *bitmap_2G,long *bitmap_5G,int band);
int execute_cmd(char *cmd,char **rbuf);

#endif

