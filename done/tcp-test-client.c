#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define SERVER_IP "127.0.0.1"  // Change as necessary for a different server IP

int main(int argc, char *argv[]) { //TODO

    // Parse port and file path from arguments
    uint16_t port = atoi(argv[1]);
    const char *filename = argv[2];

    // Get file size
    struct stat file_stat;

    int file_size = file_stat.st_size;
    char size_str[20];
    snprintf(size_str, sizeof(size_str), "%d", file_size);

    // Create socket and connect to server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return 1;
    }

    // Send the file size to the server
    if (send(sock, size_str, strlen(size_str), 0) < 0) {
        perror("send failed");
        return 1;
    }

    // Receive the response from the server
    char buffer[1024] = {0};
    if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
        perror("recv failed");
        return 1;
    }
    printf("Server response: %s\n", buffer);

    // Close the socket
    close(sock);

    return 0;
}
