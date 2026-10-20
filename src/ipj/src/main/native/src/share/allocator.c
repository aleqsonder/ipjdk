#include <stdint.h>
#include <assert.h>

#include "allocator.h"

// bump-allocator

struct ipj_allocator_shared_state {
	uint32_t capacity, refcount, next_ptr;
};

void ipj_allocator_init(
	ipj_allocator_t *allocator,
	size_t capacity,
	void *arena
) {
	assert(allocator);
	assert(arena);

	struct ipj_allocator_shared_state *const shared_state = arena;
	*shared_state = (struct ipj_allocator_shared_state) {
		.capacity = capacity,
		.refcount = 1,
		.next_ptr = 0 };

	void *const shared_arena = (uint8_t *)arena + sizeof(*shared_state);
	*allocator = (ipj_allocator_t) {
		.shared_state = shared_state,
		.arena = shared_arena };
}

// no-op
void ipj_allocator_destroy(ipj_allocator_t *allocator) {
	(void)allocator;
}

void ipj_allocator_add_ref(ipj_allocator_t *allocator) {
	assert(allocator);
	allocator->shared_state->refcount++;
}

int ipj_allocator_release(ipj_allocator_t *allocator) {
	assert(allocator);

	struct ipj_allocator_shared_state *const
	state = allocator->shared_state;

	if (state->refcount > 0)
		--state->refcount;

	return state->refcount == 0;
}

void *ipj_alloc(ipj_allocator_t *allocator, size_t size) {
	assert(allocator);

	struct ipj_allocator_shared_state *const
	state = allocator->shared_state;

	if (size == 0
		|| state->next_ptr >= state->capacity
		|| state->next_ptr + size >= state->capacity
	) return NULL;

	void *const obj = (uint8_t *)allocator->arena + state->next_ptr;
	state->next_ptr += size;

	return obj;
}

// no-op
void ipj_free(ipj_allocator_t *allocator, void *mem) {
	(void)allocator;
	(void)mem;
}
