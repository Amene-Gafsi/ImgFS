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
#include "imgfs.h"

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
static void *handle_connection(void *arg)
{
    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;   
    int *socket_fd = (int *)arg;
    char buffer[MAX_HEADER_SIZE + 1]; 
    
    int total_read = 0, currently_read = 0;

    while (total_read < MAX_HEADER_SIZE)
    {
        currently_read = tcp_read(*socket_fd, buffer + total_read, MAX_HEADER_SIZE - total_read);
        if (currently_read <= 0) {
            close(*socket_fd);
            return &our_ERR_IO;
        }
        total_read += currently_read;
        buffer[total_read] = '\0';

        if (strstr(buffer, HTTP_HDR_END_DELIM)) {
            break;
        }
    }

    if (!strstr(buffer, HTTP_HDR_END_DELIM)) {
        close(*socket_fd);
        return &our_ERR_IO;
    }

    const char *status;
    if (strstr(buffer, "test: ok")) {
        status = HTTP_OK;
    } else {
        status = HTTP_BAD_REQUEST;
    }

    //int ret = http_reply(1234, HTTP_OK, "Content-Type: text/html; charset=utf-8" HTTP_LINE_DELIM, buffer, 6789);
    int ret = http_reply(*socket_fd, status, buffer, "", 0); //TODO how to get the body ?
    if (ret != ERR_NONE) {
        close(*socket_fd);
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
    if(passive_socket == -1){
        perror("Error creating socket");
        return ERR_IO;
    } 
    int new_socket = tcp_accept(passive_socket);
    if (new_socket == -1)
    {
        close(passive_socket);
        perror("accept");
        return ERR_IO;
    }
    handle_connection(&new_socket);
    close(new_socket);
    return ERR_NONE;
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
    //Error binding socket: Address already in use  //TODO when I rereun fast
    //http_init() failed
    //I/O Error
    const char* new_header = calloc(strlen(headers) - 1, sizeof(char));
    strncpy(new_header, headers, strlen(headers) - 2);

    size_t header_size = strlen(HTTP_PROTOCOL_ID) + strlen(status) + strlen(HTTP_LINE_DELIM) + strlen(new_header) + strlen("Content-Length: ") + snprintf(NULL, 0, "%zu", body_len) + strlen(HTTP_HDR_END_DELIM);
    size_t buffer_size = header_size + body_len;

    char *buffer = calloc(1, buffer_size +1);
    if (buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    if(snprintf(buffer, header_size +1, "%s%s%s%sContent-Length: %zu%s", HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, new_header, body_len, HTTP_HDR_END_DELIM) <= 0){
        free(buffer);
        return ERR_IO;
    }

    if (body && body_len > 0) {
        memcpy(buffer + header_size, body, body_len);
    }

    if(tcp_send(connection, buffer, strlen(buffer)) == -1){  //TODO send -1 without null
        free(buffer);
        return ERR_IO;
    }
    free(buffer);
    return ERR_NONE;
}
