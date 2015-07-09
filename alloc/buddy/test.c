#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "alloc.h"

#if DEBUG_1
void* malloc_and_fill(size_t size, char padding) {

    char* ptr = malloc(size);

    size_t i;

    for (i = 0; i < size; i++) {
        ptr[i] = padding;
    }
    
    printf("Malloc, size %zu\n", size);
    printf(" ptr:\t%p\n", ptr);
    
    print_memory();
    print_freelists();
    
    return ptr;
}

void* realloc_print(void* ptr, size_t size) {

    char* new_ptr = realloc(ptr, size);
    
    printf("Reallocating %p, %zu bytes\n", ptr, size);
    printf(" ptr:\t%p\n", ptr);
    
    print_memory();
    print_freelists();
    
    return new_ptr;
}

void free_ptr(void* ptr) {
    free(ptr);

    printf("Freeing ptr (%p)\n", ptr);
    print_memory();
    print_freelists();
}
#endif

int main(void) {
#if DEBUG_1
#if 0
    size_t *a, *b, *c, *d;
    
    printf("START\n");

    a = malloc_and_fill(70 - LIST_T, 0x12);
    b = malloc_and_fill(35 - LIST_T, 0x34);

    b = realloc_print(b, 35 - LIST_T);

    c = malloc_and_fill(80 - LIST_T, 0x56);
    
    free_ptr(a);
    
    d = malloc_and_fill(60 - LIST_T, 0x78);

    free_ptr(b);
    free_ptr(d);
    free_ptr(c);
    
    printf("TEST FINISHED\n");
#else
    size_t *a, *b, *c, *d, *e, *f, *g, *h, *i;
    size_t a_val, b_val, c_val, d_val, e_val;
    size_t a_size = 32, b_size = 64;
    
    printf("START\n");

    a = malloc_and_fill(a_size - LIST_T, 0x12);
    b = malloc_and_fill(b_size - LIST_T, 0x12);
    
    a_val = *a;
    b_val = *b;

    c = malloc_and_fill(128 - LIST_T, 0x56);
    d = malloc_and_fill(32 - LIST_T, 0x78);

    c = realloc_print(c, 64 - LIST_T);
    c = realloc_print(c, 128 - LIST_T);
    d = realloc_print(d, 32 - LIST_T);
    b = realloc_print(b, 2*b_size - LIST_T);

    c_val = *c;
    d_val = *d;

    e = malloc_and_fill(256 - LIST_T, 0xbc);
    e = realloc_print(e, 256 - LIST_T);
    e = realloc_print(e, 256 - LIST_T);
    e = realloc_print(e, 32 - LIST_T);
    e_val = *e;
    
    f = malloc_and_fill(32 - LIST_T, 0x12);
    g = malloc_and_fill(32 - LIST_T, 0x34);
    h = malloc_and_fill(32 - LIST_T, 0x56);
    i = malloc_and_fill(32 - LIST_T, 0x78);

    a = realloc_print(a, a_size - LIST_T);
    assert(a_val == *a);
    free_ptr(a);
    
    assert(b_val == *b);
    free_ptr(b);

    assert(c_val == *c);
    free_ptr(c);
    
    assert(d_val == *d);
    free_ptr(d);
    
    assert(e_val == *e);
    free_ptr(e);
    
    free_ptr(i);
    free_ptr(h);
    free_ptr(g);
    free_ptr(f);
#endif
#endif

    printf("\npool size (hex): %zx\n", POOL_SIZE);

}
