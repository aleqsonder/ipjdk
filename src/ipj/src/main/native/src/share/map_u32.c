#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#include "map_u32.h"
#include "allocator.h"


// <utils>

static size_t djb2(const char *s);

// </utils>


// <aux>

#define NOT_FOUND -1
#define INVALID_OFFSET 0

static ssize_t probe(
	const struct ipj_map_u32 *map,
	const struct ipj_allocator *allocator,
	const char *key);

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


uint32_t *ipj_map_u32_provide(
	struct ipj_map_u32 *map,
	struct ipj_allocator *allocator,
	const char *key,
	bool *is_new
) {
	assert(map);
	assert(key);

	ssize_t found = probe(map, allocator, key);
	if (found == NOT_FOUND)
		return NULL;

	struct ipj_map_u32_cell *const cell = &map->cells[found];
	const bool is_new_local = cell->key_offset == INVALID_OFFSET;

	if (is_new)
		*is_new = is_new_local;

	if (is_new_local) {
		char *key_stored = ipj_alloc(allocator, strlen(key) + 1);
		if (!key_stored)
			return NULL;

		strcpy(key_stored, key);
		cell->key_offset = get_offset(allocator, key_stored);

		map->elem_count++;
	}

	return &cell->value;
}

uint32_t *ipj_map_u32_search(
	struct ipj_map_u32 *map,
	const struct ipj_allocator *allocator,
	const char *key
) {
	assert(map);
	assert(key);

	const ssize_t found = probe(map, allocator, key);

	if (found == NOT_FOUND)
		return NULL;

	struct ipj_map_u32_cell *const cell = &map->cells[found];
	return cell->key_offset == INVALID_OFFSET ? NULL : &cell->value;
}

void ipj_map_u32_erase(
	struct ipj_map_u32 *map,
	struct ipj_allocator *allocator,
	const char *key) {
	assert(map);
	assert(key);

	const ssize_t found = probe(map, allocator, key);

	if (found == NOT_FOUND)
		return;

	struct ipj_map_u32_cell *const cell = &map->cells[found];
	char *const key_stored = resolve_offset(allocator, cell->key_offset);
	ipj_free(allocator, key_stored);

	cell->key_offset = INVALID_OFFSET;
	map->elem_count--;
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

ssize_t probe(
	const struct ipj_map_u32 *map,
	const struct ipj_allocator *allocator,
	const char *key
) {
	assert(map);
	assert(key);

	const size_t
		cell_count = map->cell_count,
		h = djb2(key);

	for (size_t i = 0; i < cell_count; ++i) {
		const size_t idx = (h + i) % cell_count;

		const struct ipj_map_u32_cell
			*const cell = &map->cells[idx];

		if (cell->key_offset == INVALID_OFFSET)
			return idx;

		const char *const key_stored = resolve_offset(
			allocator, cell->key_offset);
		assert(key_stored);

		if (strcmp(key, key_stored) == 0)
			return idx;
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
