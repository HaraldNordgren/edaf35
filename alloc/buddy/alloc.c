#define _BSD_SOURCE

#define DEBUG 1
#if DEBUG
    #include <stdio.h>
#endif

#include <unistd.h>
#include "alloc.h"

static list_t *start = NULL;
static list_t* freelist[N+1];

void print_freelists() {
    int i;
    for (i = N; i >= 4; i--) {
        list_t* e = freelist[i];
        
        printf("\nsize %7d (freelist[%d]):\t", 1 << i, i);
        while (e != NULL) {
            printf("%d -> ", (int) ((char*) e - (char*) start));
            e = e->succ;
        }
    }
    printf("\n\n");
}

void init_pool() {
    start = sbrk(POOL_SIZE);

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

        list_t* first_half = original_segment;
        list_t* second_half = (list_t*) ((char*) first_half +
                (1 << (k_avail - 1)));

        first_half->reserved = 0;
        first_half->kval = k_avail - 1;
        first_half->pred = NULL;
        first_half->succ = second_half;

        second_half->reserved = 0;
        second_half->kval = k_avail - 1;
        second_half->pred = first_half;
        second_half->succ = NULL;

        freelist[k_avail - 1] = first_half;

        //printf("first_half:\t%p\n", first_half);
        //printf("second_half:\t%p\n", second_half);
        
        //print_freelists();
        
        k_avail--;
    }

    list_t* result = freelist[k];
    
    freelist[k] = result->succ;
    result->reserved = 1;

    if (result->succ != NULL) {
        result->succ->pred = NULL;
    }
    
    //print_freelists();

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
    //printf("size: %zu\n", min_size);

    char k = 0;
    size_t tmp_size = min_size - 1;

    while (tmp_size > 0) {
        tmp_size >>= 1;
        k++;
    }

    list_t* result = find_block(k);
    
    return result->data;
}

list_t* recursively_merge(list_t* freed_segment) {

    if (freed_segment->kval == N) {
        return freed_segment;
    }

    size_t diff = (char*) freed_segment - (char*) start;
    size_t buddy_offset = diff ^ (1 << freed_segment->kval);

    list_t* buddy = (list_t*) ((char*) start + buddy_offset);

    /*printf("freed_segment:\t\t%p\n", freed_segment);
    printf("buddy:\t\t\t%p\n", buddy);

    printf("kval:\t\t\t%d\n", freed_segment->kval);
    printf("buddy kval:\t\t%d\n", buddy->kval);
    
    printf("reserved:\t\t%d\n", freed_segment->reserved);
    printf("buddy->reserved:\t%d\n\n", buddy->reserved);*/

    list_t* merged_segment = freed_segment;
    
    if (! buddy->reserved && buddy->kval == freed_segment->kval) {
        
        if (freed_segment < buddy) {
            merged_segment = freed_segment;
        } else {
            merged_segment = buddy;
        }

        if (buddy->pred == NULL && buddy->succ == NULL) {
            freelist[(int) buddy->kval] = NULL;
        } else {
            
            if (buddy->pred != NULL) {
                buddy->pred->succ = buddy->succ;
            }
            
            if (buddy->succ != NULL) {
                buddy->succ->pred = buddy->pred;
            }
        }

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
    
    //printf("freed_segment:\t%p\n", freed_segment);

    if (prev == NULL) {
        freed_segment->succ = p;
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
