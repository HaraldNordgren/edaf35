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
}

void print_avail() {
    list_t *p = get_avail();
    
    printf("\navail:\t%p\n", p);

    if (p == NULL) {
        return;
    }

    while (p->next != NULL) {
        p = p->next;
        printf("next:\t%p\n", p);
    }
    
    if (get_avail() != NULL) {
        printf("next:\t%p\n", p->next);
    }
}

#if 1
int main() {

    memory_start = sbrk(0);

    size_t *p, *q, *r;
    
    p = malloc(11);
    
    printf("after first malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
    
    *p = 0x1122;
    *(p+1) = 0x3344;
    
    printf("\nafter filling p\n");
    print_memory();
    
    r = malloc(1);
    *r = 0x5566;
    
    printf("\nafter second malloc and filling r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    
    q = malloc(8);
    *q = 0x7788;

    printf("\nafter third malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory();
    print_avail();
    
    free(p);
    printf("\nafter freeing p (%p)\n", p);
    print_memory();
    
    free(q);
    printf("\nafter freeing q (%p)\n", q);
    print_memory();
    print_avail();
    
    free(r);
    printf("\nafter freeing r (%p)\n", r);
    print_memory();
    print_avail();
    
    q = malloc(8);
    *q = 0x99aa;

    printf("\nafter fourth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
    
    q = malloc(30);
    *q = 0xbbcc;

    printf("\nafter fifth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
     
    q = malloc(1);
    *q = 0xddee;

    printf("\nafter sixth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
    return 0;
}

#else
int main() {

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
