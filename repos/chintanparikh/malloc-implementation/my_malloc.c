#include "my_malloc.h"

// Chintan Parikh

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
 * only the _first_ call to my_malloc() returns the beginning of
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

/* 
 * Get the freelist index based on the size
 * Note that size is size_t not an int
 */
int get_index(size_t size)
{
	int index = 0;
	int free_size = 16;
	while ((int) size > free_size)
	{
		free_size = free_size * 2;
		index++;
	}
	return index;
}

/* 
 * Get the buddy for the address given
 * address must be the address of the METADATA, not the address given to the user
 */ 
metadata_t* get_buddy(metadata_t* address)
{
	// XORing here works because address->size will always have the n+1 bit turned on (and only that bit)
	// so when we XOR, we flip the n+1 bit
	int buddy_address = (int) address ^ (int) address->size;
	metadata_t* buddy = (metadata_t*) buddy_address;
	if (address->size == buddy->size) return buddy;

	return NULL;
}

/*
 * We need this when freeing
 * Consider the cash where one->next = two and one and two are buddies
 * where we first remove_from_freelist(two), then remove_from_freelist(one)
 * freelist[index] would be set back to two, which messes up our freelist because
 * one and two are about to be merged (and either way, we just removed two so it 
 * shouldn't be there)
 *
 * The reason we can't just NULL it out is in case there are nodes between the two that need
 * to be kept in the freelist (eg freelist[5]: one -> a -> b -> two)
 */
void setup_for_removal(metadata_t* one, metadata_t* two)
{
	if (one->next == two) one->next = two->next;
	if (one->prev == two) one->prev = two->prev;
	if (two->next == one) two->next = one->next;
	if (two->prev == one) two->prev = one->prev;
}

/* 
 * Remove a node from the freelist
 * Make sure node's next and prev pointers haven't been altered
 */
metadata_t* remove_from_freelist(metadata_t* node)
{	
	metadata_t* next = node->next;
	metadata_t* prev = node->prev;

	int index = get_index(node->size);

	if (prev && next)
	{
		prev->next = next;
		next->prev = prev;
	}
	else if (prev && !next)
	{
		prev->next = NULL;
	}
	else if (!prev && next)
	{
		freelist[index] = next;
		next->prev = NULL;
	}
	else
	{
		freelist[index] = NULL;
	}

	node->next = NULL;
	node->prev = NULL;

	return node;
}

/*
 * Adds a block (or multiple) to the freelist
 * Node must be the head of the linkedlist to be attached
 */
void add_to_freelist(int index, metadata_t* node)
{
	// If there are currently any blocks in the freelist, make sure we add to the tail
	metadata_t *current = freelist[index];
	if (current)
	{
		// Get current tail
		while(current->next)
		{
			current = current->next;
		}
		current->next = node;
		node->prev = current;
		node->next = NULL;
	}
	else
	{
		freelist[index] = node;
	}

}

/*
 * Recursively split memory according to buddy allocation system
 * target_index: the target index in the freelist that you need to hit
 * current_index: your current index that the memory lies in (this gets decremented as the memory is split)
 * current: a pointer to the start of the metadata of the block at freelist[current_index]
 */
void split_memory(int target_index, int current_index, metadata_t *current)
{
	if (target_index == current_index) return;

	// Move current out of the free list
	remove_from_freelist(current);

	current->size /= 2;
	
	// Create new 
	metadata_t *new = (metadata_t *) ((char *) current + current->size);
	new->size = current->size;
	new->in_use = 0;
	new->next = NULL;
	new->prev = current;

	current->next = new;
	current->prev = NULL;

	// Move current into the lower freelist	
	add_to_freelist(current_index - 1, current);

	// Recurse again with the second chunk of memory
	split_memory(target_index, current_index - 1, new);
}

/*
 * Return the node that is first in memory
 */
metadata_t* get_first(metadata_t* one, metadata_t* two)
{
	if (one > two)
	{
		return two;
	}
	return one;
}

/*
 * Merge two buddies and return one large block
 */
metadata_t* merge_buddies(metadata_t* block, metadata_t* buddy)
{
	// Merge into one block (note that the 0 in the n+1 bit comes first)
	metadata_t* first = get_first(block, buddy);

	first->size *= 2;

	return first;
}

/*
 * Remove the block from the freelist, set it's in_use flag, and return the address with correct offset
 */
void* remove_and_return_block(int index)
{
	// The block is now in use, so we remove it from the freelist
	metadata_t* block = remove_from_freelist(freelist[index]);
	block->in_use = 1;
	// Return a pointer to the start of the block (after the metadata)
	ERRNO = NO_ERROR;
	return (void *)(((char *)block) + sizeof(metadata_t));
}

void* my_malloc(size_t size)
{
	// Actual size needed needs to include metadata
	size = sizeof(metadata_t) + size;

	if (size > 2048) 
	{
		ERRNO = SINGLE_REQUEST_TOO_LARGE;
		return NULL;
	}

	// First time running my_malloc?
	if (!heap)
	{
		heap = my_sbrk(SBRK_SIZE);
		freelist[7] = (metadata_t *) heap;
		freelist[7]->in_use = 0;
		freelist[7]->size = SBRK_SIZE;
		freelist[7]->next = NULL;
		freelist[7]->prev = NULL;
	}

	// If my_sbrk failed ...
	if (!heap) return NULL;

	// Get index based on size
	// Round up to nearest power of two, then log2(size) - 3
	int index = get_index(size);

	// If the index needed exists, then we're good
	if (freelist[index])
	{
		return remove_and_return_block(index);
	}
	
	// Loop until we find the next valid index
	int next_index = index + 1;
	while (!freelist[next_index] && next_index <= 8)
	{
		next_index++;
	}

	// If the index we found is actually valid
	if (next_index < 8)
	{
		// Split the memory until we reach the index we wanted (smallest possible size)
		split_memory(index, next_index, freelist[next_index]);
		return remove_and_return_block(index);
	}
	// If we can't find it, then we're out of space and we need to add to the heap
	else
	{
		void* heap_ptr = my_sbrk(SBRK_SIZE);

		if (heap_ptr == (void*) -1)
		{
			ERRNO = OUT_OF_MEMORY;
			return NULL;
		}

		metadata_t *new_heap = (metadata_t *) heap_ptr;

		new_heap->in_use = 0;
		new_heap->size = SBRK_SIZE;
		new_heap->next = NULL;
		new_heap->prev = NULL;

		add_to_freelist(7, new_heap);

		// Now that we have this extra memory, we need to break it down again to the target size
		split_memory(index, 7, freelist[7]);

		return remove_and_return_block(index);
	}
}

void* my_calloc(size_t num, size_t size)
{
	if (num * size > 2048)
	{
		ERRNO = SINGLE_REQUEST_TOO_LARGE;
		return NULL;
	}

	void* p = my_malloc(num * size);

	int i;
	for (i = 0; i < (((metadata_t*)p - 1)->size - sizeof(metadata_t)); i++)
	{
		*((char *) p + i) = 0;
	}
	ERRNO = NO_ERROR;
	return p;
}

void my_free(void* ptr)	
{
	// Calculate proper address
	metadata_t *block = (metadata_t *)((char *)ptr - sizeof(metadata_t));
	
	// Get buddy address
	metadata_t *buddy = get_buddy(block);

	// my_free will only recurse if it has a buddy
	// block->in_use is only set to true when recursing or when its already free
	// so if there isn't a buddy, and the block isn't in use, then we have a double free
	if (!buddy && !block->in_use)
	{
		ERRNO = DOUBLE_FREE_DETECTED;
		return;
	}
	// Set flag
	block->in_use = 0;

	// Check if buddy is free, not in use, and not the max size
	if (buddy && !buddy->in_use && buddy->size != SBRK_SIZE)
	{
		setup_for_removal(block, buddy);
		remove_from_freelist(buddy);
		remove_from_freelist(block);
		metadata_t* first = merge_buddies(block, buddy);

		add_to_freelist(get_index(first->size), first);

		// If there's no buddy, OR it's reached max size, we're done (no need to recurse more)
		// This allows for detection of a double free (and it's just better to not recurse
		// when we don't need to)
		if (get_buddy(first) && first->size != SBRK_SIZE)
		{
			my_free((char *)first + sizeof(metadata_t));
		}
		else
		{
			ERRNO = NO_ERROR;
			return;
		}
	}
	else
	{
		add_to_freelist(get_index(block->size), block);
		ERRNO = NO_ERROR;
		return;
	}
}

void* my_memmove(void* dest, const void* src, size_t num_bytes)
{
    char *d = (char *) dest;
    char *s = (char *) src;
    int i = 0;
    
    if (d == s)
    {
    	ERRNO = NO_ERROR;
    	return (void *)dest;
    }

    if (d > s)
    {
    	for (i = num_bytes - 1; i >= 0; i--)
    	{
    		d[i] = s[i];
    	}
    }
    else
    {
    	for (i = 0; i < num_bytes; i++)
    	{
    		d[i] = s[i];
    	}
    }

    ERRNO = NO_ERROR;
    return (void *) dest;
}
