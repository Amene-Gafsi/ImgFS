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

int main(int argc, char *argv[])
{

    if (argc < 2)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    uint16_t port = (uint16_t)atoi(argv[1]);

    int server_fd = tcp_server_init(port);
    int new_socket;

    printf("Server started on port %d\n", port);

    while (1)
    {
        long valread = 0;

        printf("Waiting for a size...\n");
        while (1)
        {
            new_socket = tcp_accept(server_fd);
            if (new_socket != -1)
            {
                break;
            }
        }

        char buffer[MAX_BUFFER_SIZE] = {0};
        while (valread == 0)
        {
            valread = tcp_read(new_socket, buffer, MAX_BUFFER_SIZE);
        }

        printf("Received a size: %s --> ", buffer);

        int file_size = atoi(buffer);
        if (file_size <= 0 || file_size >= MAX_FILE_SIZE)
        {
            printf("rejected\n");
            tcp_send(new_socket, "NO", strlen("NO"));
            close(new_socket);
            continue;
        }

        tcp_send(new_socket, "OK", strlen("OK"));
        printf("accepted\n");
        printf("About to receive file of %d bytes\n", file_size);

        memset(buffer, 0, sizeof(buffer));
        int total = 0;
        while (total < file_size)
        {
            int reading = (int)tcp_read(new_socket, buffer + total, (size_t)(file_size - total));
            if (reading <= 0)
            {
                printf("problem\n");
                tcp_send(new_socket, "problem", strlen("problem"));
                close(new_socket);
                continue;
            }
            total += reading;
        }

        buffer[total] = '\0';
        printf("Received a file :\n%s\n", buffer);
        tcp_send(new_socket, "Received", strlen("Received"));
        close(new_socket);
    }

    return 0;
}
