#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "../config/config.h"
#include "../log/log.h"
#include "../route/route.h"
#include "../route/routeloader.h"
// #include "../database/database.h"
// #include "../openssl/openssl.h"
// #include "../mimetype/mimetype.h"
#include "../thread/thread.h"
#include "../jsmn/jsmn.h"
#include "moduleloader.h"
    #include <stdio.h>

int module_loader_init(int argc, char* argv[]) {

    if (config_init(argc, argv) == -1) return -1;

    if (module_loader_init_modules() == -1) return -1;

    if (config_free() == -1) return -1;

    return 0;
}

int module_loader_reload() {

    if (config_reload() == -1) return -1;

    if (module_loader_init_modules() == -1) return -1;

    if (config_free() == -1) return -1;

    return 0;
}

int module_loader_init_modules() {

    if (module_loader_init_module(&log_init, NULL) == -1) return -1;

    if (module_loader_init_module(&route_init, &module_loader_load_routes) == -1) return -1;

    // if (module_loader_init_module(&database::init, &module_loader_parse_database) == -1) return -1;

    // if (module_loader_init_module(&openssl::init, &module_loader_parse_openssl) == -1) return -1;

    // if (module_loader_init_module(&mimetype::init, &module_loader_parse_mimetype) == -1) return -1;

    // if (module_loader_init_module(&thread::init, NULL) == -1) return -1;

    // if (module_loader_init_module(&yookassa::init, &module_loader_parse_yookassa) == -1) return -1;

    // if (module_loader_init_module(&s3::init, &module_loader_parse_s3) == -1) return -1;

    // if (module_loader_init_module(&mail::init, &module_loader_parse_mail) == -1) return -1;

    // if (module_loader_init_module(&encryption::init, &module_loader_parse_encryption) == -1) return -1;

    if (module_loader_init_module(&thread_init, &module_loader_load_threads) == -1) return -1;

    return 0;
}

int module_loader_init_module(void* (*init)(), int (*parse)(void*)) {

    if (init == NULL) return -1;

    void* moduleStruct = init();

    if (parse == NULL) return 0;

    if (parse(moduleStruct) == -1) return -1;

    return 0;
}

int module_loader_load_routes(void* moduleStruct) {

    const jsmntok_t* token_root = config_get_section("routes");

    for (jsmntok_t* token_path = token_root->child; token_path; token_path = token_path->sibling) {

        // printf("%.*s\n", token_path->string_len, token_path->string);

        const char* route_path = jsmn_get_value(token_path);

        // printf("%s\n", route_path);

        route_t* route = route_create(route_path, NULL);

        if (route == NULL) goto failed;

        for (jsmntok_t* token_method = token_path->child->child; token_method; token_method = token_method->sibling) {
            const char* method = jsmn_get_value(token_method);

            // printf("%s\n", method);

            jsmntok_t* token_array = token_method->child;

            const char* lib_file = jsmn_get_array_value(token_array, 0);
            const char* lib_handler = jsmn_get_array_value(token_array, 1);

            // printf("%s %s: [%s, %s]\n", route_path, method, lib_file, lib_handler);

            if (routeloader_load_lib(lib_file) == -1) goto failed;

            void*(*function)(void*) = (void*(*)(void*))routeloader_get_handler(lib_file, lib_handler);

            if (function == NULL) goto failed;

            if (route_set_method_handler(route, method, function) == -1) goto failed;

            // (*function)("Hello");

            // --------------

            // void*(*funion)(void*) = route->method[ROUTE_GET]->run;

            // (*funion)("Hello");

            // --------------

            route->method[ROUTE_GET]("HELLO");
        }
    }

    int result = -1;

    result = 0;

    failed:

    return result;
}

int module_loader_parse_database(void* moduleStruct) {

    // database::database_t* structure = (database::database_t*)moduleStruct;

    const jsmntok_t* document = config_get_section("database");

    // const auto& object = document->GetObject();

    // if (module_loader_set_string(&object, &structure->host, "host") == -1) return -1;
    // if (module_loader_set_string(&object, &structure->dbname, "dbname") == -1) return -1;
    // if (module_loader_set_string(&object, &structure->user, "user") == -1) return -1;
    // if (module_loader_set_string(&object, &structure->password, "password") == -1) return -1;
    // if (module_loader_set_ushort(&object, &structure->port, "port") == -1) return -1;
    // if (module_loader_set_ushort(&object, &structure->connections_timeout, "connections_timeout") == -1) return -1;
    // if (module_loader_set_database_driver(&object, &structure->driver, "driver") == -1) return -1;
    // if (module_loader_set_database_connections(&object, &structure->connections, "connections_count") == -1) return -1;

    // printf("driver: %ld\n", sizeof(*structure->connections));

    return 0;
}

int module_loader_parse_openssl(void* moduleStruct) {
    return 0;
}

int module_loader_parse_mimetype(void* moduleStruct) {
    return 0;
}

int module_loader_load_threads(void* moduleStruct) {
    const jsmntok_t* token_root = config_get_section("main");

    unsigned int workers = 0;
    unsigned int handlers = 0;

    for (jsmntok_t* token = token_root->child; token; token = token->sibling) {
        const char* key = jsmn_get_value(token);
        const char* value = jsmn_get_value(token->child);

        if (strcmp(key, "workers") == 0) {
            workers = atoi(value);
        }
        else if (strcmp(key, "threads") == 0) {
            handlers = atoi(value);
        }
    }

    if (!workers || !handlers) {
        log_error("Thread error: Set the number of threads\n");
        return -1;
    }

    pthread_t thread_workers[workers];

    for(int i = 0; i < workers; i++) {

        int err_thread = pthread_create(&thread_workers[i], NULL, thread_worker, NULL);

        pthread_setname_np(thread_workers[i], "Server worker");

        if (err_thread != 0) {
            log_error("Thread error: Unable to create worker thread");
            return -1;
        }
        else {
            log_info("Worker thread created %d\n", i);
        }
    }

    pthread_t thread_handlers[handlers];

    for(int i = 0; i < handlers; i++) {

        int err_thread = pthread_create(&thread_handlers[i], NULL, thread_handler, NULL);

        pthread_setname_np(thread_handlers[i], "Server handler");

        if (err_thread != 0) {
            log_error("Thread error: Unable to create handler thread");
            return -1;
        }
        else {
            log_info("Handler thread created %d\n", i);
        }
    }

    for (int i = 0; i < workers; i++) {
        pthread_join(thread_workers[i], NULL);
    }

    for (int i = 0; i < handlers; i++) {
        pthread_join(thread_handlers[i], NULL);
    }

    return 0;
}

int module_loader_parse_yookassa(void* moduleStruct) {
    return 0;
}

int module_loader_parse_s3(void* moduleStruct) {
    return 0;
}

int module_loader_parse_mail(void* moduleStruct) {
    return 0;
}

int module_loader_parse_encryption(void* moduleStruct) {
    return 0;
}

int module_loader_set_string(const void* p_object, char** value, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsString()) return -1;

    // *value = (char*)malloc(object[key].GetStringLength() + 1);

    // if (*value == NULL) return -1;

    // strcpy(*value, (char*)object[key].GetString());

    return 0;
}

char* module_loader_get_string(const void* p_object, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsString()) return NULL;

    // return (char*)object[key].GetString();

    return NULL;
}

int module_loader_set_int(const void* p_object, int* value, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsInt64()) return -1;

    // *value = object[key].GetUint64();

    return 0;
}

int module_loader_set_ushort(const void* p_object, unsigned short* value, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsUint64()) return -1;

    // *value = object[key].GetUint64();

    // if (*value < 0 || *value > 65535) return -1;

    return 0;
}

unsigned short module_loader_get_ushort(const void* p_object, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsUint64()) return -1;

    // unsigned short value = object[key].GetUint64();

    // if (value < 0 || value > 65535) return -1;

    // return value;

    return 0;
}

int module_loader_set_database_driver(const void* p_object, void* p_value, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // if (!object.HasMember(key) || !object[key].IsString()) return -1;

    // char* driver = (char*)object[key].GetString();

    // database::driver_e& value = *(database::driver_e*)p_value;

    // if (strcmp(driver, "postgresql") == 0) {
    //     value = database::POSTGRESQL;
    // }
    // else if (strcmp(driver, "mysql") == 0) {
    //     value = database::MYSQL;
    // }

    // if (value == database::NONE) return -1;

    return 0;
}

int module_loader_set_database_connections(const void* p_object, void* p_value, const char* key) {

    // auto& object = *(rapidjson::GenericValue<rapidjson::UTF8<>>::ConstObject*)p_object;

    // int connectionsCount = module_loader_get_ushort(&object, "connections_count");

    // if (connectionsCount < 1) return -1;

    // database::connection_t* connections = (database::connection_t*)malloc(sizeof(database::connection_t) * connectionsCount);

    // for (int i = 0; i < connectionsCount; i += sizeof(database::connection_t)) {
    //     // database::initConnections(connections[i]);
    // }

    // p_value = connections;

    return 0;
}
