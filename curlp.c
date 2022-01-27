#include <stdio.h>
#include <string.h>

typedef struct {
    char *protocol;
    char *host;
    char *path;
    int port;
} URL;

char tolower(char ch){
    if(ch >= 'A' && ch <= 'Z'){
        ch = 'a' + (ch - 'A');
    }
    return ch;
}

URL toURL(char *buf){
    URL url = {};

    int len = strlen(buf), i = 0, deli;

    url.protocol = malloc(20);
    for(; i < 20; i++){
        if(buf[i] == ':' || buf[i] == '\0'){
            break;
        }
        url.protocol[i] = tolower(buf[i]);
    }

    url.protocol[i] = '\0';
    i += 3;

    url.host = malloc(len);
    if(buf[i] == '['){
        i++;
        deli = i;

        for(; i < len; i++){
            if(buf[i] == ']'){
                break;
            }
            url.host[i-deli] = buf[i];
        }

        url.host[i-deli] = '\0';
        i++;

    }else{
        deli = i;

        for(; i < len; i++){
            if(buf[i] == ':' || buf[i] == '/'){
                break;
            }
            url.host[i-deli] = buf[i];
        }

        url.host[i-deli] = '\0';
    }

    if(buf[i] == ':'){
        char port[6] = "";
        i++;
        deli = i;

        for(; i < len; i++){
            if(buf[i] >= '0' && buf[i] <= '9'){
                port[i-deli] = buf[i];
                continue;
            }
            break;
        }
        port[i-deli] = '\0';
        url.port = atoi(port);
    }

    deli = i;

    url.path = malloc(len-deli);
    for(; i < len; i++){
        url.path[i-deli] = buf[i];
    }

    url.path[len-deli-1] = '\0';

    return url;
}
