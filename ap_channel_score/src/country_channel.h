/* country_channel.h */
#ifndef _COUNTRY_CHANNEL_H_
#define _COUNTRY_CHANNEL_H_

#include <json-c/json.h>
#include "lib_unifyframe.h"
#include "list.h"
#include "channel_score_config.h"

#define MAX_CHANNEL_NUM         200 


struct country_channel_info {
    char frequency[8];
    int channel;
    int NopOccupancy;
    struct hlist_node node;
};
int channel_to_bitmap (int channel);
int bitmap_to_channel (int bit_set);
int get_country_channel_bitmap(int bw,long *bitmap_2G,long *bitmap_5G,int band);

#endif

