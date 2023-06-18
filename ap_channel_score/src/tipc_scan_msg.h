
#include <stdio.h>
#include <errno.h>
#include <linux/tipc.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <pthread.h>
#include "channel_score_config.h"


#define SERVER_TYPE_SCAN        18888
#define SERVER_TYPE_GET         102

#define SERVER_INST  17
#define BUF_SIZE 40

void *tipc_get_msg_thread();
void *tipc_scan_msg_thread();
