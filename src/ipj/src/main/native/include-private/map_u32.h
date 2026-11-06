#ifndef IPJ_MAP_U32_H
#define IPJ_MAP_U32_H

#include <stdbool.h>
#include "allocator.h"

struct ipj_map_u32_cell { uint32_t key_offset, value; };

struct ipj_map_u32_cell_array {
	uint32_t cell_count, elem_count;
	struct ipj_map_u32_cell cells[];
};

struct ipj_map_u32 {
	struct ipj_allocator *allocator;
	struct ipj_map_u32_cell_array *cell_arr;
};

bool ipj_map_u32_init(
	struct ipj_map_u32 *,
	struct ipj_allocator *,
	size_t cell_count);

void ipj_map_u32_destroy(struct ipj_map_u32 *);

// returns NULL if the object with required key does not exist
// and there is no capacity available to create it
uint32_t *ipj_map_u32_provide(
	struct ipj_map_u32 *,
	const char *key,
	
	// [out, nullable]
	bool *is_new);

uint32_t *ipj_map_u32_search(const struct ipj_map_u32 *, const char *key);
void ipj_map_u32_erase(struct ipj_map_u32 *, const char *key);
bool ipj_map_u32_rehash(struct ipj_map_u32 *, size_t new_cell_count);

#endif
