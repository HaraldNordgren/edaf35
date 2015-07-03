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
    char    data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

static list_t *avail = NULL;
static size_t *memory_start;

void print_memory() {
    size_t i;

    printf("\n");

    for (i = 0; i < 2 * 0xa ; i++) {
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
void *malloc(size_t size) {
    list_t *prev, *p, *mem_segment;
    size_t *new, min_size;
    
    if (size <= 0) {
        return NULL; 
    }

    prev = NULL;
    p = avail;

    min_size = size + LIST_T;

    while (p != NULL && p->size < min_size) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        mem_segment = sbrk(min_size);
        mem_segment->size = min_size;
        
        return mem_segment->data;
    }
    
    if (p->size >= min_size + LIST_T + 8) {
	size_t total_size = p->size;
	list_t* second_next = p->next;
	
	p->size = min_size;
	list_t* next = (list_t*) ( (char*) p + p->size );
	
	next->size = total_size - min_size;

	if (prev == NULL) {
		avail = next;
	} else {
		prev->next = next;
	}

	return p->data;
    }
     
    if (prev != NULL) {
        printf("\nprev:\t\t%p\n", prev);
        printf("prev->next:\t%p\n", prev->next);
        printf("\np:\t\t%p\n", p);
        printf("p->next:\t%p\n\n", p->next);
        
        prev->next = p->next;
    }
    
    if (p == avail) {
        printf("\nMALLOC");
        print_avail();

        /*printf("avail:\t\t%p\t\n", avail);
        printf("avail->next:\t%p\n", avail->next);*/

        avail = avail->next;
    }

    return p->data;
}

void free(void *ptr) {
    list_t  *p, *prev, list_entry, *mem_segment;
    size_t  size;
    char    *addr;

    mem_segment = (list_t*) ( (char*) ptr - LIST_T );

    if (avail == NULL) {
        avail = mem_segment;
        
        printf("\nFREE");
        print_avail();
        
        return;
    }

    prev = NULL;
    p = avail;
    
    while (p != NULL && p < mem_segment) {
        prev = p;
        p = p->next;
    }

    if (prev == NULL) {
        list_t *tmp = avail;

        avail = p;
        p->next = tmp;
        
        return;
    }


    if (p == NULL) {
        printf("\nhej\n");
        prev->next = mem_segment;
        return;
    }
 
    mem_segment->next = p;
    prev->next = mem_segment;
}
#endif

int main() {

    memory_start = sbrk(0);

    size_t *p, *q, *r;
    
    printf("before first malloc\n");
    printf(" p:\t%p\n", p);
    
    p = malloc(8*10);
    
    printf("\nafter first malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
    
    *p = 0x1122;
    *(p+1) = 0x3344;
    
    printf("\nafter filling p\n");
    print_memory();
    
    r = malloc(8);
    *r = 0x5566;
    
    printf("\nafter second malloc and filling r\n");
    printf(" r:\t%p\n", r);
    print_memory();
    
    free(p);
    printf("\nafter freeing p (%p)\n", p);
    print_memory();
    
    q = malloc(8);
    *q = 0x7788;

    printf("\nafter third malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    printf("\navail:\t%p\n", avail);
    
    q = malloc(8);
    *q = 0x99aa;

    printf("\nafter fourth malloc and filling q\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    return 0;
}
