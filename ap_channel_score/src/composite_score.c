#include "composite_score.h"
#include "channel_score_config.h"
#include "device_list.h"

int bondchannel(struct device_info *device_info,int bw)
{
    if (bw != BW_40 || bw != BW_80 || device_info == NULL) {
        return FAIL;
    }
    if (bw == BW_40) {
        // device_info->channel_info[i];
        
    }

}