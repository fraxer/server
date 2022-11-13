#define _GNU_SOURCE
#include <stddef.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <stdlib.h>
#include "threadworker.h"
#include "../log/log.h"
#include "../epoll/epoll.h"
    #include <stdio.h>

typedef struct thread_worker_item {
    pthread_t thread;
    server_t* server;
} thread_worker_item_t;

static int thread_worker_index = 0;
static int thread_worker_count = 0;

static thread_worker_item_t* thread_workers = NULL;

void* thread_worker(void* server) {
    // if (fd_handler == THREAD_EPOLL) {
        epoll_run(server);
    // }
    // if (fd_handler == THREAD_SELECT) {
    //     select_run();
    // }
    // if (fd_handler == THREAD_POLL) {
    //     poll_run();
    // }
    // if (fd_handler == THREAD_KQUEUE) {
    //     kqueue_run();
    // }

    pthread_exit(NULL);
}

thread_worker_item_t* thread_worker_alloc(int count) {
    return (thread_worker_item_t*)malloc(sizeof(thread_worker_item_t) * count);
}

thread_worker_item_t* thread_worker_create(int count, server_t* server) {
    thread_worker_item_t* threads = thread_worker_alloc(count);

    if (threads == NULL) return NULL;

    size_t thread_size = sizeof(thread_worker_item_t);

    for (int i = 0; i < count; i++) {
        threads[i].server = server;
    }

    return threads;
}

int thread_worker_run(int worker_count, server_t* server) {
    thread_worker_item_t* threads = NULL;

    if (thread_worker_count > worker_count) {
        // -
        threads = thread_worker_create(thread_worker_count + worker_count, server);
    }
    else {
        // +
        threads = thread_worker_create(thread_worker_count * 2 + worker_count, server);
    }

    if (threads == NULL) return -1;

    for (int i = thread_worker_index, j = 0; i < thread_worker_index + thread_worker_count; i++, j++) {
        threads[j].server = thread_workers[i].server;
        threads[j].server->is_deprecated = 1;
    }

    for (int i = thread_worker_count; i < thread_worker_count + worker_count; i++) {
        if (pthread_create(&threads[i].thread, NULL, thread_worker, threads[i].server) != 0) {
            log_error("Thread error: Unable to create worker thread\n");
            return -1;
        }

        pthread_detach(threads[i].thread);

        pthread_setname_np(threads[i].thread, "Server worker");
    }

    if (thread_worker_count > worker_count) {
        // -
        thread_worker_index = thread_worker_count;

        thread_worker_count = worker_count;
    }
    else {
        // +
        thread_worker_count = thread_worker_count + worker_count;

        thread_worker_index = thread_worker_count - worker_count;
    }

    if (thread_workers) free(thread_workers);

    thread_workers = threads;

    return 0;
}