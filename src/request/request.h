#ifndef __REQUEST__
#define __REQUEST__

typedef struct request {
    void(*reset)(void*);
    void(*free)(void*);
} request_t;

#endif
