#ifndef IPJ_STRSET_H
#define IPJ_STRSET_H

#include <stdbool.h>
#include <sys/param.h>

#include "allocator.h"

struct ipj_strset_cell { uint32_t key_offset; };

struct ipj_strset_cell_array {
	uint32_t count;
	struct ipj_strset_cell cells[];
};

struct ipj_strset {
	struct ipj_allocator *allocator;
	struct ipj_strset_cell_array *cell_array;
};

#define IPJ_STRSET_CELL_COUNT(BUFSIZE) ( \
	((BUFSIZE) - sizeof(struct ipj_strset_cell_array)) \
		/ sizeof(struct ipj_strset_cell))

#define IPJ_STRSET_CELL_ARRAY_INITIAL(CELL_COUNT) ( \
	(struct ipj_strset_cell_array) { .count = (CELL_COUNT) })

// returns -1 if the object
// with required key does not exist
// and there is no memory available to create it
// within the `set->allocator`
ssize_t ipj_strset_provide(
	struct ipj_strset *set,
	const char *key,
	
	// [out, nullable]
	bool *is_new);

ssize_t ipj_strset_search(struct ipj_strset *, const char *key);

// no-op for `handle==-1`
void ipj_strset_erase(struct ipj_strset *, ssize_t index);

#endif
