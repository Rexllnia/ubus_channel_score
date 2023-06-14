#ifndef _CHANNEL_SCAN_H_
#define _CHANNEL_SCAN_H_

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
#include "config.h"
#include "tools.h"
#include "device_list.h"
#include "channel_bitmap.h"

#define MIN_SCAN_TIME 15 
#define MAX_SCAN_TIME 60
#define ONE_BYTE 8
#define EXPIRE_TIME 14

int get_channel_info(struct channel_info *info,int band);
double calculate_channel_score(struct channel_info *info);
int change_channel(int channel);
void *scan_func();

#endif
