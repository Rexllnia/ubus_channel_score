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
#define bridge



int calculate_channel_score(struct channel_info *info);
int change_channel(int channel);
extern struct channel_info g_channel_info_5g[36];
extern struct user_input g_input;
volatile int g_status;

extern long g_bitmap_2G,g_bitmap_5G;
pthread_mutex_t g_mutex;
pthread_t pid1, pid2 ,pid3;
sem_t g_semaphore;

int get_channel_info(struct channel_info *info,int band) 
{
    FILE *fp;
    char *rbuf;
    char *p;
    char cmd[1024];

    if (info == NULL) {
         return FAIL;
    }

    if (band == 5) {
#ifdef bridge
        execute_cmd("wlanconfig ra0 radio",&rbuf);
#elif defined AP
        execute_cmd("wlanconfig rax0 radio",&rbuf);
#endif
    } else if (band == 2) {

    } else {
        return FAIL;
    }
    
    
    strtok(rbuf,"\n");

    strtok(NULL,":");
    p=strtok(NULL,"\n");


    info->channel=atoi(p);
      
    strtok(NULL,":");
    p=strtok(NULL,"\n");
    info->floornoise=atoi(p);
     
    strtok(NULL,":");
    p=strtok(NULL,"\n");
    info->utilization=atoi(p);
     
    strtok(NULL,"\n");
    strtok(NULL,":");
    p=strtok(NULL,"\n");
    info->bw=atoi(p);
    
    strtok(NULL,":");
    p=strtok(NULL,"\n");
    info->obss_util=atoi(p);
    
    strtok(NULL,":");
    p=strtok(NULL,"\n");
    info->tx_util=atoi(p);
   
    strtok(NULL,":");
    p=strtok(NULL,"\n");

    info->rx_util=atoi(p);
    
    free(rbuf);

    return 0;
}


    

void *scan_thread(void *arg) 
{

    char *json_str;

    
    int i,j,score,len;
    struct channel_info channel_info_5g[36];
    
    while (1) {
        sem_wait(&g_semaphore);
        if (g_status == SCAN_BUSY) {

            for (j = 0,i = 0; i < sizeof(long) * 8; i++) {
                if ((g_input.channel_bitmap& (1L << i)) != 0) {
                    
                    channel_info_5g[j].channel = bitmap_to_channel(i);
                    printf("Bit to channel %d >>>>>\n",channel_info_5g[j].channel);
                    sleep(2);
                    change_channel(channel_info_5g[j].channel);
                    sleep(2);
                    get_channel_info(&channel_info_5g[j],5);
                    
                    printf("%d\r\n",channel_info_5g[j].channel);
                    printf("%d\r\n",channel_info_5g[j].floornoise);
                    printf("%d\r\n",channel_info_5g[j].utilization);
                    printf("%d\r\n",channel_info_5g[j].bw);
                    printf("%d\r\n",channel_info_5g[j].obss_util);
                    printf("%d\r\n",channel_info_5g[j].rx_util);
                    printf("%d\r\n",channel_info_5g[j].tx_util);
                    channel_info_5g[j].score = calculate_channel_score(&channel_info_5g[j]);
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

int main(int argc, char **argv)
{
    printf("qwewewe");

    sem_init(&g_semaphore,0,0);
    g_status == SCAN_NOT_START;
    pthread_mutex_init(&g_mutex, NULL);

    if ((pthread_create(&pid1, NULL, scan_thread, NULL)) != 0) {

        return 0;
    }

    if ((pthread_create(&pid2, NULL, udp_thread, NULL)) != 0) {

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
int calculate_channel_score(struct channel_info *info) 
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
    return (1 - N/20)*info->obss_util;
}



#ifdef popen_cmd
int change_channel(int channel) 
{
    char cmd[MAX_POPEN_BUFFER_SIZE];

    sprintf(cmd,"dev_config update -m radio '{ \"radioList\": [ { \"radioIndex\": \"1\", \"type\":\"5G\", \"channel\":\"%d\" } ]}'",channel);

    execute_cmd(cmd,NULL);

    return 0;
}
#elif defined unifyframework
int change_channel(int channel) 
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


