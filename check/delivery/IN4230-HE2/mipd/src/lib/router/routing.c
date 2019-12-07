#include <limits.h>

#include "../../../../routerd/src/lib/mip_route_table.h"
#include "../../../../commons/src/dbg.h"
#include "../../../../commons/src/list.h"


#include "routing.h"
#include "../server.h"
#include "../interface.h"
#include "../link.h"
#include "../mip_arp.h"


MIPRouteTablePackage *recv_table_to_be_broadcasted(int socket_fd){
    int rc = 0;
    MIPRouteTablePackage *table_packet = calloc(1, sizeof(MIPRouteTablePackage));

    rc = recv(socket_fd, table_packet, sizeof(MIPRouteTablePackage), 0);
    check(rc != -1, "Failed to receive table packet");

    //memcpy(table_packet, raw_packet, sizeof(MIPRouteTablePackage));

    return table_packet;

    error:
        MIPRouteTablePackage_destroy(table_packet);
        return NULL;
}

MIPRouteTablePackage *parse_broadcasted_table(MIPPackage *package){
    MIPRouteTablePackage *table_packet = calloc(1, sizeof(MIPRouteTablePackage));

    memcpy(table_packet, package->message, sizeof(MIPRouteTablePackage));
    check_mem(table_packet);

    return table_packet;

    error:
        MIPRouteTablePackage_destroy(table_packet);
        return NULL;
}

// Poisons the table where next hop is destination, returns void
void poison_reverse(MIPRouteTablePackage *package, MIP_ADDRESS destination){
    int i = 0;
    for(i = 0; i < package->num_entries; i++){
        if(package->entries[i].next_hop == destination){
            package->entries[i].cost = UINT_MAX;
        }
    }
}

// Broadcasts a routing table to the configured interfaces
// Handles the Split horizon poisoning before sending table
// Returns 1 on success, -1 on failure
int broadcast_route_table(MIPDServer *server, MIPRouteTablePackage *table_package){
    int rc = 0;
    uint8_t ether_broadcast_address[] = ETH_BROADCAST_ADDR;
    MIPRouteTablePackage *tmp = calloc(1, sizeof(MIPRouteTablePackage));

    int i = 0;
    for(i = 0; i < server->i_table->size; i++){

        memcpy(tmp, table_package, sizeof(MIPRouteTablePackage));
        tmp->table_address = server->i_table->interfaces[i].mip_address;
        MIP_ADDRESS destination = query_cache_socket_mip_address(server->cache, server->i_table->interfaces[i].raw_socket);
        if(destination == 255){
            MIPDServer_log(server, "Failed to get destination mip address raw socket connects to, might not be connected to another mipd node");
            destination = tmp->table_address;
        }
        poison_reverse(tmp, destination);

        MIPDServer_log(server, "Broadcasting route table\tnum_entries: %d\tmip address: %d", tmp->num_entries, tmp->table_address);
        MIPPackage *package = MIPPackage_create_raw(server->i_table->interfaces[i].mip_address,
                                                    server->i_table->interfaces[i].interface,
                                                    255,
                                                    ether_broadcast_address,
                                                    (BYTE *)tmp,
                                                    sizeof(MIPRouteTablePackage),
                                                    2);

        int out_socket = server->i_table->interfaces[i].raw_socket;
        struct sockaddr_ll *so_name = server->i_table->interfaces[i].so_name;
        rc = sendto_raw_mip_package(out_socket, so_name, package);
        check(rc != -1, "Failed to send broadcast table package");
    }

    MIPRouteTablePackage_destroy(table_package);
    MIPRouteTablePackage_destroy(tmp);
    return 1;

    error:
        MIPRouteTablePackage_destroy(tmp);
        MIPRouteTablePackage_destroy(table_package);
        return -1;
}

// Handles a received route table broadcast from a neighbor mipd, returns 1 on success, -1 on failure, 0 if no routerd is connected to receive the table
int recv_route_table_broadcast(MIPDServer *server, MIPPackage *package){
    if(server->route_socket->connected_socket_fd == -1){
        MIPDServer_log(server, "No routerd connected to receive route table");
        return 0;
    }
    int rc = 0;

    MIPRouteTablePackage *table = parse_broadcasted_table(package);
    check_mem(table);

    rc = write(server->route_socket->connected_socket_fd, table, sizeof(MIPRouteTablePackage));
    check(rc != -1, "Failed to send received route table to local routerd");

    return 1;

    error:
        return -1;
}