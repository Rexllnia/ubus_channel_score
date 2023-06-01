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
#include "ubus_thread.h"

#define BAND_5G

struct channel_info {
    int channel;
    int floornoise;
    int utilization;
    int bw;
    int obss_util;
    int tx_util;
    int rx_util;
};


int test_calculate_channel_score(struct channel_info *info);
int test_change_channel(int channel);

extern struct user_input g_input;
volatile int g_status;
pthread_mutex_t g_mutex;
pthread_t pid1, pid2 ,pid3;

#ifdef USE_POPEN
int get_5g_channel_info (struct channel_info *info) 
{
    FILE *fp;
    char buf[1024];
    char *p;
    char cmd[1024];


    sprintf(cmd,"wlanconfig rax0 radio");

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
#endif

#ifdef BAND_5G
void *scan_thread(void *arg) 
{
    struct json_object *obj;
    json_object *root[10] ;
    struct json_object *list = json_object_new_array();
    char *json_str;


    int i,j,score,len;
    struct channel_info channel_info_5g;

    while (1) {
        if (g_status == SCAN_RUNNING) {
            for (j=0,i = 36;i <= 64;i += 4,j++) {
                sleep(1);

                get_5g_channel_info(&channel_info_5g);

                printf("%d\r\n",channel_info_5g.channel);
                printf("%d\r\n",channel_info_5g.floornoise);
                printf("%d\r\n",channel_info_5g.utilization);
                printf("%d\r\n",channel_info_5g.bw);
                printf("%d\r\n",channel_info_5g.obss_util);
                printf("%d\r\n",channel_info_5g.rx_util);
                printf("%d\r\n",channel_info_5g.tx_util);
                printf("------------------\r\n");
                score = test_calculate_channel_score(&channel_info_5g);

                test_change_channel(i);

            }

            pthread_mutex_lock(&g_mutex);
            g_status = SCAN_IDLE;
            pthread_mutex_unlock(&g_mutex);
        }
    }
}
#endif

int main(int argc, char **argv)
{


    pthread_mutex_init(&g_mutex, NULL);
#ifdef BAND_5G
    if ((pthread_create(&pid1, NULL, scan_thread, NULL)) != 0) {

        return 0;
    }
#endif
    if ((pthread_create(&pid3, NULL, ubus_thread, NULL)) != 0) {

        return 0;
    }
#ifdef BAND_5G
	if (pthread_join(pid1, NULL) != 0) {
	
		return 0;
    }
#endif
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
