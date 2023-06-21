/* spctrm_scn_wireless.h */
#ifndef _COUNTRY_CHANNEL_H_
#define _COUNTRY_CHANNEL_H_

#include <json-c/json.h>
#include "lib_unifyframe.h"
#include "list.h"
#include "spctrm_scn_config.h"
#include "spctrm_scn_dev.h"

#define MAX_CHANNEL_NUM         200 


struct country_channel_info {
    char frequency[8];
    int channel;
    int NopOccupancy;
    struct hlist_node node;
};

void spctrm_scn_wireless_wds_state();
int timeout_func();
int spctrm_scn_wireless_channel_info(struct channel_info *info,int band);

double spctrm_scn_wireless_channel_score(struct channel_info *info);
double calculate_N(struct channel_info *info) ;
int spctrm_scn_wireless_change_channel(int channel);
void *spctrm_scn_wireless_scan_thread(void *arg);
void *scan_func();

int spctrm_scn_wireless_country_channel(int bw,long *bitmap_2G,long *bitmap_5G,int band);

static inline channel_to_bitmap (int channel);
static inline bitmap_to_channel (int bit_set);

#endif

