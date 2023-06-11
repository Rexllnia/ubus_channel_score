#ifndef _DEVICE_LIST_H_
#define _DEVICE_LIST_H_

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
#include <json-c/json.h>
#include "channel_score_config.h"

#define SN_LEN 15
#define ROLE_STRING_LEN 4

struct device_info {
	char series_no[SN_LEN];
	char role[ROLE_STRING_LEN];
	int status;
	struct user_input input;
	struct channel_info channel_info[36];
	time_t timestamp;
};
struct device_list {
    int list_len;
    struct device_info device[32];
};

/* 
    p: type device_info loop
    i: counter
    dev_list : type device_list
*/
#define list_for_each_device(p,i,dev_list) \
    for ((p) = (dev_list)->device,i = 0;i < (dev_list)->list_len;p++,i++)

int modify_device(struct device_list *device_list,struct device_info *device);
int find_device_by_sn(struct device_list *device_list,char *series_no);
int find_ap(struct device_list *device_list);
void get_online_device(struct device_list *device_list);

#endif
