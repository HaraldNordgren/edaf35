#define _BSD_SOURCE

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "alloc.h"

#define PRINT_MAX (2 * 0xa)

list_t *avail = NULL;

#if DEBUG
    #define DEBUG_1 1
    #define DEBUG_2 1

size_t *memory_start = NULL;

void indent() {
    printf("  ");
}

list_t* get_avail(void) {
    return avail;
}

void print_memory() {

    printf("\n");
    
    if (memory_start == NULL) {
        printf("Memory Uninitialized\n");
        return;
    }

    long brk_diff = (char*) sbrk(0) - (char*) memory_start;
    unsigned print_max;
    
    if (brk_diff >= PRINT_MAX) {
        print_max = PRINT_MAX;
    } else {
        print_max = brk_diff;
    }

    size_t i;
    for (i = 0; i < print_max; i++) {
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
#endif

size_t align_size(size_t size) {
    unsigned offset;

    offset = size % SIZE_T;
    if (offset != 0) {
        return size + SIZE_T - offset;
    }

    return size;
}

void attempt_merge(list_t* p, list_t* q) {
    if ((list_t*) ((char*) p + p->size) == q) {
        p->size += q->size;
        p->next = q->next;
    }
}

void *malloc(size_t size) {
    list_t *p, *prev, *memory_segment;

    #if DEBUG
    if (memory_start == NULL) {
        memory_start = sbrk(0);
    }
    #endif

    #if DEBUG_1
    printf("Malloc size %zu\n", size);
    #endif
    
    if (size == 0) {
        return NULL; 
    }

    size_t min_size = align_size(size) + LIST_T;

    p = avail;
    prev = NULL;
    
    while (p != NULL && p->size < min_size) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        memory_segment = sbrk(min_size);

        if ((void*) memory_segment == (void*) -1) {
            //printf("Not enough memory for sbrk (size: %zu)\n", (size_t) sbrk(0));
            //exit(EXIT_FAILURE);
            
            return NULL;
        }

        memory_segment->size = min_size;

        #if DEBUG_2
        print_memory();
        print_avail();
        #endif
        
        return memory_segment->data;
    }
    
    /* Space for the allocation and for at least one word after,
     * split the segment in two*/
    if (p->size >= min_size + LIST_T + SIZE_T) {
        size_t total_size = p->size;
        p->size = min_size;
        
        list_t* new_segment = (list_t*) ((char*) p + p->size);
        
        new_segment->size = total_size - min_size;
        new_segment->next = p->next;

        if (prev == NULL) {
            avail = new_segment;
        } else {
            prev->next = new_segment;
        }

        #if DEBUG_2
        print_memory();
        print_avail();
        #endif

        return p->data;
    }
     
    if (prev != NULL) {
        prev->next = p->next;
    }
    
    if (p == avail) {
        avail = p->next;
    }

    #if DEBUG_2
    print_memory();
    print_avail();
    #endif

    return p->data;
}

void free(void *ptr) {
    list_t *p, *prev, *freed_segment;
    
    #if DEBUG_1
    printf("Free (%p)\n", ptr);
    #endif

    if (ptr == NULL) {
        return;
    }

    freed_segment = (list_t*) ((char*) ptr - LIST_T);

    if (avail == NULL) {
        avail = freed_segment;
        avail->next = NULL;

        #if DEBUG_2
        print_memory();
        print_avail();
        #endif

        return;
    }

    p = avail;
    prev = NULL;
    
    while (p != NULL && p < freed_segment) {
        prev = p;
        p = p->next;
    }

    if (prev == NULL) {
        freed_segment->next = avail;
        avail = freed_segment;

        attempt_merge(freed_segment, freed_segment->next);

        #if DEBUG_2
        print_memory();
        print_avail();
        #endif

        return;
    }
    
    if (p == NULL) {
        prev->next = freed_segment;
        freed_segment->next = NULL;

        attempt_merge(prev, freed_segment);

        #if DEBUG_2
        print_memory();
        print_avail();
        #endif

        return;
    }
    
    prev->next = freed_segment;
    freed_segment->next = p;

    attempt_merge(freed_segment, p);
    attempt_merge(prev, freed_segment);
}

void *realloc(void *ptr, size_t size) {
    
    #if DEBUG_1
    printf("Realloc size %zu (from %p)\n", size, ptr);
    #endif

    #if DEBUG_2
    print_memory();
    print_avail();
    #endif

    if (ptr == NULL) {
        #if DEBUG_1
        indent();
        #endif

        return malloc(size);
    }

    if (size == 0) {
        #if DEBUG_1
        indent();
        #endif
        
        free(ptr);    
        return NULL;
    }

    size_t minimum_new_size = align_size(size) + LIST_T;
    list_t *old_segment = (list_t*) ((char*) ptr - LIST_T);

    if (minimum_new_size < old_segment->size) {

        size_t size_diff = old_segment->size - minimum_new_size;

        if (size_diff >= LIST_T + SIZE_T) {
            
            list_t *list_to_free = (list_t*) ((char*) old_segment +
                    minimum_new_size);
            
            list_to_free->size = size_diff;
            free((char*) list_to_free + LIST_T);
            
            old_segment->size = minimum_new_size;
        }

        return ptr;
    }

    #if DEBUG_1
    indent();
    #endif
    
    void *new_ptr = malloc(size);
    memcpy(new_ptr, ptr, old_segment->size - LIST_T);

    #if DEBUG_1
    indent();
    #endif
    
    free(ptr);
    return new_ptr;
}

void* calloc(size_t nmemb, size_t size) {
    #if DEBUG_1
    printf("Calloc size %zu\n", nmemb * size);
    #endif

    #if DEBUG_2
    print_memory();
    print_avail();
    #endif

    #if DEBUG_1
    indent();
    #endif
    
    void* ptr = malloc(nmemb * size);
    
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }
    
    return ptr;
}
