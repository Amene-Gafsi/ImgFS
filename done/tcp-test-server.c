#include "socket_layer.h"
#include "imgfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_FILE_SIZE 1024
#define MAX_BUFFER_SIZE 2048

int main(int argc, char *argv[]) {  //TODO 

    uint16_t port = atoi(argv[1]);

    int server_fd = tcp_server_init(port);    int new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    printf("Server started on port %d\n", port);

    while(1) {
        new_socket = tcp_accept(server_fd);
        if (new_socket == -1){
            perror("accept");
            continue;
        }
        break;
    }

    char buffer[MAX_BUFFER_SIZE] = {0};
    valread = tcp_read(new_socket, buffer, MAX_BUFFER_SIZE);
    printf("Received a size: %s\n", buffer);

    int file_size = atoi(buffer);
    if (file_size < MAX_FILE_SIZE) {
        tcp_send(new_socket, "Small file", strlen("Small file"));
        memset(buffer, 0, MAX_BUFFER_SIZE);
    } else {
        tcp_send(new_socket, "File too large", strlen("File too large"));
    }

    close(new_socket);

    return 0;
}




