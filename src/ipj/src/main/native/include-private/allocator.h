#ifndef IPJ_ALLOCATOR_H
#define IPJ_ALLOCATOR_H

#include <stddef.h>

typedef struct {
	struct ipj_allocator_shared_state *shared_state;
	void *arena;
} ipj_allocator_t;

void ipj_allocator_init(ipj_allocator_t *, size_t capacity, void *arena);
void ipj_allocator_destroy(ipj_allocator_t *);

void ipj_allocator_add_ref(ipj_allocator_t *);

// returns non-zero on refcount==0
int ipj_allocator_release(ipj_allocator_t *);

void *ipj_alloc(ipj_allocator_t *, size_t size);
void ipj_free(ipj_allocator_t *, void *mem);

#endif
