#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h> 
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>

#include "../../../commons/src/polling.h"
#include "../../../commons/src/dbg.h"
#include "../../../commons/src/application.h"
#include "../../../commons/src/definitions.h"
#include "../../../commons/src/mipd_message.h"
#include "app_connection.h"
#include "link.h"
#include "mip_arp.h"
#include "packaging/mip_header.h"
#include "packaging/mip_package.h"
#include "interface.h"
#include "server.h"
#include "router/routing.h"
#include "router/forwarding.h"

#define INTERFACE_BUF_SIZE 10
#define MIP_MESSAGE_BUF 1600

extern void DumpHex(const void* data, size_t size);


void MIPDServer_destroy(MIPDServer *server){
    if(server){
        if(server->cache) free(server->cache);
        if(server->i_table) free(server->i_table);
        LocalSocket_destroy(server->route_socket);
        LocalSocket_destroy(server->forward_socket);
        LocalSocket_destroy(server->app_socket);
        ForwardQueue_destroy(server->forward_queue);
        free(server);
    }
}

MIPDServer *MIPDServer_create(LocalSocket *app_socket, LocalSocket *route_socket, LocalSocket *forward_socket, struct interface_table *table, int debug_enabled, long cache_update_freq_milli) {
    MIPDServer *server = calloc(1, sizeof(MIPDServer));
    server->app_socket = app_socket;
    server->route_socket = route_socket;
    server->forward_socket = forward_socket;
    server->i_table = table;
    server->cache = create_cache(cache_update_freq_milli);
    server->forward_queue = ForwardQueue_create();
    server->debug_enabled = debug_enabled;
    return server;
}

// Returns file descriptor for the accepted socket on success, -1 on failure
int handle_domain_socket_connection(MIPDServer *server, int epoll_fd, struct epoll_event *event){
    int rc = 0;

    MIPDServer_log(server, "Received message on domain socket");
    int new_socket = 0;
    new_socket = accept(event->data.fd, NULL, NULL);
    MIPDServer_log(server, "new connection bound to socket: %d", new_socket);
    struct epoll_event conn_event = create_epoll_in_event(new_socket);
    rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &conn_event);
    check(rc != -1, "Failed to add file descriptor to epoll");
    return new_socket;

    error:
        log_warn("Failed to handle domain socket connection");
        return -1;
}

void handle_domain_socket_disconnect(MIPDServer *server, struct epoll_event *event){
    MIPDServer_log(server, "Client on domain socket: %d disconnected", event->data.fd);
    close(event->data.fd);
}

// returns MIP header tra which descripes the type of package received
int handle_raw_socket_frame(MIPDServer *server, struct epoll_event *event){
    int rc = 0;
    struct sockaddr_ll *active_interface_so_name;
    MIPPackage *received_package = MIPPackage_create_empty();
    check_mem(received_package);

    rc = recv_raw_mip_package(event->data.fd, received_package);
    //rc = receive_raw_mip_packet(event->data.fd, &e_frame, &received_so_name, &received_header);
    check(rc != -1, "Failed to receive from raw socket");

    int i_pos = get_interface_pos_for_socket(server->i_table, event->data.fd);
    active_interface_so_name = server->i_table->interfaces[i_pos].so_name;


    // LOGG received packet to console
    char *received_package_str = MIPPackage_to_string(received_package);
    MIPDServer_log(server, " RECEIVED PACKET:\n%s", received_package_str);
    free(received_package_str);
    if(server->debug_enabled)print_cache(server->cache);

    if (received_package->m_header.tra == 0){
        // MIP arp response
        rc = append_to_cache(server->cache, event->data.fd, received_package->m_header.src_addr, active_interface_so_name->sll_addr);
        check(rc != -1, "Failed to add MIP ARP response to cache");
        
    } else if(received_package->m_header.tra == 1){
        // MIP arp request
        rc = handle_mip_arp_request(server->cache, received_package, &server->i_table->interfaces[i_pos]);
        check(rc != -1, "Failed to handle mip arp request");

    } else if (received_package->m_header.tra == 2){
        // MIP route table package
        rc = recv_route_table_broadcast(server, received_package);
        check(rc != -1, "Failed to receive new route table");

    }else if (received_package->m_header.tra == 3){
        MIPDServer_log(server,"Request is transport type request");

         // check if the request is to a mip address the mipd owns
        rc = get_interface_pos_for_mip_address(server->i_table, received_package->m_header.dst_addr);
        if(rc != -1){
            // MIPPackage is to this host
            rc = handle_MIPPackage_for_application(server, received_package);
            check(rc != -1, "Failed to deliver package to local application");
        } else {
            // Forward package if TTL is not 0
            int timetolive = received_package->m_header.ttl;
            if(received_package->m_header.ttl == 0){
                MIPDServer_log(server, "Dropping received package from mip src: %d\tto dst %d\tTTL is %d",
                                        received_package->m_header.src_addr,
                                        received_package->m_header.dst_addr,
                                        timetolive);

            }else {
                received_package->m_header.ttl--;
                rc = request_forwarding(server, received_package->m_header.dst_addr, received_package);
                check(rc != -1, "Failed to request forwarding for received package");
                return 0;
            }
        }
    }

    if(received_package) MIPPackage_destroy(received_package);
    return 0;

    error:
        if(received_package) MIPPackage_destroy(received_package);
        return -1;
}

// Tries not send message to a direct neighbor if it is located in the cache
// Does not free message
// Handle a request to send a message on the domain socket
// Returns 1 on success, the message was sent to a direct mipd neighbor
// Returns 0 if mip address could not be found amongst direct neighbors
// Returns -1 on critical error
int handle_domain_socket_request(MIPDServer *server, MIPDMessage *message){
    int rc = 0;
    struct mip_header *m_header = NULL;
    struct ether_frame *e_frame = NULL;
    MIPPackage *m_packet = NULL;

    int sock = query_mip_address_src_socket(server->cache, message->mip_address);
    if(sock == -1){
        MIPDServer_log(server, "could not lockate mip address: %d in cache", message->mip_address);
        return 0;
    }

    int i_pos = get_interface_pos_for_socket(server->i_table, sock);
    check(i_pos != -1, "Could not locate sock address in interface table");

    // set src mip address
    uint8_t src_mip_addr = server->i_table->interfaces[i_pos].mip_address;

    struct sockaddr_ll *sock_name = server->i_table->interfaces[i_pos].so_name;
    int cache_pos = query_mip_address_pos(server->cache, message->mip_address);
    check(cache_pos != -1, "Could not locate cache pos");


    // Create headers for the message
    e_frame = create_ethernet_frame(server->cache->entries[cache_pos].dst_interface, sock_name);
    m_header = create_transport_mip_header(src_mip_addr, message->mip_address);

    // Create MIP packet
    m_packet = MIPPackage_create(e_frame, m_header, message->data, message->data_size);

    // Send the message
    rc = sendto_raw_mip_package(sock, sock_name, m_packet);
    check(rc != -1, "Failed to send transport packet");
    MIPDServer_log(server, " Domain message sent");

    free(e_frame);
    free(m_header);
    return 1;

    error:
        if(e_frame)free(e_frame);
        if(m_header)free(m_header);
        return -1;
}


/*
Main server function. Starts the server loop and contains the entry for branching into different event handlers
*/
int MIPDServer_run(MIPDServer *server, int epoll_fd, struct epoll_event *events, int event_num, int timeout) {
    int rc = 0;
    int running = 1;
    int event_count = 0;
    size_t bytes_read = 0;
    BYTE read_buffer[MAX_MIPMESSAGE_SIZE];

    rc = complete_mip_arp(server->i_table, server->cache);
    check(rc != -1, "Failed to complete mip arp");

    while(running){
        MIPDServer_log(server," Polling...");
        update_arp_cache(server->i_table, server->cache);

        event_count = epoll_wait(epoll_fd, events, event_num, timeout);
        int i = 0;
        for(i = 0; i < event_count; i++){
            memset(read_buffer, '\0', MAX_MIPMESSAGE_SIZE);
            bytes_read = 0;

            // Event on the listening local domain socket, should only be for new connections
            if(events[i].data.fd == server->app_socket->listening_socket_fd){
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->app_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New route route daemon connection on route socket
            else if(events[i].data.fd == server->route_socket->listening_socket_fd)
            {
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->route_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New route deamon connection on forward socket
            else if(events[i].data.fd == server->forward_socket->listening_socket_fd)
            {
                int accepted_socket = handle_domain_socket_connection(server, epoll_fd, &events[i]);
                check(accepted_socket != -1, "Failed to accept connection on domain socket");
                server->forward_socket->connected_socket_fd = accepted_socket;
                continue;
            }

            // New request from routerd to broadcast route table
            else if(events[i].data.fd == server->route_socket->connected_socket_fd)
            {
                MIPRouteTablePackage *table_package = calloc(1, sizeof(MIPRouteTablePackage));
                check_mem(table_package);
                rc = read_from_domain_socket(events[i].data.fd , table_package, sizeof(MIPRouteTablePackage));
                check(rc != -1, "Failed to read route table package from routerd");
                if(rc == 0){
                    handle_domain_socket_disconnect(server, &events[i]);
                    server->route_socket->connected_socket_fd = -1;
                } else {
                    rc = broadcast_route_table(server, table_package);
                    check(rc != -1, "Failed to broadcast route table");   
                }
                continue;
            }

            // New reponse from routerd on forwarding mip packet
            else if(events[i].data.fd == server->forward_socket->connected_socket_fd)
            {
                MIP_ADDRESS *forward_response = calloc(1, sizeof(MIP_ADDRESS));
                check_mem(forward_response);

                MIPDServer_log(server, "Route forward event");
                rc = read_from_domain_socket(events[i].data.fd , forward_response, sizeof(MIP_ADDRESS));
                check(rc != -1, "Failed to read route forward response from routerd");

                if(rc == 0){
                    handle_domain_socket_disconnect(server, &events[i]);
                    server->forward_socket->connected_socket_fd = -1;
                } else {
                    if(forward_found(*forward_response)){
                        rc = handle_forward_response(server, *forward_response);
                        check(rc != -1, "Failed to handle forward response from routerd");
                    } else {
                        MIPPackage *package = ForwardQueue_pop(server->forward_queue);
                        check(package != NULL, "Invalid package popped from ForwardQueue");
                        MIPDServer_log(server, "Forward not found for destination: %d", package->m_header.dst_addr);
                        MIPPackage_destroy(package);
                    }
                }
                free(forward_response);
                continue;
            }

            // Raw socket event
            else if(is_socket_in_table(server->i_table, events[i].data.fd)){
                MIPDServer_log(server, "raw socket packet");
                rc = handle_raw_socket_frame(server, &events[i]);
                check(rc != -1, "Failed to handle raw socket package");
                continue;
            }

            // Application socket event
            else if(events[i].data.fd == server->app_socket->connected_socket_fd){
                MIPDMessage *message = NULL;
                MIPPackage *message_package = NULL;
                MIPDServer_log(server, "application socket event");
                bytes_read = recv(events[i].data.fd, read_buffer, MAX_MIPMESSAGE_DATA_SIZE, 0);

                if(bytes_read == 0 || bytes_read == -1){
                    handle_domain_socket_disconnect(server, &events[i]);
                    server->app_socket->connected_socket_fd = -1;
                } else if(bytes_read < sizeof(MIPDMessage)){
                    log_warn("Received invalid or corrupted message from connected application");
                } else {
                    // Parse message on domain socket
                    printf("bytes read: %d\n", bytes_read);
                    message = MIPDMessage_deserialize(read_buffer);
                    MIPDServer_log(server, "application message - dst:%d\tsize: %d", message->mip_address, message->data_size);

                    rc = handle_domain_socket_request(server, message);
                    check(rc != -1, "Failed to handle domain socket event");
                    if(rc == 0){
                        // MIP address is not a neighbor
                        message_package = create_queueable_MIPDMessage_MIPPackage(message);
                        check(message_package != NULL, "Failed to create ping message MIPPackage");
                        rc = request_forwarding(server, message_package->m_header.dst_addr, message_package);
                        check(rc != -1, "Failed to request forwarding");
                        if(rc == 0){
                            // No palce to send package, drop it
                            MIPPackage_destroy(message_package);
                        }
                    }
                    // Clean up message as it has been send to neighbor or the data has been safe copied
                    MIPDMessage_destroy(message);
                }
                continue;
            }

            // stdin event
            else if(events[i].data.fd == 0){
                bytes_read = read(events[i].data.fd, read_buffer, MAX_MIPMESSAGE_SIZE);
                
                if(!strncmp(read_buffer, "stop\n", 5)){
                    running = 0;
                    log_info("Exiting...");
                } else {
                    printf("unkown command\n");
                }
                continue;
            }
        }
    }

    MIPDServer_destroy(server);
    return 1;

    error:
        MIPDServer_destroy(server);
        return -1;
}
