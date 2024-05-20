#include "http_prot.h"
#include <string.h>
#include "util.h"
#include "error.h"

/*************************************************************************
 * Checks whether the `message` URI starts with the provided `target_uri`
 ************************************************************************* */
int http_match_uri(const struct http_message *message, const char *target_uri)
{
    return !strncmp(message->uri.val, target_uri, strlen(target_uri));
}

/************************************************************************
 * Compare method with verb and return 1 if they are equal, 0 otherwise
 ************************************************************************ */
int http_match_verb(const struct http_string *method, const char *verb)
{
    return !strcmp(method->val, verb);
}

/************************************************************************
 * Writes the value of parameter `name` from URL in message to buffer out.
 ************************************************************************ */
int http_get_var(const struct http_string *url, const char *name, char *out, size_t out_len)
{
    M_REQUIRE_NON_NULL(url);
    M_REQUIRE_NON_NULL(name);

    const char parameter[strlen(name) + 2]; // TODO : = and \0
    strcpy(parameter, name);
    strcat(parameter, '=');

    const char *url_args = strchr(url, '?') + 1;
    if (!url_args)
        return 0;

    const char *param_start = strstr(url_args, parameter);
    if (!param_start)
        return 0;

    param_start += strlen(name) + 1;

    // Find the end of the parameter value
    const char *param_end = strchr(param_start, '&');
    if (!param_end)
        param_end = url + strlen(url);
    if (!param_end)
    {
        out = param_start;
        out_len = strlen(param_start);
        return out_len;
    }

    out_len = param_end - param_start;
    strncpy(out, param_start, out_len);

    return out_len;
}

static const char *get_next_token(const char *message, const char *delimiter, struct http_string *output)
{
    if (message == NULL || delimiter == NULL)
    {
        return NULL;
    }

    const char *end = strstr(message, delimiter);
    if (end == NULL)
    {
        if (output != NULL)
        {
            output->val = message;
            output->len = strlen(message);
        }
        return NULL;
    }

    if (output != NULL)
    {
        strncpy(output->val, message, end - message);
        output->len = end - message;
    }

    return end + strlen(delimiter);
}

// TODO : should we add this line ?   _Static_assert(strcmp(HTTP_HDR_END_DELIM, HTTP_LINE_DELIM HTTP_LINE_DELIM) == 0, "HTTP_HDR_END_DELIM is not twice HTTP_LINE_DELIM");
static const char *http_parse_headers(const char *header_start, struct http_message *output)
{
    const char *remaining = header_start;
    struct http_string pair;

    // put key in output
    remaining = get_next_token(remaining, HTTP_HDR_KV_DELIM, &pair);
    if (remaining == NULL)
        return NULL;
    output->headers[output->num_headers].key = pair;
    pair.val = NULL;
    remaining = get_next_token(remaining, HTTP_LINE_DELIM, &pair);
    if (remaining == NULL)
        return NULL;
    output->headers[output->num_headers].value = pair;

    return remaining;
}

/************************************************************************
 * Accepts a potentially partial TCP stream and parses an HTTP message.
 ************************************************************************ */
int http_parse_message(const char *stream, size_t bytes_received, struct http_message *out, int *content_len)
{
    // check that headers have been completly received
    if (!strstr(stream, HTTP_HDR_END_DELIM))
        return -1; // TODO : return negative value if error

    // Parse the first line
    const char *after_key = stream;
    struct http_string pair;
    // int content_length = 0; //TODO : we used content_length instead of content_len

    after_key = get_next_token(after_key, " ", &pair);
    out->method = pair;
    pair.val = NULL;
    after_key = get_next_token(after_key, " ", &pair);
    out->uri = pair;
    pair.val = NULL;
    after_key = get_next_token(after_key, HTTP_LINE_DELIM, NULL);

    while (after_key != NULL && strncmp(after_key, HTTP_LINE_DELIM, strlen(HTTP_LINE_DELIM)))
    {
        after_key = http_parse_headers(after_key, out);
    }

    // get the content length from the headers
    for (size_t i = 0; i < out->num_headers; i++)
    {
        if (!strncmp(out->headers[i].key.val, "Content-Length", out->headers[i].key.len))
        {
            *content_len = atoi(out->headers[i].value.val);
            break;
        }
    }
    /*if (*content_len <= 0) //TODO : we should first check that the header was fully received?
        return 1;
    if (after_key == NULL)
        return 0;

    out->body.val = after_key; //TODO: should we check that we have exactly content_length elements?
    out->body.len = *content_len;

    return 1;*/
    //TODO : make a helper function ?
    // Check if message received completly
    if (after_key == NULL)
        return 0;
    // Received a complete header message with no body
    if (*content_len <= 0 && !strncmp(after_key, HTTP_LINE_DELIM, strlen(HTTP_LINE_DELIM)))
        return 1;
    // Received a complete header message with body
    if (*content_len > 0)
    {
        out->body.val = after_key;
        out->body.len = *content_len;
        return 1;
    }

    return -1;
}
