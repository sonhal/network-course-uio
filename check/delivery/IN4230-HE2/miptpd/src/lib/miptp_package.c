#include <stdlib.h>
#include <string.h>

#include "../../../commons/src/dbg.h"

#include "miptp_package.h"


MIPTPPackage *MIPTPPackage_create(uint16_t port, uint16_t PSN, BYTE *data, uint16_t data_size){
    check(data_size <= MAX_MIPTP_PACKAGE_DATA_SIZE, "data_size is to large");

    MIPTPPackage *package = calloc(1, sizeof(MIPTPPackage));
    check_mem(package);

    package->miptp_header.PL = calc_pl(data_size);
    package->miptp_header.port = port;
    package->miptp_header.PSN = PSN;

    package->data = calloc(data_size, sizeof(BYTE));
    check_mem(data);
    package->data_size = data_size;
    memcpy(package->data, data, package->data_size);

    return package;

    error:
        log_err("Failed to create MIPTPPackage in MIPTPPackage_create");
        return NULL;
}

MIPTPPackage *MIPTPPackage_create_raw(uint8_t PL, uint16_t port, uint16_t PSN, BYTE *data, uint16_t data_size){
    MIPTPPackage *package = calloc(1, sizeof(MIPTPPackage));
    package->miptp_header.PL = PL;
    package->miptp_header.port = port;
    package->miptp_header.PSN = PSN;

    package->data = calloc(data_size, sizeof(BYTE));
    check_mem(data);
    package->data_size = data_size;
    memcpy(package->data, data, package->data_size);

    return package;

    error:
        log_err("Failed to create MIPTPPackage in MIPTPPackage_create");
        return NULL;
}

void MIPTPPackage_destroy(MIPTPPackage *package){
    if(package){
        if(package->data){
            free(package->data);
        }
        free(package);
    }
}

// Serializes the contents of the MIPTPPackage into the buffer
// On success returns the byte size of the serialized package
// Returns -1 on failure
uint16_t MIPTPPackage_serialize(BYTE *buffer, MIPTPPackage *package){
    // Size of MIPTPPackage minus the data pointer
    uint16_t package_size = sizeof(MIPTPHeader);
    package_size += package->data_size;
    check(package_size <= MAX_MIPTP_PACKAGE_SIZE, "deserialized package size is to large");

    memcpy(buffer, &package->miptp_header, sizeof(MIPTPHeader));
    memcpy(&buffer[sizeof(MIPTPHeader)], &package->data_size, sizeof(uint16_t));
    memcpy(&buffer[sizeof(MIPTPHeader) + sizeof(uint16_t)], package->data, package->data_size);

    return package_size;

    error:
        log_err("Failed to serialize MIPTPPackage in MIPTPPackage_serialize");
        return -1;
}

MIPTPPackage *MIPTPPackage_deserialize(BYTE *s_package, uint16_t package_size){

    MIPTPHeader header = {};
    uint16_t size_of_header = sizeof(MIPTPHeader);
    uint16_t data_size = package_size - size_of_header;

    BYTE *data = calloc(data_size, sizeof(BYTE));
    check_mem(data);

    MIPTPPackage_serialized_get_data(s_package, data, data_size);
    MIPTPPackage_serialized_get_header(s_package, &header);

    MIPTPPackage *package = MIPTPPackage_create_raw(header.PL, header.port, header.PSN, data, data_size);
    check(package != NULL, "Failed to create MIPTPPackage for deserialized data");

    return package;

    error:
        log_err("Failed to deserialize MIPTPPackage in MIPTPPackage_deserialize");
        return NULL;
}

MIPTPPackage_serialized_get_header(BYTE *s_package, MIPTPHeader *header){
    memcpy(header, s_package, sizeof(MIPTPHeader));
}

MIPTPPackage_serialized_get_data(BYTE *s_package, BYTE *data, uint16_t data_size){
    memcpy(data, &s_package[sizeof(MIPTPHeader)], data_size);
}

uint8_t calc_pl(uint16_t data_size){
    if(data_size % 4 != 0){
        return 4 - (data_size % 4); 
    }
    return 0;
}