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

#define SCAN_BUSY       1
#define SCAN_IDLE       2
#define SCAN_NOT_START  0

void udp_send(void *buf,size_t size,char *dest_ip);
void *udp_thread(void *arg);

#endif
