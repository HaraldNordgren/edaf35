#define _BSD_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "alloc.h"

static size_t *memory_start;

void print_memory() {
    size_t i;

    printf("\n");

    for (i = 0; i < 2 * 0xa; i++) {
        printf("%p:\t%016zx\n", memory_start + i, memory_start[i]);
    }

    printf("\n");
}

void print_avail() {
    list_t *p = get_avail();
    
    printf("avail:\t%p\n", p);

    if (p == NULL) {
        printf("\n");
        return;
    }

    while (p->next != NULL) {
        p = p->next;
        printf("next:\t%p\n", p);
    }
    
    if (get_avail() != NULL) {
        printf("next:\t%p\n", p->next);
    }

    printf("\n");
}

void* malloc_and_fill(int size, char padding) {
    char* ptr = malloc(size);
    int i;

    for (i = 0; i < size; i++) {
        ptr[i] = padding;
    }
    
    printf("Malloc, size %d\n", size);
    printf(" ptr:\t%p\n", ptr);
    print_memory();

    return ptr;
}

void free_ptr(void* ptr) {
    free(ptr);

    printf("after freeing ptr (%p)\n", ptr);
    print_memory();
    print_avail();
}


#if 1
int main(void) {

    memory_start = sbrk(0);

    char *p, *q, *r, *rr, *s, *t, *u;

    p = malloc_and_fill(11, 0x12);
    q = malloc_and_fill(15, 0x34);
    r = malloc_and_fill(16, 0x56); 
    rr = malloc_and_fill(16, 0x78);
    print_avail();

    free_ptr(p);
    free_ptr(rr); 
    free_ptr(r);
    free_ptr(q);
    
    s = malloc_and_fill(8, 0x78);
    print_avail();
    
    t = malloc_and_fill(30, 0x9a);
    print_avail();

    u = malloc_and_fill(4, 0xbc);
    print_avail();
    
    return 0;
}

#else
int main(void) {

    memory_start = (size_t*) sbrk(0);

    size_t *p, *r;
    
    p = malloc(40);
    *p = 0x1122;
    *(p+1) = 0x3344;
    
    printf("after first malloc and filling p\n");
    printf(" p:\t%p\n", p);
    print_memory();
    

    r = malloc(12);
    *r = 0x1122334455667788;
    *(r+1) = 0x99aabbccddeeff00; 
    
    printf("\nafter second malloc and filling r\n");
    printf(" r:\t%p\n", r);
    print_memory();


    p = realloc(p, 1);
    
    printf("\nreallocing p\n");
    printf(" p:\t%p\n", p);
    print_memory();
    print_avail();


    r = realloc(r, 20);
    
    printf("\nreallocking r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    print_avail();
    
    return 0;
}
#endif
