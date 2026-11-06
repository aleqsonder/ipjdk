#ifndef IPJ_ALLOCATOR_H
#define IPJ_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

struct ipj_allocator { uint32_t capacity, _bump; uint8_t arena[]; };

#define IPJ_ALLOCATOR_INITIAL(BUFSIZE) ((struct ipj_allocator) { \
	.capacity = (BUFSIZE) - sizeof(struct ipj_allocator), \
	._bump = 0 })

void *ipj_alloc(struct ipj_allocator *, size_t size);
void *ipj_realloc(struct ipj_allocator *, void *mem, size_t size);
void ipj_free(struct ipj_allocator *, void *mem);

#endif
