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
#include <string.h>


static int passive_socket = -1;
static EventCallback cb; //TODO : should I change this?

#define MK_OUR_ERR(X) \
static int our_ ## X = X

MK_OUR_ERR(ERR_NONE);
MK_OUR_ERR(ERR_INVALID_ARGUMENT);
MK_OUR_ERR(ERR_OUT_OF_MEMORY);
MK_OUR_ERR(ERR_IO);

/*******************************************************************
 * Handle connection
 */
/*static void *handle_connection(void *arg)
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
}*/

/*static void *handle_connection(void *arg) {
    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;
    int *socket_fd = (int *)arg;
    char *buffer = malloc(MAX_HEADER_SIZE + 1);
    if (!buffer) {
        close(*socket_fd);
        return &our_ERR_IO;
    }

    int total_read = 0, currently_read = 0;
    int extended = 0;
    int content_len = 0;

    struct http_message message;
    memset(&message, 0, sizeof(struct http_message));

    while (1) {
        // Read data from socket
        currently_read = tcp_read(*socket_fd, buffer + total_read, MAX_HEADER_SIZE - total_read);
        if (currently_read < 0) {
            free(buffer);
            close(*socket_fd);
            return &our_ERR_IO;
        } 

        if (currently_read == 0) {
            // Client closed the connection
            break;
        }

        total_read += currently_read;
        buffer[total_read] = '\0';

        // Parse the message
        int parse_result = http_parse_message(buffer, total_read, &message, &content_len);
        
        // Check for error
        if (parse_result < 0) {
            free(buffer);
            close(*socket_fd);
            return &parse_result;
        }
       
       // Check if messsage has not been received completely
        if (parse_result == 0) {
            if (!extended && content_len > 0 && total_read < MAX_HEADER_SIZE + content_len) {
                char *extended_buffer = realloc(buffer, MAX_HEADER_SIZE + content_len + 1);
                if (!extended_buffer) {
                    free(buffer);
                    close(*socket_fd);
                    return &our_ERR_IO;
                }
                buffer = extended_buffer;
                extended = 1;
                continue;
            } else {
                free(buffer);
                close(*socket_fd);
                return &our_ERR_IO;
            }
        }
        // Check if messsage has been received completely
        if (parse_result > 0) {
            int callback_result = cb(&message, *socket_fd);
            if (callback_result != ERR_NONE) {
                free(buffer);
                close(*socket_fd);
                return &callback_result;
            }
            
            // Check for "test: ok" header to determine the status //TODO: Should do this?
            // int found = NOT_FOUND;
            // for (size_t i = 0; i < message.num_headers; ++i) {
            //     if (strncmp(message.headers[i].key.val, "test", message.headers[i].key.len) == 0 &&
            //         strncmp(message.headers[i].value.val, "ok", message.headers[i].value.len) == 0) {
            //         found = FOUND;
            //         break;
            //     }
            // }
            // const char *status;
            // if (found == FOUND) {
            //     status = HTTP_OK;
            // } else {
            //     status = HTTP_BAD_REQUEST;
            // }
            // int ret = http_reply(*socket_fd, HTTP_OK, buffer, "", 0); 
            // if (ret != ERR_NONE) {
            //     free(buffer);
            //     close(*socket_fd);
            //     return &our_ERR_IO;
            // }
            // Reset variables for a new round of tcp_read
            total_read = 0;
            extended = 0;
            memset(&message, 0, sizeof(struct http_message));
        }
    }

    free(buffer);
    close(*socket_fd);
    return &our_ERR_NONE;
} */

static void *handle_connection(void *arg) {
    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;
    int *socket_fd = (int *)arg;
    int buffer_size = MAX_HEADER_SIZE + 1;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        close(*socket_fd);
        return &our_ERR_IO;
    }
    int total_read = 0, currently_read = 0 , extended = 0, content_len = 0;
    struct http_message message;
    memset(&message, 0, sizeof(struct http_message));

    while (1) {
        // Read data from socket
        currently_read = tcp_read(*socket_fd, buffer + total_read, buffer_size - total_read-1);
        if (currently_read < 0) {
            free(buffer);
            close(*socket_fd);
            return &our_ERR_IO;
        } 
        total_read += currently_read;
        buffer[total_read] = '\0';

        int parse_result = http_parse_message(buffer, total_read, &message, &content_len);
        if (parse_result < 0) {
            free(buffer);
            close(*socket_fd);
            return &parse_result;
        }
       
       // Check if messsage has not been received completely
        if (parse_result == 0) {
             if (content_len > 0) {
                if (!extended) {
                    buffer_size = MAX_HEADER_SIZE + content_len + 1;
                    buffer = realloc(buffer, buffer_size);
                    if (!buffer) {
                        free(buffer);
                        close(*socket_fd);
                        return &our_ERR_IO;
                    }
                    extended = 1;
                }
                continue;
            } else {
                free(buffer);
                close(*socket_fd);
                return &our_ERR_IO;
            }
        }

        // Check if messsage has been received completely
        if (parse_result > 0) {
            buffer_size = MAX_HEADER_SIZE + 1;
            int callback_result = cb(&message, *socket_fd);
            if (callback_result != ERR_NONE) {
                free(buffer);
                close(*socket_fd);
                return &callback_result;
            }
            // Reset variables for a new round of tcp_read
            total_read = 0;
            extended = 0;
            memset(&message, 0, sizeof(struct http_message));
            buffer_size = MAX_HEADER_SIZE + 1;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                free(buffer);
                close(*socket_fd);
                return &our_ERR_IO;}
        }
    }
    free(buffer);
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
    M_REQUIRE_NON_NULL(filename);

    // open file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to open file \"%s\"\n", filename);
        return http_reply(connection, "404 Not Found", "", "", 0);
    }

    // get its size
    fseek(file, 0, SEEK_END);
    const long pos = ftell(file);
    if (pos < 0) {
        fprintf(stderr, "http_serve_file(): Failed to tell file size of \"%s\"\n",
                filename);
        fclose(file);
        return ERR_IO;
    }
    rewind(file);
    const size_t file_size = (size_t) pos;

    // read file content
    char* const buffer = calloc(file_size + 1, 1);
    if (buffer == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to allocate memory to serve \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "http_serve_file(): Failed to read \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    // send the file
    const int  ret = http_reply(connection, HTTP_OK,
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
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len) {
    M_REQUIRE_NON_NULL(status);
    M_REQUIRE_NON_NULL(headers);

    size_t content_length_size = snprintf(NULL, 0, "%zu", body_len);

    size_t header_size = strlen(HTTP_PROTOCOL_ID) + strlen(status) + strlen(HTTP_LINE_DELIM) +
                         strlen(headers) + strlen("Content-Length: ") + content_length_size +
                         strlen(HTTP_HDR_END_DELIM);

    size_t total_size = header_size + body_len + 1;

    char *buffer = calloc(1, total_size);
    if (buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    
    int header_length = snprintf(buffer, total_size, "%s%s%s%sContent-Length: %zu%s",
                                 HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, headers, body_len, HTTP_HDR_END_DELIM);

    if (header_length < 0) {
        free(buffer);
        return ERR_IO;
    }

    if (body) {
        memcpy(buffer + header_length, body, body_len);
    }

    if (tcp_send(connection, buffer, header_length + body_len) == -1) {
        free(buffer);
        return ERR_IO;
    }
    free(buffer);
    return ERR_NONE;
}

