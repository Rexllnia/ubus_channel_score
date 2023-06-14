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
#include "config.h"
#include "tools.h"
#include "channel_scan.h"
#include "kv_nav_task.h"
#include "tipc_scan_msg.h"

extern struct user_input g_input;
extern volatile int g_status;
extern pthread_mutex_t g_mutex;
extern sem_t g_semaphore;

pthread_t pid1, pid2 ,pid3;

int main(int argc, char **argv)
{

    sem_init(&g_semaphore,0,0);
    g_input.scan_time = MIN_SCAN_TIME;
    g_status = SCAN_NOT_START;
	g_input.channel_bitmap = 0;

    pthread_mutex_init(&g_mutex, NULL);

    if ((pthread_create(&pid1, NULL, scan_func, NULL)) != 0) {

        return 0;
    }

    if ((pthread_create(&pid2, NULL, tipc_scan_msg_thread, NULL)) != 0) {

        return 0;
    }
    if ((pthread_create(&pid3, NULL, tipc_get_msg_thread, NULL)) != 0) {

        return 0;
    }

	if (pthread_join(pid1, NULL) != 0) {
	
		return 0;
    }
   	if (pthread_join(pid2, NULL) != 0) {
	
		return 0;
    }

    if (pthread_join(pid3, NULL) != 0) {
	
		return 0;
    }
	return 0;
}



