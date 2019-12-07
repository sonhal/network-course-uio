#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <stdint.h>

#include "definitions.h"

struct ping_message {
    uint8_t dst_mip_addr;
    char content[32];
};

typedef struct ApplicationMessage {
    MIP_ADDRESS mip_src;
    struct ping_message message;
} ApplicationMessage;

#define ApplicationMessage_destroy(AM) if((AM)) free(AM)

#endif