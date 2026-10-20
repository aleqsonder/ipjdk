#ifndef IPJ_OBJ_MAP_H
#define IPJ_OBJ_MAP_H

#include "allocator.h"

typedef struct {
	struct 
} ipj_obj_map_t;

void ipj_obj_map_init(ipj_obj_map_t *, 
void ipj_obj_map_add_ref(ipj_obj_map_t *);

// returns non-zero on refcount==0
int ipj_obj_map_release(ipj_obj_map_t *);

// returns size of the object found
// returns 0 on fail
size_t ipj_obj_map_search(
	const ipj_obj_map_t *,
	const char *id,
	void **obj);

// returns size of the object provided
// returns 0 on fail
size_t ipj_obj_map_provide(
	const ipj_obj_map_t *,
	const char *id,
	void **obj,
	ipj_allocator_t *,
	size_t size);

void ipj_obj_add_ref(void *obj);
void ipj_obj_release(void *obj);

#endif
