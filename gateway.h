#include <stdio.h>
#include <ctype.h>

//#define aa 100
//int bb = 22;

/*
HTTP/1.1 200 OK
Server: Custom/1.0 UPnP/1.0 Proc/Ver
EXT:
Location: http://192.168.0.1:5431/dyndev/uuid:98834f4a-5fc7-4a52-8535-dae7a5983a4b
Cache-Control:max-age=1800
ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1
USN:uuid:98834f4a-5fc7-4a52-8535-dae7a5983a4b::urn:schemas-upnp-org:device:InternetGatewayDevice:1


M-SEARCH * HTTP/1.1
HOST: 239.255.255.250:1900
ST: urn:schemas-upnp-org:service:WANIPConnection:1
MAN: "ssdp:discover"
MX: 2


M-SEARCH * HTTP/1.1
HOST: 239.255.255.250:1900
ST: urn:schemas-upnp-org:service:WANPPPConnection:1
MAN: "ssdp:discover"
MX: 2


*/

void gateway(char *buffer, struct in_addr sin_addr){

    //printf("%ld\n", sizeof());
    if(strncmp(buffer, "HTTP/1.", 7) == 0 || strncmp(buffer, "NOTIFY *", 8) == 0){
        char location[1024];

        char* token = strtok(buffer, "\n");
        while(token){
            char lower[strlen(token)];
            for(int i = 0; i < strlen(token); i++){
                lower[i] = tolower(token[i]);
            }

            if(strncmp(lower, "location: ", 10) == 0){
                for(int i = 10; i < strlen(token); i++){
                    location[i-10] = token[i];
                }
                //location[strlen(token)-11] = '\0';
                break;
            }

            token = strtok(NULL, "\n");
        }

        if(strlen(location) > 0){

            //CONNECT TO LOCATION USING TCP SOCKET








            printf("LOC: %s\n", location);
            //printf("ADD: %s\n", inet_ntoa(sin_addr));
        }

        //printf("asdasdasd\n");
        //printf("%s\n", buffer);
    }

    //printf("%s\n", buffer);


}
