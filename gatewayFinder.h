#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <ifaddrs.h>

#include <pthread.h> // https://github.com/openbsd/src/blob/master/include/pthread.h

#include "gateway.h"

//#define PORT    8080
#define MAXLINE 1536


struct net_inter {
    //struct servaddr *servaddr;
    struct in_addr sin_addr;
    int i;
};

int total_gateways = 0;

char *schemas[] = {
    "urn:schemas-upnp-org:device:InternetGatewayDevice:1",
    "urn:schemas-upnp-org:service:WANIPConnection:1",
    "urn:schemas-upnp-org:service:WANPPPConnection:1"
};

pthread_mutex_t pause_lock;


void *gatewayListener(void *arg){
    struct net_inter *net_inter = (struct net_inter*) arg;

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;// IPv4
    servaddr.sin_port = htons(0);//   FROM PORT
    servaddr.sin_addr = net_inter->sin_addr;
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//INADDR_ANY;   FROM ADDRESS
    //net_inter->servaddr->sin_family = AF_INET;
    //net_inter->servaddr->sin_port = htons(0);

    // Bind the socket with the server address
    //if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    //if(bind(sockfd, (struct sockaddr *)&net_inter->servaddr, sizeof(net_inter->servaddr)) < 0){
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("bind failed");
        //exit(EXIT_FAILURE);

    }else{
        // Filling server information
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_port = htons(1900);//   TO PORT
        cliaddr.sin_addr.s_addr = inet_addr("239.255.255.250");//INADDR_ANY;   TO ADDRESS

        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);


        char buffer[MAXLINE] = "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nST: ";
        strcat(buffer, schemas[net_inter->i]);
        strcat(buffer, "\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\n\r\n");

        //char buffer[] = "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nST: "+URN[i]+"\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\n\r\n";
        //sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
        sendto(sockfd, (char *)buffer, strlen(buffer), MSG_CONFIRM, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

        int len;
        int n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';


        //printf("%d  GATEWAY  %d   %s\n", total_gateways, net_inter->i, inet_ntoa(net_inter->sin_addr));
        gateway(buffer, net_inter->sin_addr);
    }

    close(sockfd);

    pthread_mutex_lock(&pause_lock);
    total_gateways--;
    pthread_mutex_unlock(&pause_lock);

    free(arg);
}






void gatewayFinder(){
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;

    getifaddrs(&ifap);

    for(ifa = ifap; ifa; ifa = ifa->ifa_next){
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET){
            sa = (struct sockaddr_in *) ifa->ifa_addr;

            if(sa->sin_addr.s_addr != htonl(INADDR_LOOPBACK)){ //NO LOOPBACK - LOCALHOST
                for(int i = 0; i < sizeof(schemas)/sizeof(schemas[0]); i++){
                    struct net_inter *net_inter;
                    net_inter = (struct net_inter*) malloc(sizeof(struct net_inter));

                    net_inter->sin_addr = sa->sin_addr;
                    net_inter->i = i;

                    pthread_t thread_id;
                    pthread_create(&thread_id, NULL, gatewayListener, net_inter);

                    pthread_mutex_lock(&pause_lock);
                    total_gateways++;
                    pthread_mutex_unlock(&pause_lock);
                }
            }
        }
    }

    freeifaddrs(ifap);

    while(total_gateways > 0){
        //sleep(0.1);
    }

    printf("DONE\n");
}
