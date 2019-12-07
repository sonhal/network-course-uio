#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>


extern void DumpHex(const void* data, size_t size);

#define BUF_SIZE 1600

int main(int argc, char *argv[]){

    int so;

    so = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if((so == -1)){
        perror("socket");
        return 1;
    }

    do {
        int rc;
        char buf[BUF_SIZE];

        rc = recv(so, buf, BUF_SIZE, 0);
        if(rc == -1){
            perror("recv");
            return 1;
        }
        printf("Received Ethernet frame\n");
        DumpHex(buf, rc);
    }while (1);
    

    DumpHex(argv[0], strlen(argv[0]));

    return 0;
}
