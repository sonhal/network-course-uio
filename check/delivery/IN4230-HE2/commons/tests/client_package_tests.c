#include <string.h>

#include "../src/dbg.h"
#include "minunit.h"

#include "../src/client_package.h"


char *test_ClientPackage_serialize() {
    ClientPackage *package = ClientPackage_create(10, 110, MIPTP_SENDER, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPackage");
    
    int s_size = 0;
    BYTE *serialized_package = calloc(100, sizeof(char));
    s_size = ClientPackage_serialize(serialized_package, package);
    mu_assert(serialized_package != NULL, "Failed to serialize package");
    mu_assert(s_size != 0, "Failed to serialize package, size is 0");
    char *ser_string = calloc(MAX_DATA_SIZE_BYTES, sizeof(BYTE));

    ClientPackage_serialized_get_data(serialized_package, ser_string);
    mu_assert(strncmp(package->data, ser_string, package->data_size) == 0, "Serialized data does not match original data");

    MIP_ADDRESS destination = 0;
    ClientPackage_serialized_get_destination(serialized_package, &destination);
    mu_assert(destination == 110, "Failed to get destination from serialized package");

    uint16_t port = 0;
    ClientPackage_serialized_get_port(serialized_package, &port);
    mu_assert(port == 10, "Failed to get port from serialized package");

    enum MIPTPClientType type = 1;
    ClientPackage_serialized_get_type(serialized_package, &type);
    mu_assert(type == MIPTP_SENDER, "Failed to get type from serialized package");


    ClientPackage_destroy(package);
    free(serialized_package);
    return NULL;
}

char *test_ClientPackage_deserialize() {
    ClientPackage *package = ClientPackage_create(1, 11, MIPTP_SENDER, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPackage");
    
    int s_size = 0;
    BYTE *serialized_package = calloc(100, sizeof(char));
    s_size = ClientPackage_serialize(serialized_package, package);
    mu_assert(serialized_package != NULL, "Failed to serialize package");
    ClientPackage *package2 = ClientPackage_deserialize(serialized_package);

    mu_assert(package2 != NULL, "Failed to deserialize package, pointer is NULL");
    mu_assert(package->port == package2->port, "Bad deserialization, port is not equal");
    mu_assert(package->destination == package2->destination, "Bad deserialization, destination is not equal");
    mu_assert(package->type == package2->type, "Bad deserialization, type is not equal");
    mu_assert(package->data_size == package2->data_size, "Bad deserialization, data_size is not equal");
    mu_assert(strncmp(package->data, package2->data, package->data_size) == 0, "Bad deserialization, data is not equal");


    ClientPackage_destroy(package);
    ClientPackage_destroy(package2);
    free(serialized_package);
    return NULL;
}

char *test_ClientPackage_deserialize_no_data_package() {
    ClientPackage *package = ClientPackage_create(1, 11, MIPTP_SENDER, "", 0);
    mu_assert(package != NULL, "Failed to create MIPTPackage");
    
    int s_size = 0;
    BYTE *serialized_package = calloc(100, sizeof(char));
    s_size = ClientPackage_serialize(serialized_package, package);
    mu_assert(serialized_package != NULL, "Failed to serialize package");
    ClientPackage *package2 = ClientPackage_deserialize(serialized_package);

    mu_assert(package2 != NULL, "Failed to deserialize package, pointer is NULL");
    mu_assert(package->port == package2->port, "Bad deserialization, port is not equal");
    mu_assert(package->destination == package2->destination, "Bad deserialization, destination is not equal");
    mu_assert(package->data_size == package2->data_size, "Bad deserialization, data_size is not equal");
    mu_assert(strncmp(package->data, package2->data, package->data_size) == 0, "Bad deserialization, data is not equal");


    ClientPackage_destroy(package);
    ClientPackage_destroy(package2);
    free(serialized_package);
    return NULL;
}

char *test_ClientPackage_create_destroy() {
    ClientPackage *package = ClientPackage_create(10, 110, MIPTP_SENDER, "Hello", strlen("Hello"));
    mu_assert(package != NULL, "Failed to create MIPTPackage");
    mu_assert(package->data_size == strlen("Hello"), "Failed to set data_size");
    mu_assert(package->destination == 110, "Failed to set destination");
    mu_assert(package->port == 10, "Failed to set port");
    mu_assert(strncmp("Hello", package->data, strlen("Hello")) == 0, "Failed to set data");

    ClientPackage_destroy(package);
    package = NULL;
    ClientPackage_destroy(package);

    return NULL;
}

char *all_tests(){

    mu_suite_start();

    mu_run_test(test_ClientPackage_create_destroy);
    mu_run_test(test_ClientPackage_serialize);
    mu_run_test(test_ClientPackage_deserialize);
    mu_run_test(test_ClientPackage_deserialize_no_data_package);

    return NULL;
}

RUN_TESTS(all_tests);