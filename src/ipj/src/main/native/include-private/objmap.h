#ifndef IPJ_OBJMAP_H
#define IPJ_OBJMAP_H

#include <stdbool.h>

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
ipj_obj_t *ipj_objmap_provide(
	struct ipj_objmap *,
	const char *id,
	size_t payload_size,
	
	// [out, nullable]
	// Is being set to non-zero after the
	// function returns if the returned
	// object has not existed before.
	bool *is_new);

ipj_obj_t *ipj_objmap_search(const struct ipj_objmap *, const char *id);
void ipj_objmap_erase(struct ipj_objmap *, const char *id);

#endif
