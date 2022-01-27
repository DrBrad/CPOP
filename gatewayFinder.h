//ADDRESS FINDER
#include <ifaddrs.h>
#include <arpa/inet.h>

//THREAD
#include <pthread.h>

//SOCKET  - TO DETERMINE...
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

//BULLSHIT
#include <stdio.h> //REMOVE LATER
#include <stdlib.h> //REMOVE LATER
#include <string.h>

//#include "curlp.c"
#include "gateway.h"

typedef struct {
    struct in_addr sin_addr;
    int schema;
} net_interface;

char *schemas[] = {
    "urn:schemas-upnp-org:device:InternetGatewayDevice:1",
    "urn:schemas-upnp-org:service:WANIPConnection:1",
    "urn:schemas-upnp-org:service:WANPPPConnection:1"
};

pthread_mutex_t pause_lock;

int total_gateways = 0;

#define MAXLINE 1536

extern void *gatewayListener(void *arg);



void gatewayFinder(){
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;

    getifaddrs(&ifap);

    for(ifa = ifap; ifa; ifa = ifa->ifa_next){
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET){
            sa = (struct sockaddr_in *) ifa->ifa_addr;

            if(sa->sin_addr.s_addr != htonl(INADDR_LOOPBACK)){ //NO LOOPBACK - LOCALHOST
                for(int i = 0; i < sizeof(schemas)/sizeof(schemas[0]); i++){
                    net_interface *netinter = malloc(sizeof(net_interface));
                    netinter->sin_addr = sa->sin_addr;
                    netinter->schema = i;

                    pthread_t thread_id;
                    pthread_create(&thread_id, NULL, gatewayListener, netinter);

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
}

void *gatewayListener(void *arg){
    net_interface *netinter = arg;

    int sockfd;

    // Creating socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0){
        struct sockaddr_in servaddr, cliaddr;

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET;// IPv4
        servaddr.sin_port = htons(0);//   FROM PORT
        servaddr.sin_addr = netinter->sin_addr;

        if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0){
            // Filling server information
            cliaddr.sin_family = AF_INET;
            cliaddr.sin_port = htons(1900);//   TO PORT
            cliaddr.sin_addr.s_addr = inet_addr("239.255.255.250");//INADDR_ANY;   TO ADDRESS

            struct timeval timeout = {
                .tv_sec = 3,
                .tv_usec = 0
            };
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);


            char buffer[MAXLINE];
            sprintf(buffer, "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nST: %s\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\n\r\n", schemas[netinter->schema]);

            sendto(sockfd, (char *)buffer, strlen(buffer), MSG_CONFIRM, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

            int len;
            int n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            gateway(buffer, netinter->sin_addr);
        }

        close(sockfd);
    }

    pthread_mutex_lock(&pause_lock);
    total_gateways--;
    pthread_mutex_unlock(&pause_lock);

    //free(arg);
    pthread_exit(NULL);
}
