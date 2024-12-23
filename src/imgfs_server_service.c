/*
 * @file imgfs_server_services.c
 * @brief ImgFS server part, bridge between HTTP server layer and ImgFS library
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // uint16_t
#include <vips/vips.h>

#include "error.h"
#include "util.h" // atouint16
#include "imgfs.h"
#include "http_net.h"
#include "imgfs_server_service.h"

// Main in-memory structure for imgFS
static struct imgfs_file fs_file;
static uint16_t server_port;

#define URI_ROOT "/imgfs"
static pthread_mutex_t imgfs_mutex;

/********************************************************************/ /**
                                                                        * Startup function. Create imgFS file and load in-memory structure.
                                                                        * Pass the imgFS file name as argv[1] and optionnaly port number as argv[2]
                                                                        ********************************************************************** */
int server_startup(int argc, char **argv)
{
    if (argc < 2)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    if (VIPS_INIT(argv[0]))
    {
        return ERR_IMGLIB;
    }

    char *filename = argv[1];
    int temp_port = 0;

    // server_port takes the value of the port number passed as argument if it's valid, otherwise it takes the default value
    if (argc > 2)
    {
        temp_port = atoi(argv[2]);
        if (temp_port == 0)
        {
            vips_shutdown();
            return ERR_INVALID_ARGUMENT;
        }

        if (temp_port > 0)
        {
            server_port = (uint16_t)temp_port;
        }
    }
    if (server_port <= 0)
    {
        server_port = DEFAULT_LISTENING_PORT;
    }

    if (pthread_mutex_init(&imgfs_mutex, NULL))
    {
        vips_shutdown();
        return ERR_THREADING;
    }

    int ret = ERR_NONE;
    ret = do_open(filename, "rb+", &fs_file);
    if (ret != ERR_NONE)
    {
        vips_shutdown();
        return ret;
    }
    print_header(&fs_file.header);
    EventCallback cb = handle_http_message;

    if (http_init(server_port, cb) == -1)
    {
        vips_shutdown();
        return ERR_IO;
    }
    return ERR_NONE;
}

/********************************************************************/ /**
                                                                        * Shutdown function. Free the structures and close the file.
                                                                        ********************************************************************** */
void server_shutdown(void)
{
    fprintf(stderr, "Shutting down...\n");
    http_close();
    do_close(&fs_file);
    vips_shutdown();
    pthread_mutex_destroy(&imgfs_mutex);
}

/**********************************************************************
 * Sends error message.
 ********************************************************************** */
static int reply_error_msg(int connection, int error)
{
#define ERR_MSG_SIZE 256
    char err_msg[ERR_MSG_SIZE]; // enough for any reasonable err_msg
    if (snprintf(err_msg, ERR_MSG_SIZE, "Error: %s\n", ERR_MSG(error)) < 0)
    {
        fprintf(stderr, "reply_error_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "500 Internal Server Error", "",
                      err_msg, strlen(err_msg));
}

/**********************************************************************
 * Sends 302 OK message.
 ********************************************************************** */
static int reply_302_msg(int connection)
{
    char location[ERR_MSG_SIZE];
    if (snprintf(location, ERR_MSG_SIZE, "Location: http://localhost:%d/" BASE_FILE HTTP_LINE_DELIM,
                 server_port) < 0)
    {
        fprintf(stderr, "reply_302_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "302 Found", location, "", 0);
}

/**********************************************************************
 * Reply with the list of images in JSON format.
 ********************************************************************** */
int handle_list_call(int connection)
{
    char *json = NULL;
    int ret = ERR_NONE;
    pthread_mutex_lock(&imgfs_mutex);
    ret = do_list(&fs_file, JSON, &json);
    pthread_mutex_unlock(&imgfs_mutex);
    if (ret != ERR_NONE)
    {
        free(json);
        json = NULL;
        return reply_error_msg(connection, ret);
    }
    ret = http_reply(connection, HTTP_OK, "Content-Type: application/json" HTTP_LINE_DELIM, json, strlen(json));
    free(json);
    json = NULL;
    if (ret != ERR_NONE)
    {
        return reply_error_msg(connection, ret);
    }
    return ret;
}

/**********************************************************************
 * Reply with the image requested.
 ********************************************************************** */
int handle_read_call(struct http_message *msg, int connection)
{
    char out_res[MAX_HEADER_SIZE + NULL_TERMINATOR];
    char out_img_id[MAX_IMG_ID + NULL_TERMINATOR];

    if (http_get_var(&msg->uri, "res", out_res, MAX_HEADER_SIZE) == 0)
    {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS);
    }
    if (http_get_var(&msg->uri, "img_id", out_img_id, MAX_IMG_ID) == 0)
    {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS);
    }
    int res = resolution_atoi(out_res);
    if (res == -1)
    {
        return reply_error_msg(connection, ERR_RESOLUTIONS);
    }
    uint32_t image_size = 0;
    char *image_buffer = NULL;
    int ret = ERR_NONE;
    pthread_mutex_lock(&imgfs_mutex);
    ret = do_read(out_img_id, res, &image_buffer, &image_size, &fs_file);
    pthread_mutex_unlock(&imgfs_mutex);
    if (ret != ERR_NONE)
    {
        return reply_error_msg(connection, ret);
    }
    ret = http_reply(connection, HTTP_OK, "Content-Type: image/jpeg" HTTP_LINE_DELIM, image_buffer, image_size);
    free(image_buffer);
    image_buffer = NULL;
    if (ret != ERR_NONE)
    {
        return reply_error_msg(connection, ret);
    }
    return ret;
}

/**********************************************************************
 * Delete the image requested and reply with 302 OK message.
 ********************************************************************** */
int handle_delete_call(struct http_message *msg, int connection)
{
    char out_img_id[MAX_IMG_ID + NULL_TERMINATOR];
    if (http_get_var(&msg->uri, "img_id", out_img_id, MAX_IMG_ID) == 0)
    {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS);
    }
    int ret = ERR_NONE;
    pthread_mutex_lock(&imgfs_mutex);
    ret = do_delete(out_img_id, &fs_file);
    pthread_mutex_unlock(&imgfs_mutex);
    if (ret != ERR_NONE)
    {
        return reply_error_msg(connection, ret);
    }
    return reply_302_msg(connection);
}

/**********************************************************************
 * Insert the image requested and reply with 302 OK message.
 ********************************************************************** */
int handle_insert_call(struct http_message *msg, int connection)
{
    char out_img_id[MAX_IMG_ID];
    if (http_get_var(&msg->uri, "name", out_img_id, MAX_IMG_ID) == 0)
    {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS);
    }
    char *image_data = (char *)malloc(msg->body.len);
    if (image_data == NULL)
    {
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY);
    }
    memcpy(image_data, msg->body.val, msg->body.len);

    pthread_mutex_lock(&imgfs_mutex);
    int ret = do_insert(image_data, msg->body.len, out_img_id, &fs_file);
    free(image_data);
    image_data = NULL;
    pthread_mutex_unlock(&imgfs_mutex);

    if (ret != ERR_NONE)
    {
        return reply_error_msg(connection, ret);
    }
    return reply_302_msg(connection);
}

/**********************************************************************
 * Simple handling of http message. TO BE UPDATED WEEK 13
 ********************************************************************** */
int handle_http_message(struct http_message *msg, int connection)
{
    M_REQUIRE_NON_NULL(msg);
    if (http_match_verb(&msg->uri, "/") || http_match_uri(msg, "/index.html"))
    {
        return http_serve_file(connection, BASE_FILE);
    }

    debug_printf("handle_http_message() on connection %d. URI: %.*s\n",
                 connection,
                 (int)msg->uri.len, msg->uri.val);
    if (http_match_uri(msg, URI_ROOT "/list"))
        return handle_list_call(connection);
    if (http_match_uri(msg, URI_ROOT "/insert") && http_match_verb(&msg->method, "POST"))
        return handle_insert_call(msg, connection);
    if (http_match_uri(msg, URI_ROOT "/read"))
        return handle_read_call(msg, connection);
    if (http_match_uri(msg, URI_ROOT "/delete"))
        return handle_delete_call(msg, connection);

    return reply_error_msg(connection, ERR_INVALID_COMMAND);
}
