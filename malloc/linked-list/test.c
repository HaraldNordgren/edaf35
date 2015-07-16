#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "alloc.h"

void* malloc_and_fill(size_t size, char padding) {
    char* ptr = malloc(size);
    size_t i;

    if (ptr == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++) {
        ptr[i] = padding;
    }
    
    #if DEBUG
    printf("Malloc, size %zu\n", size);
    printf(" ptr:\t%p\n", ptr);
    print_memory();
    print_avail();
    #endif

    return ptr;
}

void free_ptr(void* ptr) {
    free(ptr);

    #if DEBUG
    printf("after freeing ptr (%p)\n", ptr);
    print_memory();
    print_avail();
    #endif
}

int main(void) {

    char *p, *q, *r, *rr, *s, *t, *u;

    p =  malloc_and_fill(9927, 0x12);
    q =  malloc_and_fill(100000, 0x34);
    r =  malloc_and_fill(100000, 0x56); 
    rr = malloc_and_fill( 30000, 0x78);

    free_ptr(p);
    free_ptr(rr); 
    free_ptr(r);
    free_ptr(q);
    
    s = malloc_and_fill(8, 0x78);
    t = malloc_and_fill(30, 0x9a);
    u = malloc_and_fill(4, 0xbc);
    
    free_ptr(s);
    free_ptr(t); 
    free_ptr(u);
    
    return 0;
}
