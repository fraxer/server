#include <stddef.h>
#include <stdlib.h>
#include "http1request.h"
    #include <stdio.h>

void http1request_reset(http1request_t*);

http1request_t* http1request_alloc() {
    return (http1request_t*)malloc(sizeof(http1request_t));
}

void http1request_header_free(http1_header_t* header) {
    while (header) {
        http1_header_t* next = header->next;

        http1_header_free(header);

        header = next;
    }
}

void http1request_query_free(http1_query_t* query) {
    while (query != NULL) {
        http1_query_t* next = query->next;

        http1_query_free(query);

        query = next;
    }
}

void http1request_free(void* arg) {
    http1request_t* request = (http1request_t*)arg;

    http1request_reset(request);

    free(request);

    request = NULL;
}

http1request_t* http1request_create(connection_t* connection) {
    http1request_t* request = http1request_alloc();

    if (request == NULL) return NULL;

    request->method = ROUTE_NONE;
    request->version = HTTP1_VER_NONE;
    request->uri_length = 0;
    request->path_length = 0;
    request->ext_length = 0;
    request->uri = NULL;
    request->path = NULL;
    request->ext = NULL;
    request->payload = NULL;
    request->query = NULL;
    request->last_query = NULL;
    request->header = NULL;
    request->last_header = NULL;
    request->connection = connection;
    request->base.reset = (void(*)(void*))http1request_reset;
    request->base.free = (void(*)(void*))http1request_free;

    return request;
}

void http1request_reset(http1request_t* request) {
    request->method = ROUTE_NONE;
    request->version = HTTP1_VER_NONE;
    request->uri_length = 0;
    request->path_length = 0;
    request->ext_length = 0;

    if (request->uri) free((void*)request->uri);
    request->uri = NULL;

    if (request->path) free((void*)request->path);
    request->path = NULL;

    if (request->ext) free((void*)request->ext);
    request->ext = NULL;

    if (request->payload) free(request->payload);
    request->payload = NULL;

    http1request_query_free(request->query);
    request->query = NULL;
    request->last_query = NULL;

    http1request_header_free(request->header);
    request->header = NULL;
    request->last_header = NULL;
}