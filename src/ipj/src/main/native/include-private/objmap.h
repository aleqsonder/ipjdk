#ifndef IPJ_OBJMAP_H
#define IPJ_OBJMAP_H

#include "allocator.h"
#include "obj.h"

typedef struct ipj_objmap {
	ipj_allocator_t *allocator;
	struct ipj_objmap_state *state;
} ipj_objmap_t;

extern const size_t IPJ_OBJMAP_STATE_HEAD_SIZE;

struct ipj_objmap_state *ipj_objmap_state_markup(
	size_t bufsize,
	void *buf);

// returns NULL if the object with required id does not exist
// and no memory available to create it
struct ipj_obj *ipj_objmap_provide(
	ipj_objmap_t *,
	const char *id,
	size_t payload_size);

struct ipj_obj *ipj_obj_map_search(
	const ipj_objmap_t *,
	const char *id);

void ipj_obj_map_erase(ipj_objmap_t *, const char *id);

#endif
