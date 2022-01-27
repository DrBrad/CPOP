#include <stdio.h>

//ADJUST THESE INCLUDES...
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "curlp.c"

void gateway(char *buffer, struct in_addr sin_addr){
    if(strncmp(buffer, "HTTP/1.", 7) == 0 || strncmp(buffer, "NOTIFY *", 8) == 0){
        char location[1024];

        char *token = strtok(buffer, "\n");
        while(token){
            if(strncasecmp(token, "location: ", 10) == 0){
                for(int i = 10; i < strlen(token); i++){
                    location[i-10] = token[i];
                }
                break;
            }

            token = strtok(NULL, "\n");
        }

        if(strlen(location) > 0){

            URL url = toURL(location);

            int sockfd;

            printf("%s  %s\n", url.host, inet_ntoa(sin_addr));

            if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0){
                struct sockaddr_in servaddr, cliaddr;

                memset(&servaddr, 0, sizeof(servaddr));
                memset(&cliaddr, 0, sizeof(cliaddr));

                cliaddr.sin_family = AF_INET;
                cliaddr.sin_addr = sin_addr;
                cliaddr.sin_port = 0;

                if(bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) >= 0){
                    //struct sockaddr_in servaddr;
                    servaddr.sin_family = AF_INET; //IPv4  - I THINK...
                    servaddr.sin_port = htons(url.port);
                    servaddr.sin_addr.s_addr = inet_addr(url.host);

                    struct timeval timeout = {
                        .tv_sec = 5,
                        .tv_usec = 0
                    };
                    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

                    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0){
                      //SEND
                      char buffer[4096];
                      sprintf(buffer, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nAccept-Language: en\r\n\r\n", url.path, url.host, url.port);
                      send(sockfd, buffer , strlen(buffer), 0);

                      //HEADER
                      int valread = read(sockfd, buffer, 1024);
                      printf("%s", buffer);

                      //CONTENT
                      valread = read(sockfd, buffer, 4096);
                      printf("%s\n\n\n\n\n", buffer);
                    }
                }

                close(sockfd);
            }
        }
    }
}
