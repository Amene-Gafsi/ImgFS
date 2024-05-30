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
#include <pthread.h>

#include "http_prot.h"
#include "http_net.h"
#include "socket_layer.h"
#include "error.h"
#include "imgfs.h"
#include <string.h>

static int passive_socket = -1;
static EventCallback cb;

#define MK_OUR_ERR(X) \
    static int our_##X = X

MK_OUR_ERR(ERR_NONE);
MK_OUR_ERR(ERR_INVALID_ARGUMENT);
MK_OUR_ERR(ERR_OUT_OF_MEMORY);
MK_OUR_ERR(ERR_IO);

/*******************************************************************
 * Manages the HTTP connection with the client
 *******************************************************************/
static void *handle_connection(void *arg)
{ // TODO : free soket ?

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    if (arg == NULL)
    {
        return &our_ERR_INVALID_ARGUMENT;
    }

    int *socket_fd = (int *)arg;
    int buffer_size = MAX_HEADER_SIZE + NULL_TERMINATOR;

    char *buffer = malloc(buffer_size);
    if (buffer == NULL)
    {
        close(*socket_fd);
        free(socket_fd);
        return &our_ERR_OUT_OF_MEMORY;
    }
    int total_read = EMPTY, currently_read = EMPTY, extended = EMPTY, content_len = EMPTY;
    struct http_message message;

    memset(&message, 0, sizeof(struct http_message));
    while (1)
    {
        // Read data from socket
        currently_read = tcp_read(*socket_fd, buffer + total_read, buffer_size - total_read - 1);
        if (currently_read < 0)
        {
            free(buffer);
            close(*socket_fd);
            free(socket_fd);
            return &our_ERR_IO;
        }
        if (currently_read == 0)
        {
            break;
        }

        total_read += currently_read;
        buffer[total_read] = '\0';

        int parse_result = http_parse_message(buffer, total_read, &message, &content_len);
        if (parse_result < 0)
        {
            free(buffer);
            close(*socket_fd);
            free(socket_fd);
            return &parse_result;
        }
        // Check if messsage has not been received completely
        if (parse_result == 0)
        {
            if (content_len > 0)
            {
                if (!extended)
                {
                    buffer_size = MAX_HEADER_SIZE + content_len + NULL_TERMINATOR;
                    buffer = realloc(buffer, buffer_size);
                    if (!buffer)
                    {
                        free(buffer);
                        close(*socket_fd);
                        free(socket_fd);
                        return &our_ERR_OUT_OF_MEMORY;
                    }
                    extended = NON_EMPTY;
                }
                else
                {
                    continue;
                }
            }
            else // Should not happen since content_len should be > 0
            {
                free(buffer);
                close(*socket_fd);
                free(socket_fd);
                return &our_ERR_IO;
            }
        }
        if (parse_result > 0)
        {
            int callback_result = cb(&message, *socket_fd);
            if (callback_result != ERR_NONE)
            {
                free(buffer);
                close(*socket_fd);
                free(socket_fd);
                return &callback_result;
            }
            total_read = 0;
            extended = 0;
            memset(&message, 0, sizeof(struct http_message));
            buffer_size = MAX_HEADER_SIZE + NULL_TERMINATOR;
            buffer = realloc(buffer, buffer_size);
            if (!buffer)
            {
                free(buffer);
                close(*socket_fd);
                free(socket_fd);
                return &our_ERR_OUT_OF_MEMORY;
            }
        }
    }
    free(buffer);
    close(*socket_fd);
    free(socket_fd);
    return &our_ERR_NONE;
}

/*******************************************************************
 * Init connection
 *******************************************************************/
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
    if (passive_socket > 0)
    {
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
    if (passive_socket == -1)
    {
        return ERR_IO;
    }
    int *active_socket = malloc(sizeof(int));
    if (active_socket == NULL)
    {
        close(passive_socket);
        return ERR_IO;
    }
    *active_socket = tcp_accept(passive_socket);
    if (*active_socket == -1)
    {
        free(active_socket);
        return ERR_IO;
    }

    // Initialize pthread attributes
    pthread_attr_t attr;
    pthread_t thread;
    if (pthread_attr_init(&attr) != 0 ||
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
    {
        close(*active_socket);
        free(active_socket);
        return ERR_THREADING;
    }

    // Create a detached thread to handle the connection
    if (pthread_create(&thread, &attr, (void *(*)(void *))handle_connection, (void *)active_socket) != 0)
    {
        pthread_attr_destroy(&attr);
        close(*active_socket);
        free(active_socket);
        return ERR_THREADING;
    }

    // Destroy pthread attributes after use
    if (pthread_attr_destroy(&attr))
    {
        close(*active_socket);
        free(active_socket);
        return ERR_THREADING;
    }
    // close(*active_socket);
    // free(active_socket);

    return ERR_NONE;
}

/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char *filename)
{
    M_REQUIRE_NON_NULL(filename);

    // open file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "http_serve_file(): Failed to open file \"%s\"\n", filename);
        return http_reply(connection, "404 Not Found", "", "", 0);
    }

    // get its size
    fseek(file, 0, SEEK_END);
    const long pos = ftell(file);
    if (pos < 0)
    {
        fprintf(stderr, "http_serve_file(): Failed to tell file size of \"%s\"\n",
                filename);
        fclose(file);
        return ERR_IO;
    }
    rewind(file);
    const size_t file_size = (size_t)pos;

    // read file content
    char *const buffer = calloc(file_size + 1, 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "http_serve_file(): Failed to allocate memory to serve \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size)
    {
        fprintf(stderr, "http_serve_file(): Failed to read \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    // send the file
    const int ret = http_reply(connection, HTTP_OK,
                               "Content-Type: text/html; charset=utf-8" HTTP_LINE_DELIM,
                               buffer, file_size);

    // garbage collecting
    fclose(file);
    free(buffer);
    return ret;
}

// /*******************************************************************
//  * Create and send HTTP reply
//  */
int http_reply(int connection, const char *status, const char *headers, const char *body, size_t body_len)
{
    M_REQUIRE_NON_NULL(status);
    M_REQUIRE_NON_NULL(headers);

    size_t content_length_size = snprintf(NULL, 0, "%zu", body_len);
    if (content_length_size < 0)
    {
        return ERR_IO;
    }

    size_t header_size = strlen(HTTP_PROTOCOL_ID) + strlen(status) + strlen(HTTP_LINE_DELIM) +
                         strlen(headers) + strlen("Content-Length: ") + content_length_size +
                         strlen(HTTP_HDR_END_DELIM);

    size_t total_size = header_size + body_len + 1;

    char *buffer = calloc(1, total_size);
    if (buffer == NULL)
    {
        return ERR_OUT_OF_MEMORY;
    }

    int header_length = snprintf(buffer, total_size, "%s%s%s%sContent-Length: %zu%s",
                                 HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, headers, body_len, HTTP_HDR_END_DELIM);

    if (header_length < 0)
    {
        free(buffer);
        return ERR_IO;
    }

    if (body)
    {
        memcpy(buffer + header_length, body, body_len);
    }

    if (tcp_send(connection, buffer, header_length + body_len) == -1)
    {
        free(buffer);
        return ERR_IO;
    }
    free(buffer);
    return ERR_NONE;
}
