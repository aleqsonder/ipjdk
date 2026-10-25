#ifndef IPJ_ALLOCATOR_H
#define IPJ_ALLOCATOR_H

#include <stddef.h>

typedef struct ipj_allocator ipj_allocator_t;

ipj_allocator_t *ipj_allocator_markup(size_t bufsize, void *buf);
void *ipj_allocator_unmarkup(ipj_allocator_t *);

void *ipj_alloc(ipj_allocator_t *, size_t size);
void ipj_free(ipj_allocator_t *, void *mem);

typedef ptrdiff_t ipj_offset_t;
#define IPJ_INVALID_OFFSET PTRDIFF_MAX

// Returns IPJ_INVALID_OFFSET on out-of-range
ipj_offset_t ipj_mem_offset(const ipj_allocator_t *, const void *mem);

// Returns NULL on out-of-range
void *ipj_offset_resolve(ipj_allocator_t *, ipj_offset_t offset);
const void *ipj_offset_resolve_const(
	const ipj_allocator_t *,
	ipj_offset_t offset);

#endif
