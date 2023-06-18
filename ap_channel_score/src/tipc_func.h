
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/tipc.h>
#include <time.h>
#include <fcntl.h>
#include "channel_score_config.h"

#define SERVER_TYPE_SCAN 18888
#define SERVER_TYPE_GET    102
#define SERVER_INST  17
#define BUF_SIZE 40

void *tipc_receive_thread(void * argv);
int rg_mist_mac_2_nodeadd(unsigned char *mac_src);
int tipc_msg_send(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait);
int tipc_msg_send_receive(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait_sec);