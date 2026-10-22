#ifndef IPJ_ALLOCATOR_H
#define IPJ_ALLOCATOR_H

#include <stdint.h>

struct ipj_allocator {
	// bump-allocator
	uint32_t capacity, refcount, _bump;
	uint8_t arena[];
};

void ipj_allocator_init(struct ipj_allocator *, size_t capacity);

void *ipj_alloc(struct ipj_allocator *, size_t size);
void ipj_free(struct ipj_allocator *, void *mem);
#define ipj_mem_offset(ALLOCATOR, MEM) ((uint8_t *)(MEM) - (ALLOCATOR)->arena)

#endif
