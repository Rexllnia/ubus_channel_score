/* usock_udp.h */
#ifndef _USOCK_UDP_H_
#define _USOCK_UDP_H_

#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 
#include <netdb.h>  
#include <net/if.h>  
#include <sys/ioctl.h>  
#include <semaphore.h>
#include <pthread.h>
#include "channel_score_config.h"
#include "device_list.h"

#define SCAN_BUSY       1
#define SCAN_IDLE       2
#define SCAN_TIMEOUT    3
#define SCAN_NOT_START  0

#define MAX_MESSAGE_LEN 1024

#define OPCODE_NULL         0
#define OPCODE_GET          1
#define OPCODE_REALTIME_GET 2
#define OPCODE_SCAN         3
#define OPCODE_GET_REPLY    4

struct remote_message {
    int opcode;
    char message[MAX_MESSAGE_LEN];
    struct user_input input;
    struct device_info device;
};

void udp_send(struct remote_message *buf,char *dest_ip);
void *udp_thread(void *arg);

#endif

