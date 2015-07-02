// Tomer Elmalem

#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

/* we need this *ONLY* for the size_t typedef */
#include <stdio.h>

/* our metadata structure for use with the buddy system.
 * you *MUST NOT* change this definition unless specified
 * in an official assignment update by the TAs on the newsgroup
 * '.announce'!
 */
typedef struct metadata
{
  short in_use;
  short size;
  struct metadata* next;
  struct metadata* prev;
} metadata_t;


/* this function should allocate a block that is big enough to hold the
 * specified size, and that is all. if there is not a block that is able
 * to satisfy the request, then you should attempt to grab more heap 
 * space with a call to my_sbrk. if this succeeds, then you should continue
 * as normal. if it fails, then you should return NULL.
 */
void* my_malloc(size_t);

/* this function should reallocate space for the specified block with
 * the given size. note that like the POSIX compliant realloc, your
 * function is required to handle the special case of the size being
 * 0, at which point it should free the block passed in. it should also
 * handle the special case of the pointer passed in being NULL by
 * simply malloc'ing the requested size. if it is neither one of these
 * cases, then you should simply malloc a new block of the requested
 * size, copy over the memory in the current block, free the current
 * block, and return the new block. we will not worry about the 
 * possibility of resizing the current block if possible.
 */
void* my_realloc(void *, size_t);

/* this function should free the block of memory, recursively merging
 * buddies up the freelist until they can be merged no more.
 */
void my_free(void *);

/** this function performs the task of locating the buddy of any metadata
* it is given. it takes in a pointer to a particular metadata and returns
* a pointer to the corresponding buddy. 
*/
metadata_t* find_buddy(metadata_t*);

/* this function should perform a byte-by-byte copy of the source area
 * into the desination area, up to the size specified. it should then
 * return a pointer to the destination area of memory.
 */
void* my_memcpy(void *, const void *, size_t);

/* this function will emulate the system call sbrk(2). if you do not
 * have enough free heap space to satisfy a memory request, then you
 * must call this function to allocate memory to your allocator. sound
 * odd? hopefully reading the NFO file will help you understand this.
 */
void* my_sbrk(int);

void init_heap();
int get_index(size_t);
void print_freelist();
void print_block(metadata_t*);
void* offset_pointer(metadata_t*, int);
#endif /* __MY_MALLOC_H__ */
