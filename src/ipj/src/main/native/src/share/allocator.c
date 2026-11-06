#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "allocator.h"


// <utils>

static size_t minz(size_t l, size_t r) { return l < r ? l : r; }

// </utils>


// <aux>

struct mem_block { uint32_t size; uint8_t data[]; };

static bool try_extend(
	struct ipj_allocator *,
	struct mem_block *,
	size_t new_size);

// </aux>


void *ipj_alloc(struct ipj_allocator *allocator, size_t size) {
	assert(allocator);

	if (size == 0
		|| allocator->_bump >= allocator->capacity
		|| allocator->_bump + size >= allocator->capacity
	) return NULL;

	struct mem_block *const
		block = (void *)(allocator->arena + allocator->_bump);
	block->size = size;

	allocator->_bump += sizeof(*block) + size;

	return block->data;
}

void *ipj_realloc(
	struct ipj_allocator *allocator,
	void *mem,
	size_t size
) {
	assert(allocator);

	if (mem == NULL)
		return ipj_alloc(allocator, size);

	if ((uint8_t *)mem < allocator->arena
		|| allocator->arena + allocator->capacity <= (uint8_t *)mem
	) return NULL;

	struct mem_block
		*const block = (void *)((uint8_t *)mem - sizeof(*block));

	if (size <= block->size)
		return mem;

	if (try_extend(allocator, block, size))
		return mem;

	void *const new_mem = ipj_alloc(allocator, size);
	if (new_mem && mem)
		memcpy(new_mem, mem, minz(size, block->size));
	return new_mem;
}

// no-op
void ipj_free(struct ipj_allocator *allocator, void *mem) {
	(void)allocator;
	(void)mem;
}


// <aux-impl>

bool try_extend(
	struct ipj_allocator *allocator,
	struct mem_block *block,
	size_t new_size
) {
	assert(allocator);
	assert(block);

	const ptrdiff_t
		block_offset = (uint8_t *)block - allocator->arena,
		supposed_bump = block_offset + sizeof(*block) + block->size;

	const bool success = supposed_bump == allocator->_bump;
	if (success) {
		const ptrdiff_t
			new_bump = allocator->_bump + new_size - block->size;

		if (new_bump >= allocator->capacity)
			return false;

		allocator->_bump = new_bump;
		block->size = new_size;
	}

	return success;
}

// </aux-impl>
