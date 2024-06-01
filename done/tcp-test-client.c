#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "socket_layer.h"

#define SERVER_IP "127.0.0.1" 
#define MAX_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    uint16_t port = (uint16_t)atoi(argv[1]);
    const char *filename = argv[2];


    // Check if the file exists and get its size
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        return 1;
    }

    int file_size = (int)file_stat.st_size;

    if (file_size > MAX_SIZE) {
    return 1;
    }

    char size_str[20];
    snprintf(size_str, sizeof(size_str), "%d", file_size);   // Convert size to string

    // Create socket and connect to server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return 1;
    }
    
    struct sockaddr_in serv_addr; 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return 1;
    }

    printf("Talking to %d\n", port);
    printf("Sending size %d\n", file_size);

    // Send the file size to the server
    if (tcp_send(sock, size_str, strlen(size_str)) < 0) {
        return 1;
    }

    char ack[1024] = {0};

    while(1){
        if (tcp_read(sock, ack, sizeof(ack)) > 0) {
            printf("Server responsed: %s\n", ack);
            break;
        }
    }

    if (strcmp(ack, "OK") != 0) {
        printf("Rejected\n");
        return 1;
    }

     // Send the file to the server
    int file_fd = open(filename, 00);
    if (file_fd == -1) {
        return 1;
    }

    char file_buffer[file_size+1];
    file_buffer[file_size] = '\0';
    
    int bytes_read = (int)read(file_fd, file_buffer, (size_t)file_size);
    close(file_fd);
    if (bytes_read != file_size) {
        return 1;
    }

    printf("Sending %s\n", filename);

    if (tcp_send(sock, file_buffer, (size_t)file_size) < 0) {
        printf("Error sending file\n");
        return 1;
    }

    // Wait for final acknowledgment
    memset(ack, 0, sizeof(ack));
    if (tcp_read(sock, ack, sizeof(ack)) <= 0) {
        printf("Not Accepted\n");
        return 1;
    }
    printf("Accepted\n");

    if (strcmp(ack, "Received") != 0) {
        printf("Not Done\n");
        return 1;
    }
    printf("Done\n");

    // Close the socket
    close(sock);

    return 0;
}
