#include "usock_udp.h"

char g_local_ip[32];
extern sem_t g_semaphore;
extern volatile int g_status;
extern pthread_mutex_t g_mutex;
extern struct user_input g_input;
extern struct channel_info realtime_channel_info_5g[36];

int rlink_get_localip(const char *ifname,char *ip)
{
    int sd;
    struct ifreq ifr;
    char ipstr[32] = {0};

    if ((sd = socket(PF_PACKET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    //get the IP of this interface
    memset (&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);

    if (!ioctl(sd, SIOCGIFADDR, &ifr)) {
        snprintf(ipstr, sizeof(ipstr), "%s",
                 (char *)inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));
    } else {
        close(sd);
        return -1;
    }

    strcpy(ip, ipstr);

    close(sd);
    return 0;
}
void udp_send(struct remote_message *buf,char *dest_ip)
{
    struct sockaddr_in ser, cli;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;

    if (sockfd < 0)
    {
        perror("sockfd");
        return -1;
    }
    
    if (dest_ip == NULL) {
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
        // ser.sin_family = AF_INET;
        // ser.sin_port = htons(7777);
        // ser.sin_addr.s_addr = inet_addr(dest_ip);
    }
    
    
    printf("////%s////\r\n",dest_ip);
    ser.sin_family = AF_INET;
    ser.sin_port = htons(7777);
    ser.sin_addr.s_addr = inet_addr(dest_ip);
    
    int n = sizeof(ser);
    

    sendto(sockfd, buf, sizeof(struct remote_message), 0, (struct sockaddr *)&ser, n);
 
    close(sockfd);
}

void server_receive_callback(struct remote_message *remote_message,void* argv)
{
    struct remote_message ret_message;

    if(remote_message->opcode == OPCODE_SCAN) {
        printf("%d\r\n",remote_message->input.band);
        printf("%ld\r\n",remote_message->input.channel_bitmap);
        printf("%d\r\n",remote_message->input.channel_num);
        printf("%d\r\n",remote_message->input.scan_time);
        if (g_status == SCAN_IDLE || g_status == SCAN_NOT_START) {
            printf("|||||||||");
            pthread_mutex_lock(&g_mutex);
            g_input = remote_message->input;
            g_status = SCAN_BUSY;
            sem_post(&g_semaphore);
            pthread_mutex_unlock(&g_mutex);
        } else {
            g_status = SCAN_TIMEOUT;
        }              
    } else if (remote_message->opcode == OPCODE_GET) {
        
    } else if (remote_message->opcode == OPCODE_REALTIME_GET) {
        memcpy(ret_message.device.channel_info,realtime_channel_info_5g,MAX_BAND_5G_CHANNEL_NUM * sizeof(struct channel_info));
        udp_send(&ret_message,argv);
    } else if (remote_message->opcode == OPCODE_GET_REPLY) {

    }
}
void *udp_thread(void *arg) 
{

    int i,pos;
    char cmd[1024];
    struct remote_message server_buf;

    memset(&server_buf,0,sizeof(struct remote_message));
    rlink_get_localip("br-wan",g_local_ip);
    


    int server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_sockfd < 0) {
        perror("server_sockfd");
        return -1;
    }
    struct sockaddr_in ser, cli;
    ser.sin_family = AF_INET;
    ser.sin_port = htons(7777);
    ser.sin_addr.s_addr = inet_addr("0");
 
    int len = sizeof(ser);
 
    int ret = bind(server_sockfd, (struct sockaddr *)&ser, len);
    if(ret < 0) {
        perror("bind");
        return -1;
    }
 
    int n = sizeof(cli);

    
    while (1) {
        char client_ip[17];
        int m = recvfrom(server_sockfd, &server_buf, sizeof(struct remote_message), 0, (struct sockaddr *)&cli, &n);
        strcpy(client_ip,inet_ntoa(cli.sin_addr));
        printf("client ip:%s  client port:%d\n",client_ip, ntohs(cli.sin_port));
        printf("len = %d\n", m);
        printf("%s\n", server_buf.message);
      
        if (strcmp(g_local_ip,client_ip) == 0) {
            printf("equal\r\n");
        } else {
            server_receive_callback(&server_buf,client_ip);
        }
    }

    close(server_sockfd);
}
