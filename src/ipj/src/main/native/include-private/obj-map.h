#ifndef IPJ_OBJ_MAP_H
#define IPJ_OBJ_MAP_H

#include "allocator.h"
#include "obj.h"

struct ipj_obj_map {
	struct ipj_allocator *allocator;
	struct ipj_obj_map_shared_state {
		uint32_t capacity, refcount, _elem_count;
		uint8_t arena[];
	} *shared_state;
};

void ipj_obj_map_shared_state_init(
	struct ipj_obj_map_shared_state *,
	size_t capacity);

// returns NULL if size==0 or if the object with
// required id does not exist and no memory available
// to create it
struct ipj_obj *ipj_obj_map_provide(
	struct ipj_obj_map *,
	const char *id,
	size_t size);

void ipj_obj_map_erase(struct ipj_obj_map *, const char *id);

#endif
