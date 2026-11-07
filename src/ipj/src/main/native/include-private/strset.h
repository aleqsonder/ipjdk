#ifndef IPJ_STRSET_H
#define IPJ_STRSET_H

#include <stdbool.h>
#include <sys/param.h>

#include "allocator.h"

typedef uint32_t ipj_strset_cell_t;
#define IPJ_STRSET_CELL_INVALID 0

struct ipj_strset_cell_array {
	uint32_t cell_count, elem_count;
	ipj_strset_cell_t cells[];
};

#define IPJ_STRSET_CELL_ARRAY_INITIAL(BUFSIZE) \
	((struct ipj_strset_cell_array) { \
		.cell_count \
			= ((BUFSIZE) - sizeof(struct ipj_strset_cell_array)) \
			/ sizeof(struct ipj_strset_cell), \
		.elem_count = 0 })

struct ipj_strset {
	struct ipj_allocator *allocator;
	struct ipj_strset_cell_array *cell_arr;
};

typedef ssize_t ipj_strset_cell_index_t;
#define IPJ_STRSET_CELL_INDEX_INVALID -1

// returns IPJ_STRSET_CELL_INDEX_INVALID if the object
// with required key does not exist
// and there is no memory available to create it
// within the `set->allocator`
ipj_strset_cell_index_t ipj_strset_provide(
	struct ipj_strset *set,
	const char *key,
	
	// [out, nullable]
	bool *is_new);

ipj_strset_cell_index_t ipj_strset_search(
	struct ipj_strset *,
	const char *key);

void ipj_strset_erase(struct ipj_strset *, ipj_strset_cell_index_t);

#endif
