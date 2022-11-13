#ifndef __SERVER__
#define __SERVER__

#include <arpa/inet.h>
#include "../route/route.h"
#include "../domain/domain.h"

typedef struct index {
    char* value;
    int length;
} index_t;

typedef struct redirect {
    char* template;
    char* target;
    struct redirect* next;
} redirect_t;

typedef struct server {
    int is_deprecated;
    unsigned short int port;
    domain_t* domain;
    in_addr_t ip;
    char* root;
    index_t* index;
    redirect_t* redirect;
    route_t* route;
    // database_t* database;
    struct server* next;
} server_t;

typedef struct server_chain {
    server_t* server;
    struct server_chain* next;
} server_chain_t;

void* server_init();

server_t* server_create();

server_t* server_alloc();

index_t* server_create_index(const char*);

void server_free(server_t*);

server_chain_t* server_chain_alloc();

server_chain_t* server_chain_create(server_t* server);

server_chain_t* server_chain_last();

int server_chain_append(server_t*);

void server_chain_destroy_first();

#endif
