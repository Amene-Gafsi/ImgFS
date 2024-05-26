#include "http_prot.h"
#include <string.h>
#include "util.h"
#include "error.h"

/*************************************************************************
 * Checks whether the `message` URI starts with the provided `target_uri`
 ************************************************************************* */
int http_match_uri(const struct http_message *message, const char *target_uri)
{
    M_REQUIRE_NON_NULL(message);
    M_REQUIRE_NON_NULL(target_uri);
    M_REQUIRE_NON_NULL(message->uri.val);
    return !strncmp(message->uri.val, target_uri, strlen(target_uri));
}

/************************************************************************
 * Compare method with verb and return 1 if they are equal, 0 otherwise
 ************************************************************************ */
int http_match_verb(const struct http_string *method, const char *verb)
{
    M_REQUIRE_NON_NULL(method);
    M_REQUIRE_NON_NULL(verb);
    M_REQUIRE_NON_NULL(method->val);

    size_t verb_length = strlen(verb);
    if (method->len != verb_length) {
        return 0;
    }

    return !strncmp(method->val, verb, verb_length);
}

/************************************************************************
 * Writes the value of parameter `name` from URL in message to buffer out.
 ************************************************************************ */
int http_get_var(const struct http_string *url, const char *name, char *out, size_t out_len)
{
    M_REQUIRE_NON_NULL(url);
    M_REQUIRE_NON_NULL(url->val);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(out);

    char parameter[strlen(name) + 2];
    snprintf(parameter, sizeof(parameter), "%s=", name);

    // look for ? in the url
    const char *url_args = strchr(url->val, '?');
    if (!url_args) {
        return 0;
    }
    url_args += 1;

    // look for the parameter in the url
    const char *param_start = strstr(url_args, parameter);
    if (!param_start) {
        return 0;
    }
    param_start += strlen(parameter);

    // look for the end of the parameter
    const char *param_end = strchr(param_start, '&');
    if (!param_end) {
        param_end = url->val + url->len;
    }

    size_t value_len = param_end - param_start;
    if (value_len >= out_len) {
        return ERR_RUNTIME;
    }

    strncpy(out, param_start, value_len);
    out[value_len] = '\0';

    return value_len;
}

/************************************************************************************************************
 * Writes the message, until the delimiter, in the output struct and returns the next char after the delimiter
 ************************************************************************************************************ */
static const char *get_next_token(const char *message, const char *delimiter, struct http_string *output)
{
    M_REQUIRE_NON_NULL(message);
    M_REQUIRE_NON_NULL(delimiter);

    const char *end = strstr(message, delimiter);
    if (!end) {
        if (output) {
            output->val = message;
            output->len = strlen(message);
        }
        return message + strlen(message);
    }

    if (output) {
        output->val = message;   //TODO need to cut the delimiter or len suffit
        output->len = end - message;
    }

    return end + strlen(delimiter);
}

// TODO : should we add this line ?   _Static_assert(strcmp(HTTP_HDR_END_DELIM, HTTP_LINE_DELIM HTTP_LINE_DELIM) == 0, "HTTP_HDR_END_DELIM is not twice HTTP_LINE_DELIM");
/************************************************************************************************************
 * Parses the headers of the message and returns the next char after the last header
 ************************************************************************************************************ */
static const char *http_parse_headers(const char *header_start, struct http_message *output)
{
    M_REQUIRE_NON_NULL(header_start);
    M_REQUIRE_NON_NULL(output);

    const char *remaining = header_start;
    struct http_string pair;

    // put key in output
    remaining = get_next_token(remaining, HTTP_HDR_KV_DELIM, &pair);
    if (remaining == NULL)
        return NULL;
    output->headers[output->num_headers].key = pair;
    pair.val = NULL;
    remaining = get_next_token(remaining, HTTP_LINE_DELIM, &pair);
    output->headers[output->num_headers].value = pair;

    return remaining;
}

/************************************************************************
 * Accepts a potentially partial TCP stream and parses an HTTP message.
 ************************************************************************ */
int http_parse_message(const char *stream, size_t bytes_received, struct http_message *out, int *content_len)
{
    M_REQUIRE_NON_NULL(stream);
    M_REQUIRE_NON_NULL(out);
    M_REQUIRE_NON_NULL(content_len);

    // check that headers have been completly received //TODO negative return ?
    if (!strstr(stream, HTTP_HDR_END_DELIM))
        return 0;
    out->num_headers = 0;   //TODO check if it is necessary
    // Parse the first line
    const char *after_key = stream;
    struct http_string pair;
    *content_len = 0;

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
        out->num_headers++;
    }

    after_key = get_next_token(after_key, HTTP_LINE_DELIM, NULL);  // skip the empty line  //TODO what if after_key null

    // get the content length from the headers
    for (size_t i = 0; i < out->num_headers; i++)
    {
        if (!strncmp(out->headers[i].key.val, "Content-Length", out->headers[i].key.len))
        {
            *content_len= atoi(out->headers[i].value.val);
            break;
        }
    }

    if (*content_len <= 0)   //TODO check afterkey null ?
        return 1;
    // if (after_key == NULL || strlen(after_key) < *content_len){ //TODO less or equal
    //     return 0;}
    if (after_key == NULL || bytes_received - (after_key - stream) < *content_len){ //TODO less or equal
         return 0;}

    out->body.val = after_key;
    out->body.len = *content_len;
    return 1;
}
