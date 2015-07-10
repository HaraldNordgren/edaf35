#define _BSD_SOURCE

#define DEBUG 0
#if DEBUG
    #include <stdio.h>
#endif

#include <unistd.h>
#include <string.h>
#include "alloc.h"

static list_t *avail = NULL;

list_t* get_avail(void) {
    return avail;
}

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
    printf("malloc, size:\t%zu\n", size);
    #endif
    
    if (size <= 0) {
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
            return NULL;
        }

        memory_segment->size = min_size;
        
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

        return p->data;
    }
     
    if (prev != NULL) {
        prev->next = p->next;
    }
    
    if (p == avail) {
        avail = p->next;
    }

    return p->data;
}

void free(void *ptr) {
    list_t *p, *prev, *freed_segment;
    
    #if DEBUG
    printf("free (%p)\n", ptr);
    #endif

    if (ptr == NULL) {
        return;
    }

    freed_segment = (list_t*) ((char*) ptr - LIST_T);

    if (avail == NULL) {
        avail = freed_segment;
        avail->next = NULL;

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
        return;
    }
    
    if (p == NULL) {
        prev->next = freed_segment;
        freed_segment->next = NULL;

        attempt_merge(prev, freed_segment);
        return;
    }
    
    prev->next = freed_segment;
    freed_segment->next = p;

    attempt_merge(freed_segment, p);
    attempt_merge(prev, freed_segment);
}

void *realloc(void *ptr, size_t size) {
    
    #if DEBUG
    printf("realloc\n");
    #endif

    if (ptr == NULL) {
        return malloc(size);
    }

    if (size <= 0) {
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
    
    void *new_ptr = malloc(size);
    memcpy(new_ptr, ptr, old_segment->size - LIST_T);
    
    free(ptr);
    return new_ptr;
}

void* calloc(size_t nmemb, size_t size) {
    #if DEBUG
    printf("calloc\n");
    #endif
    
    void* ptr = malloc(nmemb * size);
    
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }
    
    return ptr;
}
