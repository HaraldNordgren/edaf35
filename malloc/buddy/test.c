#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "alloc.h"

#if DEBUG_1
void* malloc_and_fill(size_t data_size, char* val, unsigned char padding) {

    printf("Malloc, size %zu (Including header %zu)\n",
            data_size, data_size + LIST_T);
    
    char* ptr = malloc(data_size);
    printf(" ptr:\t%p\n", ptr);
    
    if (ptr == NULL) {
        fprintf(stderr, "Malloc failed\n");
        exit(EXIT_FAILURE);
    }


    size_t i;

    for (i = 0; i < data_size; i++) {
        ptr[i] = padding;
        val[i] = ptr[i];
    }
    
    print_memory();
    print_freelists();
    
    return ptr;
}

void* realloc_print(void* ptr, size_t size) {

    printf("Reallocating %p, %zu bytes (Including header %zu)\n",
            ptr, size, size + LIST_T);
    
    char* new_ptr = realloc(ptr, size);
    printf(" new_ptr:\t%p\n", new_ptr);
    
    print_memory();
    print_freelists();
    
    return new_ptr;
}

void free_ptr(void* ptr) {
    printf("Freeing ptr (%p)\n", ptr);
    
    free(ptr);

    print_memory();
    print_freelists();
}

void assert_ptr(char* ptr, char* val, size_t size) {
    size_t ii;

    for (ii = 0; ii < size; ii++) {
        /*printf("a_val[%zu]:\t%x\na[%zu]:\t%x\n\n",
                ii, a_val[ii], ii, a[ii]);*/
        assert(val[ii] == ptr[ii]);
    }

}
#endif

int main(void) {
#if DEBUG_1
    size_t a_size = 32 - LIST_T, b_size = 32 - LIST_T,
           c_size = 32 - LIST_T, d_size = 32 - LIST_T,
           e_size = 32 - LIST_T;
    char *a, *b, *c, *d, *e, *f, *g, *h;//, *i;
    char a_val[a_size], b_val[b_size], c_val[c_size],
         d_val[d_size], e_val[e_size], f_val[a_size],
         g_val[a_size], h_val[a_size];
    
    printf("START\n");

    a = malloc_and_fill(a_size, a_val, 0x12);
    b = malloc_and_fill(b_size, b_val, 0x13);
    c = malloc_and_fill(c_size, c_val, 0x14);
    d = malloc_and_fill(d_size, d_val, 0x15);
    
    e = malloc_and_fill(a_size, e_val, 0x23);
    f = malloc_and_fill(a_size, f_val, 0x24);
    g = malloc_and_fill(a_size, g_val, 0x25);
    h = malloc_and_fill(a_size, h_val, 0x26);

    c = realloc_print(c, c_size);
    /*d = realloc_print(d, d_size);
    b = realloc_print(b, 2 * b_size);

    e = malloc_and_fill(e_size, e_val, 0xbc);
    e = realloc_print(e, e_size);
    e = realloc_print(e, e_size);
    e = realloc_print(e, e_size / 8);*/
    
    
    /*f = malloc_and_fill(32, 0x12);
    g = malloc_and_fill(32, 0x34);
    h = malloc_and_fill(32, 0x56);
    i = malloc_and_fill(32, 0x78);*/

    //a = realloc_print(a, a_size);

    assert_ptr(e, e_val, a_size);
    free_ptr(e);

    assert_ptr(a, a_val, a_size);
    free_ptr(a);

    assert_ptr(c, c_val, c_size);
    free_ptr(c);

    assert_ptr(g, g_val, a_size);
    free_ptr(g);
    
    assert_ptr(h, h_val, a_size);
    free_ptr(h);

    assert_ptr(d, d_val, d_size);
    free_ptr(d);

    assert_ptr(f, f_val, a_size);
    free_ptr(f);

    assert_ptr(b, b_val, b_size);
    free_ptr(b);
    
    /*assert_ptr(e, e_val, e_size / 8);
    free_ptr(e);*/
    
    /*free_ptr(i);
    free_ptr(h);
    free_ptr(g);
    free_ptr(f);*/
#endif

    printf("\npool size: 1 << %d = 0x%zx = %zu\n", N, POOL_SIZE, POOL_SIZE);
    
    /*int tmp;
    scanf("%d", &tmp);*/

	return 0;
}
