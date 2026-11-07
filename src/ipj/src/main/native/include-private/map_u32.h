#ifndef IPJ_MAP_U32_H
#define IPJ_MAP_U32_H

#include <stdbool.h>
#include "allocator.h"

struct ipj_map_u32_cell { uint32_t key_offset, value; };

struct ipj_map_u32 {
	uint32_t cell_count, elem_count;
	struct ipj_map_u32_cell cells[];
};

#define IPJ_MAP_U32_INITIAL(CELL_COUNT) ((struct ipj_map_u32) { \
	.cell_count = (CELL_COUNT), .elem_count = 0 })

// returns NULL if the object with required key does not exist
// and there is no capacity available to create it
uint32_t *ipj_map_u32_provide(
	struct ipj_map_u32 *,
	struct ipj_allocator *,
	const char *key,
	
	// [out, nullable]
	bool *is_new);

uint32_t *ipj_map_u32_search(
	struct ipj_map_u32 *,
	const struct ipj_allocator *,
	const char *key);

void ipj_map_u32_erase(
	struct ipj_map_u32 *,
	struct ipj_allocator *,
	const char *key);

#endif
