
#include "tipc_func.h"
#include "channel_score_config.h"
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

extern int g_status;
extern unsigned char g_mode;
extern struct user_input g_input;
extern struct device_list g_finished_device_list;
extern struct channel_info g_channel_info_5g[36];
extern struct channel_info realtime_channel_info_5g[36];
extern pthread_mutex_t g_mutex;
extern sem_t g_semaphore;

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
int rg_mist_mac_2_nodeadd(unsigned char *mac_src)
{
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
int wait_for_server(__u32 name_type, __u32 name_instance, int wait)
{
	struct sockaddr_tipc topsrv;
	struct tipc_subscr subscr;
	struct tipc_event event;

	int sd = socket(AF_TIPC, SOCK_SEQPACKET, 0);

	memset(&topsrv, 0, sizeof(topsrv));
	topsrv.family = AF_TIPC;
	topsrv.addrtype = TIPC_ADDR_NAME;
	topsrv.addr.name.name.type = TIPC_TOP_SRV;
	topsrv.addr.name.name.instance = TIPC_TOP_SRV;

	/* Connect to topology server */

	if (0 > connect(sd, (struct sockaddr *)&topsrv, sizeof(topsrv))) {
		perror("Client: failed to connect to topology server");

	}

	subscr.seq.type = htonl(name_type);
	subscr.seq.lower = htonl(name_instance);
	subscr.seq.upper = htonl(name_instance);
	subscr.timeout = htonl(wait);
	subscr.filter = htonl(TIPC_SUB_SERVICE);

	if (send(sd, &subscr, sizeof(subscr), 0) != sizeof(subscr)) {
		perror("Client: failed to send subscription");

		close(sd);
		return FAIL;
	}
	printf("line : %d fun : %s \r\n",__LINE__,__func__);

	/* Now wait for the subscription to fire */
	if (recv(sd, &event, sizeof(event), 0) != sizeof(event)) {
		perror("Client: failed to receive event");

		close(sd);
		return FAIL;
	}

	printf("line : %d fun : %s \r\n",__LINE__,__func__);

	if (event.event != htonl(TIPC_PUBLISHED)) {
		printf("Client: server {%u,%u} not published within %u [s]\n",
		       name_type, name_instance, wait/1000);
			
		close(sd);
		return FAIL;
		
	}
	close(sd);
}

int tipc_msg_send(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait)
{
	int sd;
	struct sockaddr_tipc server_addr;

	printf("****** TIPC client hello world program started ******\n\n");

	if (wait_for_server(name_type, name_instance, wait) == FAIL) {
		return FAIL;
	}
	printf("line : %d fun : %s \r\n",__LINE__,__func__);

	sd = socket(AF_TIPC, SOCK_RDM, 0);

	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = name_type;
	server_addr.addr.name.name.instance = name_instance;
	server_addr.addr.name.domain = 0;

	if (0 > sendto(sd, buf, size, 0,
	                (struct sockaddr*)&server_addr, sizeof(server_addr))) {
		perror("Client: failed to send");
		
	}

	close(sd); /* xxx */
	printf("\n****** TIPC client hello program finished ******\n");
	
}
int tipc_msg_send_receive(__u32 name_type, __u32 name_instance,void *buf,ssize_t size, int wait_sec)
{
	int sd;
	struct sockaddr_tipc server_addr;

	struct timeval timeout={4,0};

	timeout.tv_sec = wait_sec;
	
	printf("****** TIPC client hello world program started ******\n\n");

	if (wait_for_server(name_type, name_instance, 100) == FAIL) {
		return FAIL;
	}

	printf("line : %d fun : %s \r\n",__LINE__,__func__);

	sd = socket(AF_TIPC, SOCK_RDM, 0);

	setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = name_type;
	server_addr.addr.name.name.instance = name_instance;
	server_addr.addr.name.domain = 0;


	if (0 > sendto(sd, buf,size, 0,
	                (struct sockaddr*)&server_addr, sizeof(server_addr))) {
		perror("Client: failed to send");
		return FAIL;
	}

	if (0 >= recv(sd, buf, size, 0)) {
		perror("Client: unexpected response");
		return FAIL;
	}
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	close(sd); /* xxx */
	printf("\n****** TIPC client hello program finished ******\n");
	
}

typedef struct tipc_recv_packet_head {
	unsigned int type;
	size_t payload_size;
	unsigned int instant;
}tipc_recv_packet_head_t;

int tipc_p2p_send(__u32 dst_instance,__u32 type,size_t payload_size,char *payload)
{
	int sd;
	struct sockaddr_tipc server_addr;
    struct timeval timeout={4,0};
    __u32 src_instant = 0;
	char mac[20];
	char *pkt;
	tipc_recv_packet_head_t *head;
	size_t pkt_size;

	if(wait_for_server(SERVER_TYPE, ntohl(dst_instance), 100) == FAIL) {
		return FAIL;
	}
	
	debug("");
	pkt_size = sizeof(tipc_recv_packet_head_t) + payload_size;
	debug("");
	pkt = (char*)malloc(pkt_size * sizeof(char));
	memset(mac,0,sizeof(mac));
    rg_misc_read_file("/proc/rg_sys/sys_mac",mac,sizeof(mac) - 1);
    src_instant = rg_mist_mac_2_nodeadd(mac);
	memcpy((pkt+sizeof(tipc_recv_packet_head_t)),payload,payload_size);
	head = (tipc_recv_packet_head_t *)pkt;
	head->instant = src_instant;
	head->type = type;
	head->payload_size = payload_size;

	printf("****** TIPC client hello world program started ******\n\n");



	sd = socket(AF_TIPC, SOCK_RDM, 0);

	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = SERVER_TYPE;
	server_addr.addr.name.name.instance = ntohl(dst_instance);
	server_addr.addr.name.domain = 0;
    
    setsockopt(sd,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));
	if (0 > sendto(sd, pkt, pkt_size, 0,
	                (struct sockaddr*)&server_addr, sizeof(server_addr))) {
		perror("Client: failed to send");
		exit(1);
	}
	close(sd);
}

void *tipc_receive_thread(void * argv)
{

	struct sockaddr_tipc server_addr;
	struct sockaddr_tipc client_addr;
	socklen_t alen = sizeof(client_addr);
	int sd;
	char *pkt = NULL;
	tipc_recv_packet_head_t head;
	size_t pkt_size;
	char outbuf[BUF_SIZE] = "Uh ?";
    struct timeval timeout={4,0};
	unsigned char mac[20];
	__u32 instant;
#ifdef CONFIG_TIPC_CORE_DUBUG
    struct rlimit limit;
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &limit);
#endif
	printf("****** TIPC server hello world program started ******\n\n");

    memset(mac,0,sizeof(mac));
    rg_misc_read_file("/proc/rg_sys/sys_mac",mac,sizeof(mac) - 1);

    instant = rg_mist_mac_2_nodeadd(mac);

	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_TYPE;
	server_addr.addr.nameseq.lower = ntohl(instant);
	server_addr.addr.nameseq.upper = ntohl(instant);
	server_addr.scope = TIPC_ZONE_SCOPE;

	sd = socket(AF_TIPC, SOCK_RDM, 0);

	if (0 != bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
		printf("Server: failed to bind port name\n");
		exit(1);
	}
	while (1) {
		
		memset(&head, 0, sizeof(head));
		if (0 >= recvfrom(sd, &head, sizeof(head), MSG_PEEK,
						(struct sockaddr *)&client_addr, &alen)) {
			perror("Server: unexpected message");
		}
		debug("type %d",head.type);
		pkt_size = head.payload_size + sizeof(head);
		debug("pkt_size %d",pkt_size);
		pkt = (char *)malloc(sizeof(char) * pkt_size);
		if (pkt == NULL) {
			debug("malloc FAIL");
		}
		debug("malloc");
		if (0 >= recvfrom(sd, pkt,pkt_size, 0,
						(struct sockaddr *)&client_addr, &alen)) {
			perror("Server: unexpected message");
		}
		debug("");
		if (head.type == SERVER_TYPE_GET) {
			debug("SERVER_TYPE_GET_REPLY,%d",realtime_channel_info_5g[0].floornoise);
			if (g_status == SCAN_IDLE) {
				tipc_p2p_send(head.instant,SERVER_TYPE_GET_REPLY,sizeof(realtime_channel_info_5g),realtime_channel_info_5g);
			} else {
				debug("%d\r\n",g_channel_info_5g[0].floornoise);
				tipc_p2p_send(head.instant,SERVER_TYPE_GET_REPLY,sizeof(g_channel_info_5g),g_channel_info_5g);
			}
		} else if (head.type == SERVER_TYPE_GET_REPLY) {
			struct device_info *p;
			int i;
			__u32 instant = 0;
			list_for_each_device(p,i,&g_finished_device_list) {
				instant = rg_mist_mac_2_nodeadd(p->mac);
				debug("instant : %x ",instant);
				if (instant == head.instant) {	
					memcpy(p->channel_info,pkt+sizeof(tipc_recv_packet_head_t),head.payload_size);
					debug("p->channel_info[0].floornoise : %d, payload size %d",p->channel_info[0].floornoise,head.payload_size);
				}	
			}
		} else if (head.type == SERVER_TYPE_SCAN) {
			debug("SERVER_TYPE_SCAN");
			while (1) {
			if (g_status == SCAN_IDLE || g_status == SCAN_NOT_START) {
					pthread_mutex_lock(&g_mutex);
					memcpy(&g_input,(pkt+sizeof(tipc_recv_packet_head_t)),sizeof(g_input));
					debug("%ld",g_input.channel_bitmap);
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
		}
	debug("free");
	free(pkt);
	pkt = NULL;
		// printf("Server: Message received: %s !\n", pkt+sizeof(head));
	}
		
    return 0;
}

