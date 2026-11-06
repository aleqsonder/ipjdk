#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#include "map_u32.h"


// <utils>

static size_t maxz(size_t l, size_t r) { return l < r ? r : l; }
static size_t djb2(const char *s);

// </utils>


// <aux>

#define NOT_FOUND -1
#define INVALID_OFFSET 0

static ssize_t probe(
	const struct ipj_map_u32 *map,
	const char *key);

#define get_base(MAP_PTR) ((uint8_t *)((MAP_PTR)->allocator))

#define get_offset_min(MAP_PTR) (sizeof(*(MAP_PTR)->allocator))
#define get_offset_max(MAP_PTR) \
	(get_offset_min(MAP_PTR) + (MAP_PTR)->allocator->capacity)
#define offset_is_in_range(MAP_PTR, OFFSET) \
	(get_offset_min(MAP_PTR) <= (OFFSET) && (OFFSET) < get_offset_max(MAP_PTR))

static uint32_t get_offset(
	const struct ipj_map_u32 *map,
	const void *ptr);

static void *resolve_offset(
	const struct ipj_map_u32 *map,
	uint32_t offset);

static void export_elems(
	const struct ipj_map_u32 *map,
	struct ipj_map_u32_cell *dest);

static void replace_elems(
	struct ipj_map_u32 *map,
	size_t new_elem_count,
	const struct ipj_map_u32_cell *src);

// </aux>


bool ipj_map_u32_init(
	struct ipj_map_u32 *map,
	struct ipj_allocator *allocator,
	size_t cell_count
) {
	assert(map);
	assert(allocator);

	const size_t
		cells_size = cell_count * sizeof(*map->cell_arr->cells),
		cell_arr_size = sizeof(*map->cell_arr) + cells_size;

	struct ipj_map_u32_cell_array
		*const cell_arr = ipj_alloc(allocator, cell_arr_size);

	if (!cell_arr)
		return false;

	*cell_arr = (struct ipj_map_u32_cell_array) {
		.cell_count = cell_count,
		.elem_count = 0 };
	memset(cell_arr->cells, 0, cells_size);

	*map = (struct ipj_map_u32) {
		.allocator = allocator,
		.cell_arr = cell_arr };

	return true;
}

void ipj_map_u32_destroy(struct ipj_map_u32 *map) {
	assert(map);
	ipj_free(map->allocator, map->cell_arr);
}

uint32_t *ipj_map_u32_provide(
	struct ipj_map_u32 *map,
	const char *key,
	bool *is_new
) {
	assert(map);
	assert(key);

	ssize_t found = probe(map, key);
	if (found == NOT_FOUND) {
		const size_t new_cell_count = map->cell_arr->cell_count * 2;

		if (!ipj_map_u32_rehash(map, new_cell_count))
			return NULL;

		found = probe(map, key);
		assert(found != NOT_FOUND);
	}

	struct ipj_map_u32_cell *const cell = &map->cell_arr->cells[found];
	const bool is_new_local = cell->key_offset == INVALID_OFFSET;

	if (is_new)
		*is_new = is_new_local;

	if (is_new_local) {
		char *key_stored = ipj_alloc(map->allocator, strlen(key) + 1);
		if (!key_stored)
			return NULL;

		strcpy(key_stored, key);
		cell->key_offset = get_offset(map, key_stored);

		map->cell_arr->elem_count++;
	}

	return &cell->value;
}

uint32_t *ipj_map_u32_search(
	const struct ipj_map_u32 *map,
	const char *key
) {
	assert(map);
	assert(key);

	const ssize_t found = probe(map, key);

	if (found == NOT_FOUND)
		return NULL;

	struct ipj_map_u32_cell *const cell = &map->cell_arr->cells[found];
	return cell->key_offset == INVALID_OFFSET ? NULL : &cell->value;
}

void ipj_map_u32_erase(struct ipj_map_u32 *map, const char *key) {
	assert(map);
	assert(key);

	const ssize_t found = probe(map, key);

	if (found == NOT_FOUND)
		return;

	struct ipj_map_u32_cell *const cell = &map->cell_arr->cells[found];
	char *const key_stored = resolve_offset(map, cell->key_offset);
	ipj_free(map->allocator, key_stored);

	cell->key_offset = INVALID_OFFSET;
	map->cell_arr->elem_count--;
}

bool ipj_map_u32_rehash(
	struct ipj_map_u32 *map,
	size_t new_cell_count
) {
	assert(map);

	struct ipj_map_u32_cell_array *const old_cell_arr = map->cell_arr;
	const size_t elem_count = old_cell_arr->elem_count;

	struct ipj_map_u32_cell elems[maxz(1, elem_count)];
	export_elems(map, elems);

	const size_t new_cell_arr_size
		= sizeof(struct ipj_map_u32_cell_array)
		+ new_cell_count * sizeof(struct ipj_map_u32_cell);

	struct ipj_map_u32_cell_array *const new_cell_arr
		= old_cell_arr->cell_count == 0
			? ipj_alloc(map->allocator, new_cell_arr_size)
			: ipj_realloc(map->allocator, old_cell_arr, new_cell_arr_size);

	if (!new_cell_arr)
		return false;

	*new_cell_arr = (struct ipj_map_u32_cell_array) {
		.cell_count = new_cell_count,
		.elem_count = elem_count };
	map->cell_arr = new_cell_arr;

	replace_elems(map, elem_count, elems);

	return true;
}


// <aux-impl>

uint32_t get_offset(
	const struct ipj_map_u32 *map,
	const void *ptr
) {
	assert(map);
	assert(map->allocator);

	const void *const base = get_base(map);

	if (!ptr || ptr <= base)
		return INVALID_OFFSET;

	const ptrdiff_t offset = (uint8_t *)ptr - (uint8_t *)base;
	return offset_is_in_range(map, offset) ? offset : INVALID_OFFSET;
}

void *resolve_offset(
	const struct ipj_map_u32 *map,
	uint32_t offset
) {
	assert(map);
	assert(map->allocator);

	return offset != INVALID_OFFSET && offset_is_in_range(map, offset)
		? get_base(map) + offset
		: INVALID_OFFSET;
}

ssize_t probe(const struct ipj_map_u32 *map, const char *key) {
	assert(map);
	assert(map->cell_arr);
	assert(key);

	const size_t
		cell_count = map->cell_arr->cell_count,
		h = djb2(key);

	for (size_t i = 0; i < cell_count; ++i) {
		const size_t idx = (h + i) % cell_count;

		const struct ipj_map_u32_cell
			*const cell = &map->cell_arr->cells[idx];

		if (cell->key_offset == INVALID_OFFSET)
			return idx;

		const char
			*const key_stored = resolve_offset(map, cell->key_offset);
		assert(key_stored);

		if (strcmp(key, key_stored) == 0)
			return idx;
	}

	return NOT_FOUND;
}

void export_elems(
	const struct ipj_map_u32 *map,
	struct ipj_map_u32_cell *dest
) {
	assert(map);
	assert(map->cell_arr);
	assert(map->cell_arr->elem_count == 0 || dest);

	for (size_t i = 0, dest_idx = 0; i < map->cell_arr->cell_count; ++i) {
		struct ipj_map_u32_cell *const cell = &map->cell_arr->cells[i];
		if (cell->key_offset != INVALID_OFFSET)
			dest[dest_idx++] = *cell;
	}
}

void replace_elems(
	struct ipj_map_u32 *map,
	size_t new_elem_count,
	const struct ipj_map_u32_cell *src
) {
	assert(map);
	assert(map->cell_arr);
	assert(new_elem_count == 0 || src);

	const size_t cells_size
		= map->cell_arr->cell_count
		* sizeof(*map->cell_arr->cells);

	memset(map->cell_arr->cells, 0, cells_size);
	
	for (size_t i = 0; i < new_elem_count; ++i) {
		struct ipj_map_u32_cell *const cell = &map->cell_arr->cells[i];
		const char *const key = resolve_offset(map, cell->key_offset);
		const ssize_t found = probe(map, key);
		assert(found != NOT_FOUND);
		map->cell_arr->cells[found] = *cell;
	}

	map->cell_arr->elem_count = new_elem_count;
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
