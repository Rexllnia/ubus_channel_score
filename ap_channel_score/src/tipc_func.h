
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
#include "device_list.h"

#define SERVER_TYPE         103    
#define SERVER_TYPE_SCAN    18888
#define SERVER_TYPE_GET     102
#define SERVER_TYPE_GET_REPLY  104
#define SERVER_INST  17
#define BUF_SIZE 40

int tipc_p2p_send(__u32 dst_instance,__u32 type,size_t payload_size,char *payload);
void *tipc_receive_thread(void * argv);
int rg_mist_mac_2_nodeadd(unsigned char *mac_src);
int tipc_msg_send(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait);
int tipc_msg_send_receive(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait_sec);