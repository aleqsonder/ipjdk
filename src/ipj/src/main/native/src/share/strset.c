#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#include "strset.h"
#include "allocator.h"


// <utils>

static size_t djb2(const char *s);

// </utils>


// <aux>

#define NOT_FOUND (-1)
#define INVALID_OFFSET 0

static ssize_t probe(const struct ipj_strset *set, const char *key);

#define offset_is_in_range(ALLOCATOR, OFFSET) \
	(sizeof(struct ipj_allocator) <= (OFFSET) \
		&& (OFFSET) < sizeof(struct ipj_allocator) + (ALLOCATOR)->capacity)

static uint32_t get_offset(
	const struct ipj_allocator *,
	const void *ptr);

static void *resolve_offset(
	const struct ipj_allocator *,
	uint32_t offset);

// </aux>


ssize_t ipj_strset_provide(
	struct ipj_strset *set,
	const char *key,
	bool *is_new
) {
	assert(set);
	assert(key);

	const ssize_t index = probe(set, key);
	if (index != NOT_FOUND) {
		struct ipj_strset_cell
			*const cell = &set->cell_array->cells[index];

		const bool is_new_local = cell->key_offset == INVALID_OFFSET;

		if (is_new_local) {
			char *const key_stored = ipj_alloc(
				set->allocator, strlen(key) + 1);

			if (!key_stored)
				return NOT_FOUND;

			strcpy(key_stored, key);
			cell->key_offset = get_offset(set->allocator, key_stored);
		}

		if (is_new)
			*is_new = is_new_local;
	}

	return index;
}

ssize_t ipj_strset_search(struct ipj_strset *set, const char *key) {
	assert(set);
	assert(key);

	const ssize_t index = probe(set, key);

	if (index == NOT_FOUND)
		return NOT_FOUND;

	struct ipj_strset_cell *const cell = &set->cell_array->cells[index];
	return cell->key_offset == INVALID_OFFSET ? NOT_FOUND : index;
}

void ipj_strset_erase(struct ipj_strset *set, ssize_t index) {
	assert(set);
	
	if (index == NOT_FOUND)
		return;

	struct ipj_strset_cell *const cell = &set->cell_array->cells[index];
	char *const key_stored = resolve_offset(
		set->allocator, cell->key_offset);
	ipj_free(set->allocator, key_stored);

	cell->key_offset = INVALID_OFFSET;
}


// <aux-impl>

uint32_t get_offset(
	const struct ipj_allocator *allocator,
	const void *ptr
) {
	assert(allocator);

	const void *const base = allocator;

	if (!ptr || ptr <= base)
		return INVALID_OFFSET;

	const ptrdiff_t offset = (uint8_t *)ptr - (uint8_t *)base;
	return offset_is_in_range(allocator, offset) ? offset : INVALID_OFFSET;
}

void *resolve_offset(
	const struct ipj_allocator *allocator,
	uint32_t offset
) {
	assert(allocator);

	return offset != INVALID_OFFSET && offset_is_in_range(allocator, offset)
		? (uint8_t *)allocator + offset
		: INVALID_OFFSET;
}

ssize_t probe(const struct ipj_strset *set, const char *key) {
	assert(set);
	assert(key);

	const size_t
		cell_count = set->cell_array->count,
		h = djb2(key);

	for (size_t i = 0; i < cell_count; ++i) {
		const size_t index = (h + i) % cell_count;

		const struct ipj_strset_cell
			*const cell = &set->cell_array->cells[index];

		if (cell->key_offset == INVALID_OFFSET)
			return index;

		const char *const key_stored = resolve_offset(
			set->allocator, cell->key_offset);
		assert(key_stored);

		if (strcmp(key, key_stored) == 0)
			return index;
	}

	return NOT_FOUND;
}

// </aux-impl>


// <utils-impl>

size_t djb2(const char *s) {
	size_t h = 5381;
	int c;
	while ((c = *s++))
		h = ((h << 5) + h) + c; /* hash * 33 + c */
	return h;
}

// </utils-impl>
