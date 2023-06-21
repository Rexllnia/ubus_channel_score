/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <libubox/blobmsg_json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libubus.h>
#include <json-c/json.h>
#include "spctrm_scn_dev.h"
#include "spctrm_scn_ubus.h"
#include "lib_unifyframe.h"
#include "spctrm_scn_config.h"

#define PLATFORM_5G_ENABLE
#define BRIDGE_PLATFORM





extern unsigned char g_mode;
extern struct device_list g_finished_device_list;
extern struct device_list g_device_list;
struct channel_info g_channel_info_5g[MAX_BAND_5G_CHANNEL_NUM];
struct channel_info realtime_channel_info_5g[36];
extern struct user_input g_input;
volatile int g_status,g_scan_time;
volatile long g_scan_timestamp;
extern long g_bitmap_2G,g_bitmap_5G;

pthread_mutex_t g_mutex;
pthread_t pid1, pid2 ,pid3;
sem_t g_semaphore;


int main(int argc, char **argv)
{
    
    sem_init(&g_semaphore,0,0);
    g_input.scan_time = MIN_SCAN_TIME;
    g_status = SCAN_NOT_START;
	g_input.channel_bitmap = 0;
    spctrm_scn_wireless_wds_state ();
    pthread_mutex_init(&g_mutex, NULL);

    if (g_mode == AP_MODE) {
        debug("ap mode");
        if ((pthread_create(&pid1, NULL, spctrm_scn_wireless_scan_thread, NULL)) != 0) {

            return 0;
        }

        if ((pthread_create(&pid3, NULL, spctrm_scn_ubus_thread, NULL)) != 0) {

            return 0;
        }

#ifdef P2P
        debug("peer to peer");
        if ((pthread_create(&pid2, NULL, spctrm_scn_tipc_thread, NULL)) != 0) {

            return 0;
        }
#endif
    } else if (g_mode == CPE_MODE) {
        debug("cpe mode");
        if ((pthread_create(&pid1, NULL, scan_func, NULL)) != 0) {

            return 0;
        }
        if ((pthread_create(&pid2, NULL, spctrm_scn_tipc_thread, NULL)) != 0) {

            return 0;
        }

    }

if (g_mode == AP_MODE) {

	if (pthread_join(pid1, NULL) != 0) {
	
		return 0;
    }

	if (pthread_join(pid2, NULL) != 0) {
	
		return 0;
    }

#ifdef P2P
	if (pthread_join(pid3, NULL) != 0) {
	
		return 0;
    }
#endif

} else if (g_mode == CPE_MODE) {
    if (pthread_join(pid1, NULL) != 0) {
    
        return 0;
    }
#ifdef P2P
        if (pthread_join(pid2, NULL) != 0) {
        
            return 0;
        }

#elif defined CS
    if (pthread_join(pid2, NULL) != 0) {
    
        return 0;
    }
    if (pthread_join(pid3, NULL) != 0) {
    
        return 0;
    }
#endif    
}

	return 0;
}
double calculate_N(struct channel_info *info) 
{
    double N;

    if (info->floornoise <= -87) {
        N = 0;
    } else if ( -87 < info->floornoise && info->floornoise <= -85) {
        N = 1;
    } else if (-85 < info->floornoise && info->floornoise <= -82) {
        N = 2;
    } else if (-82 < info->floornoise && info->floornoise <= -80) {
        N = 2.8;
    } else if (-80 < info->floornoise && info->floornoise <= -76) {
        N = 4;
    } else if (-76 < info->floornoise && info->floornoise <= -71) {
        N = 4.8;
    } else if (-71 < info->floornoise && info->floornoise <= -69) {
        N = 5.2;
    } else if (-69 < info->floornoise && info->floornoise <= -66) {
        N = 6.4;
    } else if (-66 < info->floornoise && info->floornoise <= -62) {
        N = 7.6;
    } else if (-62 < info->floornoise && info->floornoise <= -60) {
        N = 8.2;
    } else if (-60 < info->floornoise && info->floornoise <= -56) {
        N = 8.8;
    } else if (-56 < info->floornoise && info->floornoise <= -52) {
        N = 9.4;
    } else if (-52 < info->floornoise ) {
        N = 10;
    } 
    
    return N;
}
double spctrm_scn_wireless_channel_score(struct channel_info *info) 
{
    double N;

    if (info->floornoise <= -87) {
        N = 0;
    } else if ( -87 < info->floornoise && info->floornoise <= -85) {
        N = 1;
    } else if (-85 < info->floornoise && info->floornoise <= -82) {
        N = 2;
    } else if (-82 < info->floornoise && info->floornoise <= -80) {
        N = 2.8;
    } else if (-80 < info->floornoise && info->floornoise <= -76) {
        N = 4;
    } else if (-76 < info->floornoise && info->floornoise <= -71) {
        N = 4.8;
    } else if (-71 < info->floornoise && info->floornoise <= -69) {
        N = 5.2;
    } else if (-69 < info->floornoise && info->floornoise <= -66) {
        N = 6.4;
    } else if (-66 < info->floornoise && info->floornoise <= -62) {
        N = 7.6;
    } else if (-62 < info->floornoise && info->floornoise <= -60) {
        N = 8.2;
    } else if (-60 < info->floornoise && info->floornoise <= -56) {
        N = 8.8;
    } else if (-56 < info->floornoise && info->floornoise <= -52) {
        N = 9.4;
    } else if (-52 < info->floornoise ) {
        N = 10;
    } 

    return ((double)1 - N/20)*(double)((double)1 - (double)info->obss_util / 95) * 300 * 0.75;
}



int timeout_func() 
{
    
    int i,j;
    
    
    for (j = 0; j < 30;j++) {
        debug("wait %d",j);
        sleep(1);
        if (spctrm_scn_tipc_send_get_msg(&g_device_list,2) == SUCCESS) {
            return SUCCESS;
        }
    }
    return FAIL;
}


#ifdef POPEN_CMD_ENABLE
int spctrm_scn_wireless_change_channel(int channel) 
{
    char cmd[MAX_POPEN_BUFFER_SIZE];
    sprintf(cmd,"dev_config update -m radio '{ \"radioList\": [ { \"radioIndex\": \"1\", \"type\":\"5G\", \"channel\":\"%d\" } ]}'",channel);
    // sprintf(cmd,"iwpriv ra0 set channel=%d",channel);
    spctrm_scn_common_cmd(cmd,NULL);

    return 0;
}
#elif defined UNIFY_FRAMEWORK_ENABLE
int spctrm_scn_wireless_change_channel(int channel) 
{
    uf_cmd_msg_t *msg_obj;
    int ret;
    char* rbuf;
    char param[100];
    
    sprintf(param,"{\"radioList\": [ { \"radioIndex\": \"1\", \"type\":\"5G\", \"channel\":\"%d\" }]}",channel);
    printf("%s\r\n",param);
    msg_obj = (uf_cmd_msg_t*)malloc(sizeof(uf_cmd_msg_t));
    if (msg_obj == NULL) {
        return -1;
    }
    memset(msg_obj, 0, sizeof(uf_cmd_msg_t));
    msg_obj->ctype = UF_DEV_CONFIG_CALL;/* 调用类型 ac/dev/.. */
    msg_obj->param = param;
    msg_obj->cmd = "update";
    msg_obj->module = "radio";             /* 必填参数，其它可选参数根据需要使用 */
    msg_obj->caller = "group_change";   /* 自定义字符串，标记调用者 */
    ret = uf_client_call(msg_obj, &rbuf, NULL);
    // printf("%s\r\n",rbuf);
    if (rbuf) {
      free(rbuf);
    }
    free(msg_obj);
}
#endif



