#ifndef _MIPTP_CLIENT_PACKAGE_H
#define _MIPTP_CLIENT_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>

#include "definitions.h"

#define MAX_DATA_SIZE_BYTES 65535

enum MIPTPClientType {MIPTP_SENDER = 1, MIPTP_RECEIVER = 2};

typedef struct ClientPackage {
    uint16_t port;
    MIP_ADDRESS destination;
    enum MIPTPClientType type;
    uint16_t data_size;
    BYTE *data;
} ClientPackage;


/*  Creates a byte array representation of the ClientPackage.
    Format:
        - port: 2 bytes
        - destination: 1 byte
        - type: 4 bytes
        - data_size: 2 bytes
        - data: max 65535 bytes
    Returns the size of the size of the serialized package
*/
int ClientPackage_serialize(BYTE *buffer, ClientPackage *package);

ClientPackage *ClientPackage_deserialize(BYTE *serialized_package);

void ClientPackage_serialized_get_port(BYTE *s_package, uint16_t *port);

void ClientPackage_serialized_get_destination(BYTE *s_package, MIP_ADDRESS *destination);

void ClientPackage_serialized_get_type(BYTE *s_package, enum MIPTPClientType *type);

void ClientPackage_serialized_get_data_size(BYTE *s_package, uint16_t *data_size);

void ClientPackage_serialized_get_data(BYTE *s_package, BYTE *data);

ClientPackage *ClientPackage_create(uint16_t port, MIP_ADDRESS destination, enum MIPTPClientType type, BYTE *data, uint16_t data_size);

void ClientPackage_destroy(ClientPackage *package);

#endif