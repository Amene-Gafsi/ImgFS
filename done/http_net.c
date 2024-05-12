/* 
 * @file http_net.c
 * @brief HTTP server layer for CS-202 project
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include "http_prot.h"
#include "http_net.h"
#include "socket_layer.h"
#include "error.h"

#define HTTP_PORT 8080 //TODO : Which one to pick?

static int passive_socket = -1;
static EventCallback cb;

#define MK_OUR_ERR(X) \
static int our_ ## X = X

MK_OUR_ERR(ERR_NONE);
MK_OUR_ERR(ERR_INVALID_ARGUMENT);
MK_OUR_ERR(ERR_OUT_OF_MEMORY);
MK_OUR_ERR(ERR_IO);

/*******************************************************************
 * Handle connection
 */
static void *handle_connection(void *arg) //TODO : UNDERSTAND
{
    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;
        
    int *socket_fd = (int *)arg;
    char buffer[MAX_HEADER_SIZE + 1]; 
    int total_read = 0, currently_read = 0;

    while (total_read < MAX_HEADER_SIZE)
    {
        currently_read = tcp_read(*socket_fd, buffer + total_read, MAX_HEADER_SIZE - total_read);
        if (currently_read <= 0) {
            return &our_ERR_IO;
        }
        total_read += currently_read;
        buffer[total_read] = '\0';

        if (strstr(buffer, HTTP_HDR_END_DELIM)) {
            break;
        }
    }

    if (!strstr(buffer, HTTP_HDR_END_DELIM)) {
        return &our_ERR_IO;
    }

    const char *status;
    if (strstr(buffer, "test: ok")) {
        status = HTTP_OK;
    } else {
        status = HTTP_BAD_REQUEST;
    }

    int ret = http_reply(*socket_fd, status, "", "", 0);  //TODO : don't know what arguments to give
    if (ret != ERR_NONE) {
        return &our_ERR_IO;
    }

    close(*socket_fd);

    return &our_ERR_NONE;
}


/*******************************************************************
 * Init connection
 */
int http_init(uint16_t port, EventCallback callback)
{
    passive_socket = tcp_server_init(port);
    cb = callback;
    return passive_socket;
}

/*******************************************************************
 * Close connection
 */
void http_close(void)
{
    if (passive_socket > 0) {
        if (close(passive_socket) == -1)
            perror("close() in http_close()");
        else
            passive_socket = -1;
    }
}

/*******************************************************************
 * Receive content
 */
int http_receive(void)
{
    int socket_fd = http_init(HTTP_PORT, cb);
    if(socket_fd == -1){
        perror("Error creating socket");
        return ERR_IO;
    }
    while (1)
    {
        int new_socket = tcp_accept(socket_fd);
        if (new_socket == -1)
        {
            perror("accept");
            continue;
        }
        handle_connection(&new_socket);
    }
    close(socket_fd);
}

/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char* filename)
{
    int ret = ERR_NONE;
    return ret;
}

/*******************************************************************
 * Create and send HTTP reply
 */
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len)
{
    return ERR_NONE;
}
