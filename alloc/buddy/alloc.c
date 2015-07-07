#define _BSD_SOURCE

#define DEBUG 1
#if DEBUG
    #include <stdio.h>
#endif

#include <unistd.h>
#include "alloc.h"

static char *start = NULL;
static size_t pool_size = 1 << 4;

void *malloc(size_t size) {
    

    return NULL;
}
