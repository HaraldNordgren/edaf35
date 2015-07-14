#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "alloc.h"

#define FREELIST_DISPLAY    (4)
#define FREELIST_MIN        (4)
#define PRINT_SIZE          (0x40)

#if DEBUG_1
    #define DEBUG_2 0
    #define DEBUG_3 0
#endif

list_t* start = NULL;
list_t* freelist[N+1];
size_t* memory_start;

int nval;

void* previous_brk;
long brk_offset;

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

void print_ptr(size_t* ptr) {
    printf("\n");

    if (ptr == NULL) {
        printf("%p:\t\n", VOID(ptr));
        return;
    }

    list_t* segment = (list_t*) ((char*) ptr - LIST_T);
    int kval = segment->kval;
    
    size_t i;
    for (i = 0; i < (1L << kval) / sizeof(size_t); i++) {
        printf("%p:\t%016zx\n", (void*) ( ptr + i), ptr[i]);
    }
}
#endif

void init_pool(int k) {
    //k = N - 16;

    //void* start_tmp = sbrk(POOL_SIZE);
    void* start_tmp = sbrk(1L << k);
    previous_brk = sbrk(0);

    if (start_tmp == (void *) -1) {
        //return;
        printf("Initialization of size %zu failed!\n", 1L << k);
    	exit(EXIT_FAILURE);
    }
    
    start = start_tmp;
    nval = k;

    start->reserved = 0;
    start->kval = nval;
    start->succ = NULL;
    start->pred = NULL;
    start->kval_max = N;

    freelist[nval] = start;
    memory_start = (size_t*) start;
}

void *malloc(size_t size) {

    #if DEBUG_2
    printf("\nBefore malloc (size %zu)\n", size);
    #endif
    
    if (size == 0) {
        return NULL;
    }

    size_t min_size = size + LIST_T;
    size_t tmp_size = min_size - 1;

    int k = 0;
    while (tmp_size > 0) {
        tmp_size >>= 1;
        k++;
    }
    
    if (k > N) {
        errno = ENOMEM;
        return NULL;
    }
    
    if (start == NULL) {
        init_pool(k);

        if (start == NULL) {
            errno = ENOMEM;
            return NULL;
        }
    }
    
    #if DEBUG_3
    print_freelists();
    print_memory();
    #endif

    int k_avail = k;
    list_t* block = freelist[k_avail];

    while (block == NULL && k_avail < nval) {
        k_avail++;
        block = freelist[k_avail];
    }

    if (block == NULL) {
        
        if (nval == N) {
            errno = ENOMEM;
            return NULL;
        }

        k_avail = 0;
        
        long brk_diff = (char*) sbrk(0) - (char*) previous_brk;
        printf("  brk diff: %10lu\t(nval: %d)\n", brk_diff, nval);
        
        brk_offset += brk_diff;

        while (k_avail < k) {

            block = sbrk(1L << nval);

            if (block == NULL) {
                printf("failed to sbrk more memory\n");
                errno = ENOMEM;
                return NULL;
            }

            previous_brk = sbrk(0);

            freelist[nval] = block;
            freelist[nval]->kval = nval;
            freelist[nval]->succ = NULL;
            freelist[nval]->pred = NULL;
            freelist[nval]->brk_offset = brk_offset;

            if (brk_offset == 0) {
                freelist[nval]->kval_max = N;
            } else {
                freelist[nval]->kval_max = nval;
            }

            k_avail = nval;
            nval++;
        }
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
        
        second_half->kval_max = first_half->kval_max;
        second_half->brk_offset = first_half->brk_offset;

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
    int kval = freed_segment->kval;

    if (kval == nval || kval == freed_segment->kval_max) {
        return freed_segment;
    }

    size_t diff = (char*) freed_segment - (char*) start - freed_segment->brk_offset;
    size_t buddy_offset = diff ^ (1L << kval);

    list_t* buddy = (list_t*) ((char*) start + buddy_offset);

    list_t* merged_segment = freed_segment;
    
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

    if (ptr < (void*) start || ptr >= (void*) ((char*) start + (1L << nval))) {
        printf("ptr:\t\t%p\n", ptr);
        printf("sbrk(0):\t%p\n", sbrk(0));
        printf("pool end:\t%p\n\n", (void*) ((char*) start + (1L << nval)));
        //print_freelists();

        errno = ENOMEM;
        return;
    }

    list_t* freed_segment = (list_t*) ((char*) ptr - LIST_T);
    freed_segment->reserved = 0;
    
    #if DEBUG_2
    printf("kval: %d (size %zu)\n", freed_segment->kval, 1L << freed_segment->kval);
    #endif
    
    #if DEBUG_3
    print_freelists();
    print_memory();
    print_ptr(ptr);
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
