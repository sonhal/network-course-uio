#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>



void server(){
    int rc;
    printf("I am a server!\n");

    rc = bind(so_l)
}


void client(){
    printf("I am a client!\n");
}

int main (int argc, char *argv[]){

    char *path;
    int  so;

    printf("Hello!\n");

    path = argv[1];

     /*unlink(path); */

    so = socket(PF_UNIX, SOCK_STREAM, 0);
    if(so == -1){
        perror("socket");
        return -1;
    }

    printf("socket fd: %d\n", so);

    struct sockaddr_un so_name;
    so_name.sun_family = PF_UNIX;
    strncpy(so_name.sun_path, path, sizeof(so_name.sun_path) - 1);

    if (strstr(argv[0], "hellod") != NULL){
        server();
    } else {
        client();
    }    

    return 0;
}

