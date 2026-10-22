#include <stddef.h>
#include <assert.h>

#include "allocator.h"

void ipj_allocator_init(
	struct ipj_allocator *allocator,
	size_t capacity
) {
	assert(allocator);

	*allocator = (struct ipj_allocator) {
		.capacity = capacity,
		.refcount = 1,
		._bump = 0 };
}

// no-op
void ipj_allocator_destroy(struct ipj_allocator *allocator) {
	(void)allocator;
}

void *ipj_alloc(struct ipj_allocator *allocator, size_t size) {
	assert(allocator);

	if (size == 0
		|| allocator->_bump >= allocator->capacity
		|| allocator->_bump + size >= allocator->capacity
	) return NULL;

	void *const obj = allocator->arena + allocator->_bump;
	allocator->_bump += size;

	return obj;
}

// no-op
void ipj_free(struct ipj_allocator *allocator, void *mem) {
	(void)allocator;
	(void)mem;
}
