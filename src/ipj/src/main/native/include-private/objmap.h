#ifndef IPJ_OBJMAP_H
#define IPJ_OBJMAP_H

#include "allocator.h"
#include "obj.h"

struct ipj_objmap {
	ipj_allocator_t *allocator;
	struct ipj_objmap_state *state;
};
extern const size_t IPJ_OBJMAP_STATE_MIN_SIZE;

struct ipj_objmap_state *ipj_objmap_state_markup(
	size_t bufsize,
	void *buf);

// returns NULL if the object with required id does not exist
// and no memory available to create it
void *ipj_objmap_provide(
	struct ipj_objmap *,
	const char *id,
	size_t payload_size);

void *ipj_obj_map_search(
	const struct ipj_objmap *,
	const char *id);

void ipj_obj_map_erase(struct ipj_objmap *, const char *id);

#endif
