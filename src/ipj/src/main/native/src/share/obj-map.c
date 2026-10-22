#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "obj-map.h"


// <avl>

struct avl_node { uint32_t obj_offset, subheight; };

static struct ipj_obj *avl_search(
	size_t node_count,
	const struct avl_node nodes[],
	void *objbuf,
	const char *id);

static void avl_insert(
	size_t node_count,
	struct avl_node nodes[],
	void *objbuf,
	struct ipj_obj *new_obj);

// </avl>


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

	const size_t node_count = state->_elem_count;
	struct avl_node *const nodes = (struct avl_node *)state->arena;
	uint8_t *const objbuf = allocator->arena;

	struct ipj_obj *const
	found = avl_search(node_count, nodes, objbuf, id);

	if (found)
		return found;

	const size_t
	elem_capacity = state->capacity / sizeof(struct avl_node);

	if (node_count >= elem_capacity)
		return NULL;

	const size_t obj_full_size = ipj_obj_full_size(size, id);
	struct ipj_obj *const new_obj = ipj_alloc(allocator, obj_full_size);
	if (!new_obj)
		return NULL;

	ipj_obj_init(new_obj, size, id);
	avl_insert(node_count, nodes, objbuf, new_obj);

	return new_obj;
}


// <avl-impl>

struct ipj_obj *avl_search(
	size_t node_count,
	const struct avl_node nodes[],
	void *objbuf,
	const char *id
) {
	assert(node_count == 0 || nodes);
	assert(objbuf);
	assert(id);

	for (size_t i = 0; i < node_count;) {
		const struct avl_node node = nodes[i];

		struct ipj_obj *const
		obj = (struct ipj_obj *)((uint8_t *)objbuf + node.obj_offset);

		const char *const stored_id = ipj_obj_id(obj);
		const int cmp = strcmp(id, stored_id);
	
		if (cmp == 0)
			return obj;

		i = (i+1) * 2 - (cmp < 0);
		/* explanation:

		// if i was 1-indexed then:
		if (cmp < 0) i *= 2;
		else i = i * 2 + 1;

		// but i is actually 0-indexed, so:
		if (cmp < 0) i = (i+1) * 2 - 1;
		else i = (i+1) * 2;
		--or--
		i = (i+1) * 2 - (cmp < 0 ? 1 : 0);

		// and (cmp < 0 ? 1 : 0) == (cmp < 0)
		*/
	}

	return NULL;
}

void avl_insert(
	size_t node_count,
	struct avl_node nodes[],
	void *objbuf,
	struct ipj_obj *new_obj
) {
	assert(node_count == 0 || nodes);
	assert(objbuf);
	assert(new_obj);

	
}

// </avl-impl>
