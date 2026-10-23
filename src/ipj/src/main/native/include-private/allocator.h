#ifndef IPJ_ALLOCATOR_H
#define IPJ_ALLOCATOR_H

#include <stddef.h>

typedef struct ipj_allocator ipj_allocator_t;

ipj_allocator_t *ipj_allocator_markup(size_t bufsize, void *buf);

void *ipj_alloc(ipj_allocator_t *, size_t size);
void ipj_free(ipj_allocator_t *, void *mem);

ptrdiff_t ipj_mem_offset(const ipj_allocator_t *, const void *mem);

void *ipj_offset_resolve(ipj_allocator_t *, ptrdiff_t offset);
const void *ipj_offset_resolve_const(
	const ipj_allocator_t *,
	ptrdiff_t offset);

#endif
