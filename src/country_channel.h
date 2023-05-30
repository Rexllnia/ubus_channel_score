#include <json-c/json.h>
#include "lib_unifyframe.h"
#include "list.h"

#define FAIL -1
#define SUCCESS 0
#define MAX_CHANNEL_NUM 200 

struct country_channel_info {
    char frequency[8];
    int channel;
    int NopOccupancy;
    struct hlist_node node;
};
int channel_to_bitmap (int channel);
int bitmap_to_channel (int bit_set);
int country_channel_parse(int bw,long *bitmap_2G,long *bitmap_5G);
