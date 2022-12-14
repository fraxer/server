#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../helpers/helpers.h"
#include "../log/log.h"
#include "../jsmn/jsmn.h"
#include "config.h"

char* CONFIG_PATH = NULL;

static jsmn_parser_t parser;

char* config_get_path(int argc, char* argv[]) {
    int opt = 0;

    char* path = NULL;

    int c_found = 0;

    opterr = 0;
    optopt = 0;
    optind = 0;
    optarg = NULL;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                path = optarg;
                c_found = 1;
                break;
            default:
                return NULL;
        }
    }

    if (!c_found) {
        return NULL;
    }

    if (argc < 3) {
        return NULL;
    }

    return path;
}

int config_save_path(const char* path) {
    if (path == NULL) {
        return -1;
    }

    if (strlen(path) == 0) {
        return -1;
    }

    if (CONFIG_PATH != NULL) {
        free(CONFIG_PATH);
    }

    CONFIG_PATH = (char*)malloc(sizeof(char) * strlen(path) + 1);

    if (CONFIG_PATH == NULL) {
        return -1;
    }

    strcpy(CONFIG_PATH, path);

    return 0;
}

int config_load(const char* path) {
    if (path == NULL) {
        return -1;
    }

    int fd = open(path, O_RDONLY);

    if (fd == -1) return -1;

    char* data = helpers_read_file(fd);

    close(fd);

    if (data == NULL) return -1;

    int result = -1;

    if (config_parse_data(data) == -1) goto failed;

    result = 0;

    failed:

    free(data);

    return result;
}

int config_reload() {
    if (CONFIG_PATH == NULL) {
        return -1;
    }

    if (strlen(CONFIG_PATH) == 0) {
        return -1;
    }

    return config_load(CONFIG_PATH);
}

int config_init(int argc, char* argv[]) {
    char* configPath = config_get_path(argc, argv);

    if (configPath == NULL) {
        log_print("Usage: -c <path to config file>\n", "");
        return -1;
    }

    if (config_load(configPath) == -1) {
        log_print("Can't load config\n", "");
        return -1;
    }

    if (config_save_path(configPath) == -1) {
        log_print("Can't save config path\n", "");
        return -1;
    }

    return 0;
}

int config_free() {
    jsmn_free(&parser);

    return 0;
}

int config_parse_data(const char* data) {
    if (jsmn_init(&parser, data) == -1) {
        return -1;
    }

    if (jsmn_parse(&parser) < 0) {
        return -1;
    }

    jsmntok_t* token_object = jsmn_get_root_token(&parser);

    if (!jsmn_is_object(token_object)) {
        return -1;
    }

    return 0;
}

const jsmntok_t* config_get_section(const char* section) {

    jsmntok_t* token = jsmn_object_get_field(jsmn_get_root_token(&parser), section);

    if (!token) {
        return NULL;
    }

    return token;
}
