// Tomer Elmalem
#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the 
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you will receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x)
#endif


/* make sure this always points to the beginning of your current
 * heap space! if it does not, then the grader will not be able
 * to run correctly and you will receive 0 credit. remember that
 * only the _first_ call to malloc() returns the beginning of
 * the heap. sequential calls will return a pointer to the newly
 * added space!
 * Technically this should be declared static because we do not
 * want any program outside of this file to be able to access it
 * however, DO NOT CHANGE the way this variable is declared or
 * it will break the autograder.
 */
void* heap;

/* our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * Technically this should be declared static for the same reasons
 * as above, but DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist[8];
/**** SIZES FOR THE FREE LIST ****
 * freelist[0] -> 16
 * freelist[1] -> 32
 * freelist[2] -> 64
 * freelist[3] -> 128
 * freelist[4] -> 256
 * freelist[5] -> 512
 * freelist[6] -> 1024
 * freelist[7] -> 2048
 */


void* malloc(size_t size)
{
  /* Size of memory needed */
  int needed = size + sizeof(metadata_t);
  
  /* if they request more than 2kb return NULL. */
  if (needed > 2048) return NULL; 

  /* If the heap doesn't exist initialize it. If it still 
     doesn't exist after initialization it failed so 
     return NULL */
  if (!heap) init_heap();
  if (!heap) return NULL;

  /* Get the index of the freelist we need based on the
     needed size */
  int index = get_index(needed);

  /* Declares variables for the front of the freelist
     and the next */
  metadata_t *front, *next;

  /* If there's a block of memory of that size in the
     freelist get it and return it */
  if (freelist[index]) {
    front = freelist[index];
    next = front->next;
    
    /* Set to to in_use, remove its next and prev */
    front->in_use = 1;
    front->next = NULL;
    front->prev = NULL;

    /* If there is a next block in the freelis remove 
       its previous and set the front of the list to it
       else there's nothing in the list */
    if (next) {
      next->prev = NULL;
      freelist[index] = next;
    } else {
      freelist[index] = NULL;
    }
    
    /* Return the pointer + 12 bytes for the metadata */
    return offset_pointer(front, 1);
  }

  /* If we are here that means there's no block of memory
     of our size in the freelist */

  /* Get the next largest block of memory in the freelist */
  int available = index;
  while (!freelist[available]) {
    available++;
  }

  /* If there are no more blocks of free memory in the list
     get more from my_sbrk */
  if (available == 8) {
    metadata_t *new_heap = my_sbrk(SBRK_SIZE);

    // If it's null then my_sbrk errored
    if (!new_heap) return NULL;

    // Initialize the vales
    new_heap->in_use = 0;
    new_heap->size = 2048;
    new_heap->next = NULL;
    new_heap->prev = NULL;

    // If there's something at freelist[7] set it two next
    // else set it to the front
    if (freelist[7]) freelist[7]->next = new_heap;
    else freelist[7] = new_heap;
    available--;
  }
 
  /* Declares variables for the new pointers we'll need */
  metadata_t *current, *new;

  /* While the available isn't the index, keep breaking down
     blocks of memory until you have one at the index you want */
  while (available != index) {
    current = freelist[available];
    current->size /= 2; // split in half

    /* Create a new block at an address that is <size> away
       from current */
    new = (metadata_t *) ((char *) current + current->size);
    new->size = current->size;

    /* If there's something else in the linked list move it to
       the front otherwise set it to NULL */
    if (freelist[available]->next) {
      freelist[available] = freelist[available]->next;
      freelist[available]->prev = NULL;
    } else {
      freelist[available] = NULL;
    }

    // Decrement available
    --available;

    /* Set up the linked lists for the new block. */
    current->next = new;
    new->prev = current;
    freelist[available] = current;
  }

  /* Get the block we want to return from the freelist */
  metadata_t *ret_meta = freelist[index];

  /* If there's a next move it up, else set the index to NULL */
  if (freelist[index]->next) {
    freelist[index] = freelist[index]->next;
    freelist[index]->prev = NULL;
  } else {
    freelist[index] = NULL;
  }

  /* Set the values in the return block */
  ret_meta->next = NULL;
  ret_meta->in_use = 1;

  /* Return the block offset by sizeof(metadata) from the start */
  return offset_pointer(ret_meta, 1);
}

/* Initialize the heap and freelist */
void init_heap() {
  heap = my_sbrk(SBRK_SIZE);
  freelist[7] = (metadata_t *) heap;
  freelist[7]->in_use = 0;
  freelist[7]->size = 2048;
  freelist[7]->next = NULL;
  freelist[7]->prev = NULL; 
}

/* Get the index needed based on the size. Loops until the
   memory that corresponds to the index is larger than needed */
int get_index(size_t needed) {
  int index = 0;
  int memory = 16;
  while (memory < needed) {
    memory *= 2;
    index++;
  }

  return index;
}

/* Function to offset the pointer. Takes in a metadata_t *
   and an int offset that determines which direction to go.
   Returns either ptr + sizeof() if offset is 1 else
   returns ptr - sizeof() */
void* offset_pointer(metadata_t* ptr, int offset) {
  char *offset_ptr = (char *) ptr;
  if (offset) return offset_ptr + sizeof(metadata_t);
  else return offset_ptr - sizeof(metadata_t);
}

/* Print the freelist, for debugging */
void print_freelist() {
  int size = 16;
  for (int i=0; i<8; i++) {
    fprintf(stderr, "[%d] -> %d: %p\n", i, size, (void *) freelist[i]);
    size *= 2;
  }
}

/* Print the metadata for the block of memory, for debugging */
void print_block(metadata_t *block) {
  fprintf(stderr, "Printing block data\n");
  fprintf(stderr, "address: %p\n", (void *) block);
  fprintf(stderr, "in use: %d\n", block->in_use);
  fprintf(stderr, "size: %d\n", block->size);
  fprintf(stderr, "next: %p\n", (void *) block->next);
  fprintf(stderr, "prev: %p\n", (void *) block->prev);
  fprintf(stderr, "\n");
}

void* realloc(void* ptr, size_t new_size)
{
  void *new;

  /* If the size is 0, realloc should act like free
     and return NULL */
  if (new_size == 0) {
    free(ptr);
    return NULL;
  }

  /* Allocate the new block of memory */
  new = malloc(new_size);

  /* If the pointer is null then realloc should just act
     like malloc, so return the pointer to the new
     block of memory */
  if (ptr == NULL) return new;
 
  /* Get the offset of the pointer so we can get the size
     we need to copy */
  metadata_t *old = offset_pointer(ptr, 0);

  /* Copy the old (ptr) to the new with size - metadata */
  my_memcpy(new, ptr, old->size - sizeof(metadata_t));

  free(ptr);

  return new;
}

void free(void* ptr)
{
  /* Get the other block of memory and offset it correctly. */
  metadata_t *block = offset_pointer(ptr, 0);
  block->in_use = 0; // no longer in use

  /* Get the buddy! */
  metadata_t *buddy = find_buddy(block);

  /* While buddy exists and it's not in use, merge it */
  while (buddy && !buddy->in_use) {
    /* Modify the linked list accordingly */
    if (buddy->next && buddy->prev) {
      buddy->next->prev = buddy->prev;
      buddy->prev->next = buddy->next;
    } else if (buddy->next) {
      buddy->next->prev = NULL;
    } else if (buddy->prev) {
      buddy->prev->next = NULL;
    } else {
      freelist[get_index(buddy->size)] = NULL;
    }
    
    /* If the address of buddy is lower than the block, 
       use that one instead */
    if (buddy < block) block = buddy;

    /* Double the size to account for the combination */
    block->size *= 2;

    /* Find the new block's buddy */
    buddy = find_buddy(block);
  }

  /* If there's something at the index position of the 
     freelist, push it to the front, otherwise just place
     it there */
  int index = get_index(block->size);
  if (freelist[index]) {
    metadata_t *front = freelist[index];
    front->prev = block;
    block->next = front;
  }
  freelist[index] = block;
}

/* Find the buddy for a given block. This is done fairly
   simply. Since the address of the buddy should just be
   size away from the block, either add or subtract that
   depending on which buddy you have. Alternatively, you
   could XOR the address by the size, same thing as:
        ptr +/- (1 << log2(size))
   Then return it. */
metadata_t* find_buddy(metadata_t* ptr){
  int buddy = (int) ptr ^ ptr->size;
  metadata_t *b = (metadata_t *) buddy;
  if (ptr->size == b->size) return b;
  else return NULL;
}

/* Do a byte by byte copy. Make sure to cast the pointers
   as chars so you copy bytes and nothing larger. Loop through
   the amount of bytes you want:
        metadata_t->size - sizeof(metadata_t)
   */
void* my_memcpy(void* dest, const void* src, size_t num_bytes)
{
  char *d = (char *) dest;
  char *s = (char *) src;
  for (int i=0; i<num_bytes; i++) {
    d[i]=s[i];
  }
  return d;
}



void* calloc(size_t num, size_t size)
{
    if (num * size > 2048)
    {
        ERRNO = SINGLE_REQUEST_TOO_LARGE;
        return NULL;
    }

    void* p = malloc(num * size);

    int i;
    for (i = 0; i < (((metadata_t*)p - 1)->size - sizeof(metadata_t)); i++)
    {
        *((char *) p + i) = 0;
    }
    ERRNO = NO_ERROR;
    return p;
}
