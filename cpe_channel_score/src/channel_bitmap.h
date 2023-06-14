#ifndef _CHANNEL_BITMAP_H_
#define _CHANNEL_BITMAP_H_

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

int channel_to_bitmap (int channel);
int bitmap_to_channel (int bit_set);

 
            


#endif 
