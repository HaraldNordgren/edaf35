#define MY_MALLOC 1

#if !(MY_MALLOC)
    #include <stdlib.h>
#endif

#include <stdio.h>
#include <unistd.h>

#define SIZE    (1024)
#define N       (8 * SIZE*SIZE / sizeof(size_t))

typedef struct list_t list_t;

struct list_t {
    size_t  size;
    list_t  *next;
    char    *addr;
};

#define LIST_SIZE sizeof(list_t)
#define SIZE_T sizeof(size_t)

static list_t *avail = NULL;
static size_t *memory_start;

void init_print() {
    memory_start = sbrk(0);
}

void print_memory() {
    size_t i;

    printf("\n");
    for (i = 0; i < 10; i++) {
        printf("%p:\t%016zx\n", memory_start + i,
                *(memory_start + i));
    }
}

#if MY_MALLOC
void *malloc(size_t size) {
    list_t *p, *prev;
    size_t *new, segment_size;
    
    if (size <= 0) {
        return NULL; 
    }

    p = avail;
    prev = NULL;

    segment_size = size + SIZE_T;

    while (p != NULL && p->size < segment_size) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        new = sbrk(segment_size);
        *new = segment_size;
        return new + 1;
    }
    
    if (p->size >= segment_size + 2 * LIST_SIZE) {
        printf("large segment\n");
        
        list_t list_entry = *p;

        new = (size_t*) p;
        *new = segment_size;
        

        list_entry.size = list_entry.size - segment_size;
        list_entry.addr = list_entry.addr +  segment_size;

        if (prev != NULL) {
            prev->next = (list_t*) list_entry.addr;
        }

        return new + 1;
    }
     
    if (prev != NULL) {
        printf("\nprev:\t\t%p\n", prev);
        printf("prev->next:\t%p\n", prev->next);
        printf("\np:\t\t%p\n", p);
        printf("p->next:\t%p\n\n", p->next);
        
        prev->next = p->next;
    }
    
    new = (size_t*) p;
    *new = p->size;
    
    if (p == avail) {
        printf("\navail:\t\t%p\n", avail);
        printf("avail->next:\t%p\n", avail->next);

        avail = avail->next;
    }

    return new + 1;
}

void free(void *ptr) {
    list_t  *p, *prev, list_entry;
    size_t  size;
    char    *addr;

    list_entry.addr = (char*) ptr - SIZE_T;
    list_entry.size = *list_entry.addr;
    /* Size of mallocked memory, not freed block! */

    list_entry.next = NULL;

    if (avail == NULL) {
        avail = (list_t*) list_entry.addr;
        *avail = list_entry;
        
        printf("\navail:\t\t%p\n", avail);
        printf("avail->next:\t%p\n", avail->next);
        
        return;
    }

    p = avail;

    while (p != NULL && p->addr < list_entry.addr) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        *(prev->next) = list_entry;
        return;
    }
    
    list_entry.next = p;
    *(prev->next) = list_entry;
}
#endif

int main() {

    init_print();

#if 1
    size_t *p, *q;
    
    printf("\nbefore first malloc\n");
    printf(" p:\t%p\n", p);
    
    p = malloc(2);
    /**p = 0xffffffffffffffff;*/
    
    printf("\nafter first malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
    
    free(p);
    printf("\nafter free\n");
    print_memory();
    
#if 1
    p = malloc(3);
    /**p = 0xffffffffffffffff;*/
    
    printf("\nafter second malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
#endif

    q = malloc(2);
    printf("\nafter third malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    q = malloc(2);
    printf("\nafter fourth malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    q = malloc(2);
    printf("\nafter fifth malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
#endif

    return 0;
}
