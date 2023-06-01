#include "usock_udp.h"

char g_local_ip[32];
extern sem_t g_semaphore;
extern volatile int g_status;
extern pthread_mutex_t g_mutex;

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
void udp_send(void *buf,size_t size,char *dest_ip)
{
    struct sockaddr_in ser, cli;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;

    if(sockfd < 0)
    {
        perror("sockfd");
        return -1;
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    
    
    ser.sin_family = AF_INET;
    ser.sin_port = htons(7777);
    ser.sin_addr.s_addr = dest_ip;
    
    int n = sizeof(ser);
    

    sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&ser, n);
 
    close(sockfd);
}

void *udp_thread(void *arg) {

    int i;

    rlink_get_localip("br-wan",g_local_ip);
    




    int server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_sockfd < 0)
    {
        perror("server_sockfd");
        return -1;
    }
    struct sockaddr_in ser, cli;
    ser.sin_family = AF_INET;
    ser.sin_port = htons(7777);
    ser.sin_addr.s_addr = inet_addr("0");
 
    int len = sizeof(ser);
 
    int ret = bind(server_sockfd, (struct sockaddr *)&ser, len);
    if(ret < 0)
    {
        perror("bind");
        return -1;
    }
 
    int n = sizeof(cli);
    char server_buf[64] = {0};
    
    while (1) {
        char client_ip[32];
        int m = recvfrom(server_sockfd, server_buf, 64, 0, (struct sockaddr *)&cli, &n);
        strcpy(client_ip,inet_ntoa(cli.sin_addr));
        printf("client ip:%s  client port:%d\n",client_ip, ntohs(cli.sin_port));
        printf("len = %d\n", m);
        printf("%s\n", server_buf);
        memset(server_buf, 0, 64);
        if (strcmp(g_local_ip,client_ip) == 0) {
            printf("equal\r\n");
        } else {
            execute_cmd("ubus call channel_score scan '{\"code\":8,\"band\":5}'",NULL);
        }


    }
 
    close(server_sockfd);
}