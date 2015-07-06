#define MY_MALLOC 1

#if !(MY_MALLOC)
    #include <stdlib.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SIZE    (1024)
#define N       (8 * SIZE*SIZE / sizeof(size_t))

typedef struct list_t list_t;

struct list_t {
    size_t  size;
    list_t  *next;
    char    data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

static list_t *avail = NULL;
static size_t *memory_start;

void print_memory() {
    size_t i;

    printf("\n");

    for (i = 0; i < 2 * 0xa; i++) {
        printf("%p:\t%016zx\n", memory_start + i, memory_start[i]);
    }
}

void print_avail() {
    list_t *p = avail;
    
    printf("\navail:\t%p\n", p);

    if (p == NULL) {
        return;
    }

    while (p->next != NULL) {
        p = p->next;
        printf("next:\t%p\n", p);
    }
    
    if (avail != NULL) {
        printf("next:\t%p\n", p->next);
    }
}

#if MY_MALLOC

size_t align_size(size_t size) {
    char offset;

    offset = size % SIZE_T;
    if (offset != 0) {
        return size + SIZE_T - offset;
    }

    return size;
}

void merge(list_t* p, list_t* q) {
    if ((list_t*) ((char*) p + p->size) == q) {
        p->size += q->size;
        p->next = q->next;
    }
}

void *malloc(size_t size) {
    list_t *prev, *p, *freed_segment;
    size_t *new, min_size;
    
    if (size <= 0) {
        return NULL; 
    }

    size = align_size(size);

    prev = NULL;
    p = avail;

    min_size = size + LIST_T;

    while (p != NULL && p->size < min_size) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        freed_segment = sbrk(min_size);
        freed_segment->size = min_size;
        
        return freed_segment->data;
    }
    
    if (p->size >= min_size + LIST_T + SIZE_T) {
        size_t total_size = p->size;
        p->size = min_size;
        
        list_t* second_next = p->next;
        list_t* first_next = (list_t*) ( (char*) p + p->size );
        
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
        avail = avail->next;
    }

    return p->data;
}

void free(void *ptr) {
    list_t  *p, *prev, *freed_segment;
    size_t  size;
    char    *addr;

    if (ptr == NULL) {
        return;
    }

    freed_segment = (list_t*) ((char*) ptr - LIST_T);

    if (avail == NULL) {
        avail = freed_segment;
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

        merge(freed_segment, freed_segment->next);
    } else if (p == NULL) {
        prev->next = freed_segment;
        freed_segment->next = NULL;

        merge(prev, freed_segment);
    } else {
        freed_segment->next = p;
        prev->next = freed_segment;

        merge(freed_segment, p);
        merge(prev, freed_segment);
    }
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }

    if (size <= 0) {
        free(ptr);
        return;
    }

    list_t *old_segment = (list_t*) ((char*) ptr - LIST_T);

    size_t minimum_new_size = align_size(size) + LIST_T;
    long size_diff = minimum_new_size - old_segment->size;
    
    if (size_diff <= 0) {
        if (-size_diff > LIST_T + SIZE_T) {
            list_t *list_to_free = (list_t*) ((char*) old_segment + minimum_new_size);
            
            list_to_free->size = -size_diff;
            free((char*) list_to_free + LIST_T);

            old_segment->size = minimum_new_size;
        }

        return ptr;

    } else {
        void *new_ptr = malloc(size);
        memcpy((char*) new_ptr, (char*) ptr, old_segment->size - LIST_T);

        free(ptr);

        return new_ptr;
    }
}


void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    
    if (ptr != NULL) {
        memset(ptr, 0, nmemb * size);
    }
    
    return ptr;
}

#endif

#if 0
int main() {

    memory_start = sbrk(0);

    size_t *p, *q, *r;
    
    printf("before first malloc\n");
    printf(" p:\t%p\n", p);
    
    p = malloc(11);
    
    printf("\nafter first malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
    
    *p = 0x1122;
    *(p+1) = 0x3344;
    
    printf("\nafter filling p\n");
    print_memory();
    
    r = malloc(1);
    *r = 0x5566;
    
    printf("\nafter second malloc and filling r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    
    q = malloc(8);
    *q = 0x7788;

    printf("\nafter third malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    printf("\navail:\t%p\n", avail);
    
    free(p);
    printf("\nafter freeing p (%p)\n", p);
    print_memory();
    
    free(q);
    printf("\nafter freeing q (%p)\n", q);
    print_memory();
    print_avail();
    
    free(r);
    printf("\nafter freeing r (%p)\n", r);
    print_memory();
    print_avail();
    
    q = malloc(8);
    *q = 0x99aa;

    printf("\nafter fourth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
    
    q = malloc(30);
    *q = 0xbbcc;

    printf("\nafter fifth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
     
    q = malloc(1);
    *q = 0xddee;

    printf("\nafter sixth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory(); 
    print_avail();
    return 0;
#endif

#if 1
int main() {

    memory_start = sbrk(0);

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
    print_avail();


    r = realloc(r, 20);
    
    printf("\nreallocking r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    print_avail();
    
    return 0;
#endif
}
