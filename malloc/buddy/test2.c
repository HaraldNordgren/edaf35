#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include "alloc.h"

int main(void) {
#if DEBUG_1

#if 1
    size_t *p, *q, *r;
    
    p = malloc(11);
    
    printf("after first malloc\n");
    printf(" p:\t%p\n", VOID(p));
    print_memory();
    
    *p = 0x1122;
    *(p+1) = 0x3344;
    
    printf("\nafter filling p\n");
    print_memory();
    
    r = malloc(1);
    *r = 0x5566;
    
    printf("\nafter second malloc and filling r\n");
    printf(" r:\t%p\n", VOID(r));
    print_memory();
    
    q = malloc(8);
    *q = 0x7788;

    printf("\nafter third malloc and filling q\n");
    printf(" q:\t%p\n", VOID(q));
    print_memory();
    
    free(p);
    printf("\nafter freeing p (%p)\n", VOID(p));
    print_memory();
    
    free(q);
    printf("\nafter freeing q (%p)\n", VOID(q));
    print_memory();
    
    free(r);
    printf("\nafter freeing r (%p)\n", VOID(r));
    print_memory();
    
    q = malloc(8);
    *q = 0x99aa;

    printf("\nafter fourth malloc and filling q\n");
    printf(" q:\t%p\n", VOID(q));
    print_memory(); 
    
    q = malloc(30);
    *q = 0xbbcc;

    printf("\nafter fifth malloc and filling q\n");
    printf(" q:\t%p\n", VOID(q));
    print_memory(); 
     
    q = malloc(1);
    *q = 0xddee;

    printf("\nafter sixth malloc and filling q\n");
    printf(" q:\t%p\n", VOID(q));
    print_memory(); 
    return 0;

#else

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


    r = realloc(r, 20);
    
    printf("\nreallocking r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    
    return 0;

#endif
#endif

}
