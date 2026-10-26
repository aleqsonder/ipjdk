#include <stdint.h>
#include <assert.h>

#include "allocator.h"

struct ipj_allocator {
	uint32_t capacity, _bump;
	uint8_t arena[];
};

struct ipj_allocator *ipj_allocator_markup(size_t bufsize, void *buf) {
	assert(bufsize == 0 || buf);

	if (bufsize < sizeof(struct ipj_allocator))
		return NULL;

	struct ipj_allocator *const allocator = buf;
	*allocator = (struct ipj_allocator) {
		.capacity = bufsize - sizeof(struct ipj_allocator),
		._bump = 0 };

	return allocator;
}

struct ipj_allocator *ipj_allocator_checkout(void *mem) {
	return mem; }
void *ipj_allocator_uncheckout(struct ipj_allocator *allocator) {
	return allocator; }

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

ptrdiff_t ipj_mem_offset(
	const ipj_allocator_t *allocator,
	const void *mem
) {
	assert(allocator);

	if ((const uint8_t *)mem < allocator->arena
		|| allocator->arena + allocator->capacity <= (const uint8_t *)mem
	) return IPJ_INVALID_OFFSET;

	return (const uint8_t *)mem - allocator->arena;
}

void *ipj_offset_resolve(ipj_allocator_t *allocator, ptrdiff_t offset) {
	assert(allocator);

	uint8_t *const ptr = allocator->arena + offset;
	return ptr < allocator->arena + allocator->capacity ? ptr : NULL;
}

const void *ipj_offset_resolve_const(
	const ipj_allocator_t *allocator,
	ptrdiff_t offset
) {
	assert(allocator);

	const uint8_t *const ptr = allocator->arena + offset;
	return ptr < allocator->arena + allocator->capacity ? ptr : NULL;
}
