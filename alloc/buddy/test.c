#include <stdio.h>
#include "alloc.h"

int main(void) {
    void *a, *b, *c, *d;

    printf("START\n");

    a = malloc(70 - LIST_T);
    print_freelists();
    printf("After allocating 70\n");

    b = malloc(35 - LIST_T);
    print_freelists();
    printf("After allocating 35\n");

    c = malloc(80 - LIST_T);
    print_freelists();
    printf("After allocating 80\n");
    
    free(a);
    print_freelists();
    printf("After deallocating A\n");

    d = malloc(60 - LIST_T);
    print_freelists();
    printf("After allocating 60\n");

    free(b);
    print_freelists();
    printf("After deallocating B\n");
    
    free(d);
    print_freelists();
    printf("After deallocating D\n");
    
    free(c);
    print_freelists();
    printf("After deallocating C\n");
}
