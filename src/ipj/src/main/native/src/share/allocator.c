#include <stdint.h>
#include <assert.h>

#include "allocator.h"

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
