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
#include "ubus_thread.h"
#include "lib_unifyframe.h"


#define BAND_5G



int test_calculate_channel_score(struct channel_info *info);
int test_change_channel(int channel);
extern struct channel_info g_channel_info_5g[36];
extern struct user_input g_input;
volatile int g_status;

extern long g_bitmap_2G,g_bitmap_5G;
pthread_mutex_t g_mutex;
pthread_t pid1, pid2 ,pid3;


int get_channel_info(struct channel_info *info,int band) 
{
    FILE *fp;
    char buf[1024];
    char *p;
    char cmd[1024];

    if (band == 5) {
        sprintf(cmd,"wlanconfig rax0 radio");
    } else {
        return FAIL;
    }
    

    fp = popen(cmd, "r");
    if (!fp) {
        printf("Failed to execute command\n");
        return -1;
    }
 
    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);        
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->channel=atoi(p);

    fgets(buf, sizeof(buf), fp);        
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->floornoise=atoi(p);

    fgets(buf, sizeof(buf), fp);        
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->utilization=atoi(p);

    fgets(buf, sizeof(buf), fp);       
    fgets(buf, sizeof(buf), fp);        
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->bw=atoi(p);
    fgets(buf, sizeof(buf), fp);      
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->obss_util=atoi(p);
    fgets(buf, sizeof(buf), fp);      
    strtok(buf,":");
    p=strtok(NULL,"\n");
    info->tx_util=atoi(p);

    fgets(buf, sizeof(buf), fp);        
    strtok(buf,":");
    p=strtok(NULL,"\n");

    info->rx_util=atoi(p);

    pclose(fp);
    return 0;
}


    

void *scan_thread(void *arg) 
{

    char *json_str;


    int i,j,score,len;
    struct channel_info channel_info_5g[36];
    
    while (1) {
        if (g_status == SCAN_BUSY) {

            for (j = 0,i = 0; i < sizeof(long) * 8; i++) {
                if ((g_input.channel_bitmap& (1L << i)) != 0) {
                    sleep(1);
                    channel_info_5g[j].channel = bitmap_to_channel(i);
                    printf("Bit to channel %d \n",channel_info_5g[j].channel);
                    test_change_channel(channel_info_5g[j].channel);
                    get_channel_info(&channel_info_5g[j],5);
                    printf("%d\r\n",channel_info_5g[j].channel);
                    printf("%d\r\n",channel_info_5g[j].floornoise);
                    printf("%d\r\n",channel_info_5g[j].utilization);
                    printf("%d\r\n",channel_info_5g[j].bw);
                    printf("%d\r\n",channel_info_5g[j].obss_util);
                    printf("%d\r\n",channel_info_5g[j].rx_util);
                    printf("%d\r\n",channel_info_5g[j].tx_util);
                    printf("------------------\r\n");
                    j++;
                }
            }
            
            pthread_mutex_lock(&g_mutex);
            memcpy(g_channel_info_5g,channel_info_5g,sizeof(channel_info_5g));
            g_status = SCAN_IDLE;
            pthread_mutex_unlock(&g_mutex);
        }
    }
}
/*dev_sta get -m country_channel '{"qry_type":"channellist", "band":"BW_20"}'*/

int main(int argc, char **argv)
{

    g_status == SCAN_NOT_START;
    pthread_mutex_init(&g_mutex, NULL);

    if ((pthread_create(&pid1, NULL, scan_thread, NULL)) != 0) {

        return 0;
    }
    if ((pthread_create(&pid3, NULL, ubus_thread, NULL)) != 0) {

        return 0;
    }

	if (pthread_join(pid1, NULL) != 0) {
	
		return 0;
    }

	if (pthread_join(pid3, NULL) != 0) {
	
		return 0;
    }
	return 0;
}
int test_calculate_channel_score(struct channel_info *info) 
{
    return 0;
}

int test_change_channel(int channel) 
{
    FILE *fp;
    char cmd[1024];

    sprintf(cmd,"dev_config update -m radio '{ \"radioList\": [ { \"radioIndex\": \"1\", \"type\":\"5G\", \"channel\":\"%d\" } ]}'",channel);
    fp = popen(cmd, "r");
    pclose(fp);

    return 0;
}
