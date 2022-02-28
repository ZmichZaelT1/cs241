/**
 * malloc
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _metadata_t {
    void *ptr;
    size_t size;
    int free;
    struct _metadata_t *next;
    struct _metadata_t *prev;
} metadata_t;

static metadata_t *head = NULL;
static metadata_t *tail = NULL;
static int freed = 0;
// static size_t mem_free = 0;

void merge(metadata_t *first, metadata_t *second) {
    if (second == tail) {
        tail = first;
    }
    metadata_t *next = second->next;
    first->size += second->size + sizeof(metadata_t);
    first->next = next;
    if (next) next->prev = first;
}

void split_block(metadata_t *block, size_t size) {
    metadata_t *next = block->next;
    metadata_t *new = block->ptr + size;
    new->ptr = new + 1;
    new->size = block->size - size - sizeof(metadata_t);
    new->free = 1;
    new->next = next;
    new->prev = block;
    block->size = size;
    block->next = new;
    if (next) next->prev = new;
    if (block == tail) tail = new;
    // if(new->next && new->next->free) merge(new, new->next);
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void *ret = malloc(num * size);
    if (!ret) return NULL;
    memset(ret, 0, num * size);
    return ret;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    metadata_t *curr = tail;

    metadata_t *chosen = NULL;
    if (freed) {
        if (!tail) {
            curr = head;
        }
        while (curr) {
            if (curr->free && curr->size >= size) {
                chosen = curr;
                if (size/3 < chosen->size-size && chosen->size-size > sizeof(metadata_t)) {
                    split_block(chosen, size);
                }
                break;
                // if (!chosen || (chosen && curr->size < chosen->size)) {
                //     chosen = curr;
                // }
            }
            curr = curr->prev;
        }
    }
    if (chosen) {
        chosen->free = 0;
        return chosen->ptr;
    }

    chosen = sbrk(0);
    sbrk(sizeof(metadata_t));
    chosen->ptr = sbrk(0);
    if (sbrk(size) == (void*)-1) {
        return NULL;
    }

    chosen->size = size;
    chosen->free = 0;
    // chosen->next = NULL;
    if (!head) {
        head = chosen;
        head->next = tail;
        head->prev = NULL;
    } else {
        if (tail) {
            tail->next = chosen;
            chosen->prev = tail;
            tail = chosen;
            tail->next = NULL;
        } else {
            tail = chosen;
            head->next = tail;
            tail->prev = head;
            tail->next = NULL;
        }
    }
    // chosen->prev = tail;

    // if (head) head->prev = chosen;
    // head = chosen;
    return chosen->ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (!ptr) return;

    metadata_t *curr = ptr - sizeof(metadata_t);
    // if(curr->free) return;
    // // while (curr) {
    curr->free = 1;
    // int merged_left = 0;
    if (curr->prev) {
        if(curr->prev->free) {
            merge(curr->prev, curr);
            curr = curr->prev;
        }
    }
    if (curr->next) {
        if(curr->next->free) {
            merge(curr, curr->next);
        }
    }

        // break;
    // curr = curr->next;
    // }

    freed = 1;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (!ptr) return malloc(size);
    metadata_t *entry = ((metadata_t*)ptr) - 1;
    size_t oldsize = entry->size;

    if (oldsize < size) {
        void *result = malloc(size);
        memcpy(result, ptr, oldsize);
        free(ptr);
        return result;
    } else if (oldsize > size) {
        // if (oldsize > 2*size && (oldsize - size) > 1024) {
        // }
        void *result = malloc(size);
        memcpy(result, ptr, size);
        free(ptr);
        return result;
    }
    return ptr;
}
