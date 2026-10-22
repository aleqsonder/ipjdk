#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "obj-map.h"


// <aux>

static size_t minz(size_t l, size_t r) { return l < r ? l : r; }
static size_t maxz(size_t l, size_t r) { return l < r ? r : l; }

static size_t djb2(const char *s);

// </aux>


// <hmap>

typedef uint32_t hmap_cell_t;
#define EMPTY_HMAP_CELL UINT32_MAX

#define HMAP_NEW_CELL_COUNT_RATIO 2
#define HMAP_MIN_CELL_COUNT 16

#define hash djb2

// Returns index of a suitable cell.
// Returns cell_count if there is no object
// with given id and no empty cells left.
static size_t hmap_probe(
	size_t cell_count,
	const hmap_cell_t cells[],
	const void *objbuf,
	const char *id);

static size_t hmap_new_size(
	size_t curr_cell_count,
	size_t max_cell_count);

static void hmap_rehash(
	size_t curr_cell_count,
	size_t new_cell_count,
	hmap_cell_t cells[],
	void *objbuf);

// </hmap>


void ipj_obj_map_shared_state_init(
	struct ipj_obj_map_shared_state *state,
	size_t capacity
) {
	assert(state);

	*state = (struct ipj_obj_map_shared_state) {
		.capacity = capacity,
		.refcount = 1,
		._elem_count = 0 };
}

struct ipj_obj *ipj_obj_map_provide(
	struct ipj_obj_map *map,
	const char *id,
	size_t size
) {
	assert(map);
	assert(id);

	if (size == 0)
		return NULL;

	struct ipj_allocator *const allocator = map->allocator;
	struct ipj_obj_map_shared_state *const state = map->shared_state;

	size_t cell_count = state->_elem_count;
	hmap_cell_t *const cells = (hmap_cell_t *)state->arena;
	uint8_t *const objbuf = allocator->arena;

	size_t found = hmap_probe(cell_count, cells, objbuf, id);
	if (found == cell_count) {
		const size_t
			max_cell_count = state->capacity / sizeof(hmap_cell_t),
			new_cell_count = hmap_new_size(cell_count, max_cell_count);

		if (new_cell_count == cell_count)
			return NULL;

		hmap_rehash(cell_count, new_cell_count, cells, objbuf);
		state->_elem_count = cell_count = new_cell_count;
		found = hmap_probe(cell_count, cells, objbuf, id);
	}

	if (found == cell_count)
		return NULL;

	const size_t obj_full_size = ipj_obj_full_size(size, id);
	struct ipj_obj *const new_obj = ipj_alloc(allocator, obj_full_size);
	if (!new_obj)
		return NULL;

	ipj_obj_init(new_obj, size, id);
	cells[found] = (uint8_t *)new_obj - objbuf;
	state->_elem_count++;

	return new_obj;
}

struct ipj_obj *ipj_obj_map_search(
	const struct ipj_obj_map *map,
	const char *id
) {
	assert(map);
	assert(id);

	const struct ipj_obj_map_shared_state *const
	state = map->shared_state;

	const size_t cell_count = state->_elem_count;
	const hmap_cell_t *const cells = (const hmap_cell_t *)state->arena;
	void *const objbuf = map->allocator->arena;

	const size_t found = hmap_probe(cell_count, cells, objbuf, id);

	return found == cell_count || cells[found] == EMPTY_HMAP_CELL
		? NULL
		: (struct ipj_obj *)((uint8_t *)objbuf + cells[found]);
}

void ipj_obj_map_erase(struct ipj_obj_map *map, const char *id) {
	assert(map);
	assert(id);

	struct ipj_obj_map_shared_state *const state = map->shared_state;
	const size_t cell_count = state->_elem_count;
	hmap_cell_t *const cells = (hmap_cell_t *)state->arena;
	struct ipj_allocator *const allocator = map->allocator;
	void *const objbuf = allocator->arena;

	const size_t found = hmap_probe(cell_count, cells, objbuf, id);
	if (found != cell_count && cells[found] != EMPTY_HMAP_CELL) {
		const hmap_cell_t obj_offset = cells[found];
		cells[found] = EMPTY_HMAP_CELL;

		struct ipj_obj *const
		obj = (struct ipj_obj *)((uint8_t *)objbuf + obj_offset);

		ipj_free(allocator, obj);
	}
}


// <hmap-impl>

size_t hmap_probe(
	size_t cell_count,
	const hmap_cell_t cells[],
	const void *objbuf,
	const char *id
) {
	assert(cell_count == 0 || cells);
	assert(objbuf);
	assert(id);

	if (cell_count == 0)
		return 0;

	const size_t h = hash(id);

	for (size_t i = 0; i < cell_count; ++i) {
		const size_t idx = (h + i) % cell_count;
		const hmap_cell_t cell = cells[idx];

		if (cell == EMPTY_HMAP_CELL)
			return idx;

		struct ipj_obj *const
		obj = (struct ipj_obj *)((uint8_t *)objbuf + cell);

		const char *const stored_id = ipj_obj_id(obj);
		const int cmp = strcmp(id, stored_id);
	
		if (cmp == 0)
			return idx;
	}

	return cell_count;
}

size_t hmap_new_size(size_t curr_cell_count, size_t max_cell_count) {
	return maxz(HMAP_MIN_CELL_COUNT, minz(max_cell_count,
			curr_cell_count * HMAP_NEW_CELL_COUNT_RATIO));
}

void hmap_rehash(
	size_t curr_cell_count,
	size_t new_cell_count,
	hmap_cell_t cells[],
	void *objbuf
) {
	assert(curr_cell_count == 0 || cells);
	assert(objbuf);

	if (curr_cell_count == 0) {
		memset(cells, 0xff, new_cell_count * sizeof(*cells));
		return;
	}

	hmap_cell_t old_cells[curr_cell_count];
	memcpy(old_cells, cells, curr_cell_count * sizeof(*cells));
	memset(cells, 0xff, new_cell_count * sizeof(*cells));

	for (size_t i = 0; i < curr_cell_count; ++i) {
		const hmap_cell_t old_cell = old_cells[i];

		struct ipj_obj *const
		obj = (struct ipj_obj *)((uint8_t *)objbuf + old_cell);

		const char *const id = ipj_obj_id(obj);
		const size_t idx = hmap_probe(new_cell_count, cells, objbuf, id);
		assert(idx != new_cell_count);

		cells[idx] = old_cell;
	}
}

// </hmap-impl>


// <aux-impl>

size_t djb2(const char *s) {
	size_t h = 5381;
	int c;
	while ((c = *s++))
		h = ((h << 5) + h) + c; /* hash * 33 + c */
	return h;
}

// </aux-impl>
