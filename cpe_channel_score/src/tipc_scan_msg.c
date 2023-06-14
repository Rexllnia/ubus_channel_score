#include "tipc_scan_msg.h"
#include "config.h"

extern pthread_mutex_t g_mutex;
extern sem_t g_semaphore;
extern time_t g_current_time;
struct user_input g_input;

extern struct device_list g_finished_device_list;
extern struct device_list g_device_list;
extern struct channel_info g_channel_info_5g[36];
extern struct channel_info realtime_channel_info_5g[36];

extern volatile int g_status,g_scan_time;
extern volatile long g_scan_timestamp;

extern long g_bitmap_2G,g_bitmap_5G;

extern pthread_mutex_t g_mutex;
extern sem_t g_semaphore;
extern time_t g_current_time;

char rg_misc_read_file(char *name,char *buf,char len) {
    int fd;

    memset(buf,0,len);
    fd = open(name, O_RDONLY);
    if (fd > 0) {
        read(fd,buf,len);
        close(fd);
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = 0;
        }
        return SUCCESS;
    }
    return FAIL;
}
int rg_mist_mac_2_nodeadd(unsigned char *mac_src){
    unsigned int mac[6];
    unsigned int tmp;
    char buf[30];

    memset(mac,0,sizeof(mac));
    if (sscanf(mac_src, "%2x:%2x:%2x:%2x:%2x:%2x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]) != 6) {
        perror("please input right like :rg_tipc_mac_to_nodeadd aa:bb:cc:dd:ee:ffi \n");
        exit(0);
    }

    tmp = (mac[0] ^ mac[1] ^ mac[2]) & 0xff;
    tmp = (tmp & 0x0f) ^ (tmp >> 4);

    memset(buf,0,sizeof(buf));
    sprintf(buf,"%x%02x%02x%02x",tmp,mac[3],mac[4],mac[5]);

    tmp = 0;
    sscanf(buf,"%x",&tmp);
    return tmp;
}
void *tipc_get_msg_thread()
{
	
	struct sockaddr_tipc server_addr;
	struct sockaddr_tipc client_addr;
	socklen_t alen = sizeof(client_addr);
	int sd;
	struct channel_info inbuf[36];
	struct timeval timeout={4,0};
    unsigned int instant = 0;
    unsigned char mac[20];
	
	printf("****** TIPC server hello world program started ******\n\n");

    memset(mac,0,sizeof(mac));
    rg_misc_read_file("/proc/rg_sys/sys_mac",mac,sizeof(mac) - 1);

    instant = rg_mist_mac_2_nodeadd(mac);
	printf("line : %d fun : %s instant : %x \r\n",__LINE__,__func__,instant);
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_TYPE_GET;
	server_addr.addr.nameseq.lower = instant;
	server_addr.addr.nameseq.upper = instant;
	server_addr.scope = TIPC_ZONE_SCOPE;

	sd = socket(AF_TIPC, SOCK_RDM, 0);

	
	setsockopt(sd,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));

	if (0 != bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
		printf("Server: failed to bind port name\n");
	}

	while (1) {	
		if (0 >= recvfrom(sd, inbuf, sizeof(inbuf), 0,
						(struct sockaddr *)&client_addr, &alen)) {
			perror("Server: unexpected message");
		}

		printf("realtime_channel_info_5g %d\r\n",realtime_channel_info_5g[0].floornoise);

		if (g_status == SCAN_IDLE) {
			if (0 > sendto(sd, g_channel_info_5g, 36 * sizeof(struct channel_info), 0,
						(struct sockaddr *)&client_addr, sizeof(client_addr))) {
					perror("Server: failed to send");
			}
		} else {
			if (0 > sendto(sd, realtime_channel_info_5g, 36 * sizeof(struct channel_info), 0,
						(struct sockaddr *)&client_addr, sizeof(client_addr))) {
			perror("Server: failed to send");
		}
		}


		printf("\n****** TIPC GET server program finished ******\n");
	}	
}

void *tipc_scan_msg_thread()
{
	
	struct sockaddr_tipc server_addr;
	struct sockaddr_tipc client_addr;
	socklen_t alen = sizeof(client_addr);
	int sd;
	struct user_input inbuf;
    unsigned int instant = 0;
    unsigned char mac[20];

	char outbuf[BUF_SIZE] = "Uh ?";
	
	printf("****** TIPC server hello world program started ******\n\n");

    memset(mac,0,sizeof(mac));
    rg_misc_read_file("/proc/rg_sys/sys_mac",mac,sizeof(mac) - 1);

    instant = rg_mist_mac_2_nodeadd(mac);
	printf("line : %d fun : %s instant : %x \r\n",__LINE__,__func__,instant);
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_TYPE_SCAN;
	server_addr.addr.nameseq.lower = instant;
	server_addr.addr.nameseq.upper = instant;
	server_addr.scope = TIPC_ZONE_SCOPE;

	sd = socket(AF_TIPC, SOCK_RDM, 0);

	if (0 != bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
		printf("Server: failed to bind port name\n");
		exit(1);
	}

	while (1) {	
		if (0 >= recvfrom(sd, &inbuf, sizeof(struct user_input), 0,
						(struct sockaddr *)&client_addr, &alen)) {
			perror("Server: unexpected message");

		}

		printf("Server: Message received: %ld !\n", inbuf.channel_bitmap);


		if (0 > sendto(sd, outbuf, strlen(outbuf)+1, 0,
						(struct sockaddr *)&client_addr, sizeof(client_addr))) {
			perror("Server: failed to send");
		}

		while (1) {
			if (g_status == SCAN_IDLE || g_status == SCAN_NOT_START) {
				pthread_mutex_lock(&g_mutex);
				g_input = inbuf;
				g_status = SCAN_BUSY;
				pthread_mutex_unlock(&g_mutex);
				sem_post(&g_semaphore);
				break;
				
			} else if (g_status == SCAN_BUSY) {
				pthread_mutex_lock(&g_mutex);
				g_status = SCAN_TIMEOUT;
				pthread_mutex_unlock(&g_mutex);
			}
		} 

		printf("\n****** TIPC server hello program finished ******\n");
	}
	
	
}