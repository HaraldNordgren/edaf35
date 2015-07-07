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
    char offset;

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
    size_t min_size;

    #if DEBUG
    printf("malloc, size:\t%zu\n", size);
    #endif
    
    if (size <= 0) {
        return NULL; 
    }

    size = align_size(size);
    min_size = size + LIST_T;

    p = avail;
    prev = NULL;

    #if DEBUG
    printf("p:\t\t%p\n", p);
    #endif
    
    while (p != NULL && p->size < min_size) {
        #if DEBUG
        printf("p->next:\t%p\n", p->next);
        #endif

        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        memory_segment = sbrk(min_size);
        memory_segment->size = min_size;
        
        return memory_segment->data;
    }
    
    if (p->size >= min_size + LIST_T + SIZE_T) {
        size_t total_size = p->size;
        p->size = min_size;
        
        list_t* second_next = p->next;
        list_t* first_next = (list_t*) ((char*) p + p->size);
        
        first_next->size = total_size - min_size;
        first_next->next = second_next;

        if (prev == NULL) {
            avail = first_next;
        } else {
            prev->next = first_next;
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
    printf("free\n");
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
        list_t *tmp = avail;
        avail = freed_segment;
        freed_segment->next = tmp;

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

    list_t *old_segment = (list_t*) ((char*) ptr - LIST_T);

    size_t minimum_new_size = align_size(size) + LIST_T;
    long size_diff = minimum_new_size - old_segment->size;
    
    if (size_diff <= 0) {

        if ((size_t) -size_diff > LIST_T + SIZE_T) {
            list_t *list_to_free = (list_t*) ((char*) old_segment + minimum_new_size);
            list_to_free->size = -size_diff;
            
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
