//#define _BSD_SOURCE
#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "alloc.h"

#define FREELIST_DISPLAY    (4)
#define FREELIST_MIN        (4)

//#define PRINT_SIZE          (0x1000)
#define PRINT_SIZE          (0x40)

#if DEBUG_1
    #define DEBUG_2 0
    #define DEBUG_3 0
#endif

list_t* start = NULL;
list_t* freelist[N+1];
size_t* memory_start;

#if DEBUG_1
#include <stdio.h>

void print_freelists() {
    int i;
    for (i = N; i >= FREELIST_MIN; i--) {
        list_t* e = freelist[i];

        int j = 0;
        
        printf("\nsize %10zu (freelist[%d]):\t", 1L << i, i);
        while (e != NULL && j < FREELIST_DISPLAY) {
            printf("at %zu (%p) -> ",
                    (size_t) ((char*) e - (char*) start), VOID(e));
            e = e->succ;
            j++;
        }

        if (e != NULL) {
            printf("...");
        } else {
            printf("%p", NULL);
        }
    }
    printf("\n\n");
}

void print_memory() {
    printf("\n");
    
    size_t i;
    for (i = 0; i < PRINT_SIZE; i++) {
        printf("%p:\t%016zx\n", (void*) (memory_start + i), memory_start[i]);
    }
}
#endif

void init_pool() {
    void* start_tmp = sbrk(POOL_SIZE);

    if (start_tmp == (void *) -1) {
        //return;
        printf("Not enough memory for initialization\n");
    	exit(EXIT_FAILURE);
    }
    
    start = start_tmp;
    memory_start = (size_t*) start;

    start->reserved = 0;
    start->kval = N;
    start->succ = NULL;
    start->pred = NULL;

    freelist[N] = start;
}

void *malloc(size_t size) {
    
    if (start == NULL) {
        init_pool();

        if (start == NULL) {
            errno = ENOMEM;
            return NULL;
        }
    }

    #if DEBUG_2
    printf("\nBefore malloc (size %zu)\n", size);
    #endif
    
    #if DEBUG_3
    print_freelists();
    print_memory();
    #endif
    
    if (size == 0) {
        return NULL;
    }

    size_t min_size = size + LIST_T;

    int k = 0;
    size_t tmp_size = min_size - 1;

    while (tmp_size > 0) {
        tmp_size >>= 1;
        k++;
    }
    
    int k_avail = k;
    if (k_avail > N) {
        errno = ENOMEM;
        return NULL;
    }

    list_t* block = freelist[k_avail];

    while (block == NULL && k_avail < N) {
        k_avail++;
        block = freelist[k_avail];
    }

    if (block == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    while (k_avail > k) {
        
        list_t* original_segment = freelist[k_avail];
        freelist[k_avail] = original_segment->succ;

        if (original_segment->succ != NULL) {
            original_segment->succ->pred = NULL;
        }
        
        k_avail--;

        list_t* first_half = original_segment;
        list_t* second_half = (list_t*) ((char*) first_half + (1L << k_avail));

        first_half->reserved = 0;
        first_half->kval = k_avail;
        first_half->pred = NULL;
        first_half->succ = second_half;

        second_half->reserved = 0;
        second_half->kval = k_avail;
        second_half->pred = first_half;
        second_half->succ = NULL;

        freelist[k_avail] = first_half;
    }

    list_t* result = freelist[k_avail];
    
    freelist[k_avail] = freelist[k_avail]->succ;
    result->reserved = 1;

    if (result->succ != NULL) {
        result->succ->pred = NULL;
        result->succ = NULL;
    }

    if (result == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    
    return result->data;
}

list_t* recursively_merge(list_t* freed_segment) {

    if (freed_segment->kval == N) {
        return freed_segment;
    }

    size_t diff = (char*) freed_segment - (char*) start;
    size_t buddy_offset = diff ^ (1L << freed_segment->kval);

    list_t* buddy = (list_t*) ((char*) start + buddy_offset);

    list_t* merged_segment = freed_segment;
    int kval = freed_segment->kval;
    
    if (! buddy->reserved && buddy->kval == kval) {
        
        if (freed_segment < buddy) {
            merged_segment = freed_segment;
        } else {
            merged_segment = buddy;
        }

        if (buddy == freelist[kval]) {
            freelist[kval] = buddy->succ;
            if (freelist[kval] != NULL) {
                freelist[kval]->pred = NULL;
            }

        } else {

            buddy->pred->succ = buddy->succ;
            if (buddy->succ != NULL) {
                buddy->succ->pred = buddy->pred;
            }
        }

        #if DEBUG_3
        printf("merged!\n\n");
        print_freelists();
        #endif

        merged_segment->kval++;
        merged_segment = recursively_merge(merged_segment);
    }

    return merged_segment;
}


void free(void *ptr) {
    #if DEBUG_2
    printf("\nBefore free (%p)\n", ptr);
    #endif

    if (ptr == NULL) {
        return;
    }

    if (start == NULL) {
        errno = ENOMEM;
        return;
    }

    if (ptr < (void*) start || ptr >= (void*) ((char*) start + POOL_SIZE)) {
        printf("ptr:\t\t%p\n", ptr);
        printf("sbrk(0):\t%p\n", sbrk(0));
        printf("pool end:\t%p\n\n", (void*) ((char*) start + POOL_SIZE));
        //print_freelists();

        errno = ENOMEM;
        return;
    }

    list_t* freed_segment = (list_t*) ((char*) ptr - LIST_T);
    freed_segment->reserved = 0;
    
    #if DEBUG_2
    printf("size: %zu\n", 1L << freed_segment->kval);
    #endif
    
    #if DEBUG_3
    print_freelists();
    print_memory();
    #endif

    freed_segment = recursively_merge(freed_segment);
    
    int kval = freed_segment->kval;
    
    if (freelist[kval] == NULL) {

        freelist[kval] = freed_segment;
        freelist[kval]->succ = NULL;
        freelist[kval]->pred = NULL;
        
        return;
    }
    
    list_t* p = freelist[kval];
    list_t* prev = NULL;
    
    while (p != NULL && p < freed_segment) {
        prev = p;
        p = p->succ;
    }
    
    if (prev == NULL) {
        freelist[kval] = freed_segment;
        freelist[kval]->succ = p;
        freelist[kval]->pred = NULL;
        p->pred = freed_segment;

        return;
    }
    
    if (p == NULL) {
        prev->succ = freed_segment;
        freed_segment->succ = NULL;
        freed_segment->pred = prev;

        return;
    }
    
    freed_segment->succ = p;
    freed_segment->pred = prev;
    
    prev->succ = freed_segment;
    p->pred = freed_segment;
}

void *realloc(void *ptr, size_t size) {
    #if DEBUG_2
    printf("Before realloc (%p)\n", ptr);
    #endif

    if (start == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return malloc(size);
    }

    void* new_ptr = malloc(size);

    if (new_ptr == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    list_t* old_segment = (list_t*) ((char*) ptr - LIST_T);
    list_t* new_segment = (list_t*) ((char*) new_ptr - LIST_T);
    
    size_t old_size = (1L << old_segment->kval) - LIST_T;
    size_t new_size = (1L << new_segment->kval) - LIST_T;
    size_t min_size;

    if (old_size <= new_size) {
        min_size = old_size;
    } else {
        min_size = new_size;
    }

    memcpy(new_ptr, ptr, min_size);

    free(ptr);

    return new_ptr;
}

void* calloc(size_t nmemb, size_t size) {
    #if DEBUG_2
    printf("Before calloc\n");
    #endif

    if (start == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (nmemb == 0 || size == 0) {
        return NULL;
    }

    size_t total_size = nmemb * size;
    void* ptr = malloc(total_size);
    
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}
