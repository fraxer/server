#ifndef __WEBSOCKETSPARSER__
#define __WEBSOCKETSPARSER__

#include <unistd.h>
#include "../request/websocketsrequest.h"
#include "websocketscommon.h"

typedef enum websockets_request_stage {
    FIRST_BYTE = 0,
    SECOND_BYTE,
    PAYLOAD_LEN,
    MASK_KEY,
    METHOD,
    LOCATION,
    DATA,
    CONTINUE_DATA
} websockets_request_stage_e;

typedef struct websocketsparser {
    websockets_request_stage_e stage;

    websockets_frame_t frame;

    int mask_index;

    size_t bytes_readed;
    size_t pos_start;
    size_t pos;
    size_t string_len;
    ssize_t payload_index;

    char* string;
    char* buffer;
    websocketsrequest_t* request;

    size_t* payload_length;
    char** payload;
} websocketsparser_t;

void websocketsparser_init(websocketsparser_t*, websocketsrequest_t*, char*);

void websocketsparser_free(websocketsparser_t*);

void websocketsparser_set_bytes_readed(websocketsparser_t*, size_t);

int websocketsparser_run(websocketsparser_t*);

void websocketsparser_append_query(websocketsrequest_t*, websockets_query_t*);

#endif
