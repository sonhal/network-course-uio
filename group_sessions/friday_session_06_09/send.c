#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>



extern void DumpHex(const void* data, size_t size);

struct ether_frame
{
    uint8_t dst_addr[6];
    uint8_t src_addr[6];
    uint8_t eth_proto[6];
    uint8_t contents[0];
}__attribute__((packet));


#define BUF_SIZE 1600
#define ETH_BROADCAST_ADDR{0x1,0x2,0x3,0x1,0x2,0x3,0x1,0x2}

int main(int argc, char *argv[]){

    int so;

    so = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if((so == -1)){
        perror("socket");
        return 1;
    }

    struct ether_frame frame_hdr;
    uint8_t bcast[6] = ETH_BROADCAST_ADDR;

    return 0;
}
