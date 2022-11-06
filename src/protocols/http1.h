#ifndef __HTTP1__
#define __HTTP1__

#include "../connection/connection.h"

#define HTTP1_BUFFER 16384

int http1_init();

void http1_read(connection_t*);

void http1_write(connection_t*);

ssize_t http1_read_internal(connection_t*, char*);

ssize_t http1_write_internal(connection_t*, const char*, size_t);

#endif
