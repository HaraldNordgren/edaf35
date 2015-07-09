#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include "alloc.h"

int main(void) {
#if DEBUG_1
    size_t *a, *b, *c, *d;
    
    printf("START\n");

    printf("\nAllocating and filling A (70 bytes)\n");
    a = malloc(70 - LIST_T);
    *a = 0x1122;
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");

    printf("\nAllocating and filling B (35 bytes)\n");
    b = malloc(35 - LIST_T);
    *b = 0x3344;
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");
    
    printf("\nReallocating B (35 bytes)\t[b: %p]\n", b);
    b = realloc(b, 35 - LIST_T);
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");

    printf("\nAllocating and filling C (80 bytes)\n");
    c = malloc(80 - LIST_T);
    *c = 0x5566;
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");
    
    printf("\nDeallocating A\t[a: %p]\n", a);
    free(a);
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");

    printf("\nAllocating and filling D (60 bytes)\n");
    d = malloc(60 - LIST_T);
    *d = 0x7788;
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");

    printf("\nDeallocating B\t[b: %p]\n", b);
    free(b);
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");
    
    printf("\nDeallocating D\t[d: %p]\n", d);
    free(d);
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");
    
    printf("\nDeallocating C\t[c: %p]\n", c);
    free(c);
    //print_memory();
    print_freelists();
    printf("\nDone\n\n");

    printf("TEST FINISHED\n");
#endif

    printf("\npool size (hex): %zx\n", POOL_SIZE);

}
