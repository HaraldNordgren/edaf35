#define _BSD_SOURCE

#include <unistd.h>
#include "alloc.h"

int main(void) {
    size_t i;

    for (i = 0; i < 900; i++) {
        if (fork() == 0) {
            void* p = malloc(4);
            realloc(p, 8);
        }
    }
}
