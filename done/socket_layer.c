#include "socket_layer.h"
#include "imgfs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

/********************************************************************
 * Initialise the TCP server
 *******************************************************************/
int tcp_server_init(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    #ifdef SOCKET_REUSE
    int option = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    #endif

    if (fd == -1)
    {
        perror("Error creating socket");
        return ERR_IO;
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server)); // Initialize the structure with zeros

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections on any interface

    if (bind(fd, &server, sizeof(server)))
    {
        perror("Error binding socket");
        close(fd);
        return ERR_IO;
    }

    if (listen(fd, SOMAXCONN))
    {
        perror("Error listening for connections");
        close(fd);
        return ERR_IO;
    }

    return fd;
}

/********************************************************************
 * Accept a TCP request
 *******************************************************************/
int tcp_accept(int passive_socket)
{
    return accept(passive_socket, NULL, NULL);
}

/********************************************************************
 * Read a TCP request
 *******************************************************************/
ssize_t tcp_read(int active_socket, char *buf, size_t buflen)
{
    if (active_socket == -1 || buf == NULL || buflen <= 0)
    {
        return ERR_INVALID_ARGUMENT;
    }
    return recv(active_socket, buf, buflen, EMPTY);
}

/********************************************************************
 * Send a TCP request
 *******************************************************************/
ssize_t tcp_send(int active_socket, const char *response, size_t response_len)
{
    if (active_socket == -1 || response == NULL || response_len <= 0)
    {
        return ERR_INVALID_ARGUMENT;
    }
    return send(active_socket, response, response_len, EMPTY);
}
