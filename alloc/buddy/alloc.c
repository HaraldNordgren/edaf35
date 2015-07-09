#define _BSD_SOURCE

#define DEBUG 1
#if DEBUG
    #include <stdio.h>
#endif

#include <string.h>
#include <unistd.h>
#include "alloc.h"

static list_t* start = NULL;
static list_t* freelist[N+1];

static size_t* memory_start;

void print_freelists() {
    int i;
    for (i = N; i >= 4; i--) {
        list_t* e = freelist[i];
        
        printf("\nsize %10d (freelist[%d]):\t", 1 << i, i);
        while (e != NULL) {
            printf("%d (%p) -> ", (int) ((char*) e - (char*) start), e);
            e = e->succ;
        }
    }
    printf("\n");
}

void print_memory() {
    size_t i;

    printf("\n");

    for (i = 0; i < 2 * 0x20; i++) {
        printf("%p:\t%016zx\n", memory_start + i, memory_start[i]);
    }
}

void init_pool() {
    start = sbrk(POOL_SIZE);
    memory_start = (size_t*) start;

    start->reserved = 0;
    start->kval = N;
    start->succ = NULL;
    start->pred = NULL;

    freelist[N] = start;
}

list_t* find_block(int k) {
    
    int k_avail = k;
    list_t* block = freelist[k_avail];

    while (block == NULL && k_avail < N) {
        k_avail++;
        block = freelist[k_avail];
    }

    if (block == NULL) {
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
        list_t* second_half = (list_t*) ((char*) first_half + (1 << k_avail));

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

    return result;
}

void *malloc(size_t size) {
    
    if (start == NULL) {
        init_pool();
    }

    if (size <= 0) {
        return NULL;
    }

    size_t min_size = size + LIST_T;

    char k = 0;
    size_t tmp_size = min_size - 1;

    while (tmp_size > 0) {
        tmp_size >>= 1;
        k++;
    }

    list_t* result = find_block(k);

    if (result == NULL) {
        return NULL;
    }
    
    return result->data;
}

list_t* recursively_merge(list_t* freed_segment) {

    if (freed_segment->kval == N) {
        return freed_segment;
    }

    size_t diff = (char*) freed_segment - (char*) start;
    size_t buddy_offset = diff ^ (1 << freed_segment->kval);

    list_t* buddy = (list_t*) ((char*) start + buddy_offset);

#if 0
    printf("\nfreed_segment:\t\t%d\n", (int) ((char*) freed_segment - (char*) start));
    printf("buddy:\t\t\t%d\n", (int) ((char*) buddy - (char*) start));

    printf("kval:\t\t\t%d\n", freed_segment->kval);
    printf("buddy->kval:\t\t%d\n", buddy->kval);
    
    printf("reserved:\t\t%d\n", freed_segment->reserved);
    printf("buddy->reserved:\t%d\n", buddy->reserved);

    print_freelists();
    printf("\n\n");
#endif

    list_t* merged_segment = freed_segment;
    
    if (! buddy->reserved && buddy->kval == freed_segment->kval) {
        int kval = buddy->kval;

        if (buddy == freelist[kval]) {
            freelist[kval] = buddy->succ;
        }

        if (freed_segment < buddy) {
            merged_segment = freed_segment;
        } else {
            merged_segment = buddy;
        }

        if (buddy->succ != NULL) {
            buddy->succ->pred = buddy->pred;
        }

        if (buddy->pred != NULL) {
            buddy->pred->succ = buddy->succ;
        }

        buddy = buddy->succ;

        merged_segment->kval++;
        merged_segment = recursively_merge(merged_segment);
    }

    return merged_segment;
}


void free(void *ptr) {
    list_t* freed_segment = (list_t*) ((char*) ptr - LIST_T);
    freed_segment->reserved = 0;

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
    
    while (p != NULL && freed_segment > p) {
        prev = p;
        p = p->succ;
    }
    
    if (prev == NULL) {
        freed_segment->succ = p;
        p->pred = freed_segment;

        freelist[kval] = freed_segment;
        freelist[kval]->pred = NULL;

        return;
    }
    
    if (p == NULL) {
        prev->succ = freed_segment;
        freed_segment->succ = NULL;
        freed_segment->pred = prev;

        return;
    }

    if (freed_segment > p) {
        freed_segment->succ = p->succ;
            
        if (freed_segment->succ != NULL) {
            freed_segment->succ->pred = freed_segment;
        }

        freed_segment->pred = p;
        p->succ = freed_segment;

        return;
    }

    freed_segment->succ = p->succ;
    freed_segment->pred = p;
    p->succ = freed_segment;
}

void *realloc(void *ptr, size_t size) {

    list_t* old_segment = (list_t*) ((char*) ptr - LIST_T);
    size_t old_size = (1 << old_segment->kval) - LIST_T;

    void* new_ptr = malloc(size);
    list_t* new_segment = (list_t*) ((char*) new_ptr - LIST_T);
    size_t new_size = (1 << new_segment->kval) - LIST_T;

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
    
    void* ptr = malloc(nmemb * size);
    
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }
    
    return ptr;
}
