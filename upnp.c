#include <stdio.h>
#include "gatewayFinder.h"

//CHECK FOR BOGON TO DETERMINE IF WE ARE BEHIND A NAT TO BEGIN WITH...
//10.0.0.0/8,
//127.0.0.0/8,
//172.16.0.0/12,
//192.168.0.0/16
//169.254.0.0/16

//ENSURE WE ARN'T FUCKING WITH 127 - LOCAL... FUCKING POINTLESS...

//TRY THIS:   sockaddr_in   INSTEAD OF   in_addr

//UNFORTUNATLY VOLATILE DOESN'T HELP MULTI-THREAD VARS - A SEMAPHORE MUST BE USED


int main(int argc, char *argv[]){
    gatewayFinder();

    return 0;
}
