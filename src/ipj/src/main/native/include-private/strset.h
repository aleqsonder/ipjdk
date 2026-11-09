#ifndef IPJ_STRSET_H
#define IPJ_STRSET_H

#include <stdbool.h>
#include <sys/param.h>

#include "allocator.h"

struct ipj_strset_cell { uint32_t key_offset; };
struct ipj_strset { uint32_t count; struct ipj_strset_cell cells[]; };

#define IPJ_STRSET_CELL_COUNT(BUFSIZE) ( \
	((BUFSIZE) - sizeof(struct ipj_strset)) \
		/ sizeof(struct ipj_strset_cell))

#define IPJ_STRSET_INITIAL(CELL_COUNT) ( \
	(struct ipj_strset) { .count = (CELL_COUNT) })

// returns -1 if the object
// with required key does not exist
// and there is no memory available to create it
// within the `allocator`
ssize_t ipj_strset_provide(
	struct ipj_strset *,
	struct ipj_allocator *allocator,
	const char *key,
	
	// [out, nullable]
	bool *is_new);

ssize_t ipj_strset_search(
	const struct ipj_strset *,
	const struct ipj_allocator *,
	const char *key);

// no-op for `handle==-1`
void ipj_strset_erase(
	struct ipj_strset *,
	struct ipj_allocator *,
	ssize_t index);

#endif
