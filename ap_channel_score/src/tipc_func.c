
#include "tipc_func.h"
#include "channel_score_config.h"


extern struct user_input g_input;

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