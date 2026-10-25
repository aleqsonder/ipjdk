#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "objmap.h"


// <aux>

static size_t minz(size_t l, size_t r) { return l < r ? l : r; }
static size_t maxz(size_t l, size_t r) { return l < r ? r : l; }

static size_t djb2(const char *s);

// </aux>


// <hmap>

typedef uint32_t hmap_cell_t;
#define HMAP_EMPTY_CELL UINT32_MAX

#define HMAP_NEW_CELL_COUNT_RATIO 2
#define HMAP_MIN_CELL_COUNT 16

#define hash djb2

// Returns index of a suitable cell.
// Returns cell_count if there is no object
// with given id and no empty cells left.
static size_t hmap_probe(
	size_t cell_count,
	const hmap_cell_t cells[],
	const char *id,
	const ipj_allocator_t *);

static size_t hmap_new_size(
	size_t curr_cell_count,
	size_t cell_count_limit);

static void hmap_rehash(
	size_t curr_cell_count,
	size_t new_cell_count,
	hmap_cell_t cells[],
	const ipj_allocator_t *allocator);

// </hmap>

struct ipj_objmap_state {
	uint32_t cell_count, cell_count_limit;
	hmap_cell_t cells[];
};
const size_t IPJ_OBJMAP_STATE_MIN_SIZE = sizeof(struct ipj_objmap_state);

struct ipj_objmap_state *ipj_objmap_state_markup(
	size_t bufsize,
	void *buf
) {
	assert(bufsize == 0 || buf);

	if (bufsize < IPJ_OBJMAP_STATE_MIN_SIZE)
		return NULL;

	struct ipj_objmap_state *const state = buf;
	*state = (struct ipj_objmap_state) {
		.cell_count = HMAP_MIN_CELL_COUNT,
		.cell_count_limit
			= (bufsize - IPJ_OBJMAP_STATE_MIN_SIZE)
			/ sizeof(hmap_cell_t)
	};

	for (size_t i = 0; i < HMAP_MIN_CELL_COUNT; ++i)
		state->cells[i] = HMAP_EMPTY_CELL;

	return state;
}

void *ipj_objmap_state_unmarkup(struct ipj_objmap_state *state) {
	return state;
}

ipj_obj_t *ipj_objmap_provide(
	struct ipj_objmap *map,
	const char *id,
	size_t payload_size,
	bool *is_new
) {
	assert(map);
	assert(id);

	size_t cell_count = map->state->cell_count;
	hmap_cell_t *const cells = map->state->cells;

	size_t found = hmap_probe(cell_count, cells, id, map->allocator);
	if (found == cell_count) {
		const size_t new_cell_count = hmap_new_size(
				cell_count, map->state->cell_count_limit);

		if (new_cell_count == cell_count)
			return NULL;

		hmap_rehash(cell_count, new_cell_count, cells, map->allocator);
		map->state->cell_count = cell_count = new_cell_count;
		found = hmap_probe(cell_count, cells, id, map->allocator);
	}

	if (found == cell_count)
		return NULL;

	if (is_new)
		*is_new = cells[found] == HMAP_EMPTY_CELL;

	if (cells[found] != HMAP_EMPTY_CELL)
		return ipj_offset_resolve(map->allocator, cells[found]);

	const size_t obj_size = ipj_obj_size(id, payload_size);
	void *const objbuf = ipj_alloc(map->allocator, obj_size);
	if (!objbuf)
		return NULL;

	ipj_obj_t *const new_obj = ipj_obj_markup(objbuf, id, payload_size);
	cells[found] = ipj_mem_offset(map->allocator, new_obj);

	return new_obj;
}

ipj_obj_t *ipj_objmap_search(
	const struct ipj_objmap *map,
	const char *id
) {
	assert(map);
	assert(id);

	const size_t cell_count = map->state->cell_count;
	const hmap_cell_t *const cells = map->state->cells;

	const size_t found = hmap_probe(cell_count, cells, id, map->allocator);

	return found == cell_count || cells[found] == HMAP_EMPTY_CELL
		? NULL
		: ipj_offset_resolve(map->allocator, cells[found]);
}

void ipj_objmap_erase(struct ipj_objmap *map, const char *id) {
	assert(map);
	assert(id);

	const size_t cell_count = map->state->cell_count;
	hmap_cell_t *const cells = map->state->cells;

	const size_t found = hmap_probe(cell_count, cells, id, map->allocator);
	if (found != cell_count && cells[found] != HMAP_EMPTY_CELL) {
		struct ipj_obj *const
		obj = ipj_offset_resolve(map->allocator, cells[found]);

		cells[found] = HMAP_EMPTY_CELL;
		ipj_free(map->allocator, ipj_obj_unmarkup(obj));
	}
}


// <hmap-impl>

size_t hmap_probe(
	size_t cell_count,
	const hmap_cell_t cells[],
	const char *id,
	const ipj_allocator_t *allocator
) {
	assert(cell_count == 0 || cells);
	assert(id);
	assert(allocator);

	if (cell_count == 0)
		return 0;

	const size_t h = hash(id);

	for (size_t i = 0; i < cell_count; ++i) {
		const size_t idx = (h + i) % cell_count;
		const hmap_cell_t cell = cells[idx];

		if (cell == HMAP_EMPTY_CELL)
			return idx;

		const struct ipj_obj *const
		obj = ipj_offset_resolve_const(allocator, cell);
		assert(obj);

		const char *const stored_id = ipj_obj_id_const(obj);
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
	const ipj_allocator_t *allocator
) {
	assert(curr_cell_count == 0 || cells);
	assert(allocator);

	if (curr_cell_count == 0) {
		memset(cells, 0xff, new_cell_count * sizeof(*cells));
		return;
	}

	hmap_cell_t old_cells[curr_cell_count];
	memcpy(old_cells, cells, curr_cell_count * sizeof(*cells));
	
	for (size_t i = 0; i < new_cell_count; ++i)
		cells[i] = HMAP_EMPTY_CELL;

	for (size_t i = 0; i < curr_cell_count; ++i) {
		const hmap_cell_t old_cell = old_cells[i];

		const struct ipj_obj *const
		obj = ipj_offset_resolve_const(allocator, old_cell);

		const char *const id = ipj_obj_id_const(obj);
		const size_t idx = hmap_probe(new_cell_count, cells, id, allocator);
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
