#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "shmem.h"
#include "shmmap.h"
#include "objmap.h"

// <settings>

#ifndef IPJ_OBJMAP_SIZE
	// 1MB
	#define IPJ_OBJMAP_SIZE 1<<20
#endif

#ifndef IPJ_OBJMAP_FNAME
	#define IPJ_OBJMAP_FNAME "ipj-objmap"
#endif

#ifndef IPJ_SHBUF_SIZE
	// 4MB
	#define IPJ_SHBUF_SIZE 4<<20
#endif

#ifndef IPJ_SHBUF_FNAME
	#define IPJ_SHBUF_FNAME "ipj-shbuf"
#endif

// </settings>


// <globals>

static ipj_allocator_t *allocator = NULL;
static struct ipj_objmap objmap;

static ipj_shm_t *mapbuf = NULL, *arena = NULL;

// </globals>


// <aux>

static int allocator_init(void);
static void allocator_destroy(void);

static int objmap_init(void);
static void objmap_destroy(void);

// </aux>


int ipj_shm_init(void) { return objmap_init(); }
void ipj_shm_cleanup(void) { objmap_destroy(); }

void *ipj_shalloc(const char *name, size_t *size, bool *is_new) {
	assert(name);

	const size_t size_local = size ? *size : 0;

	ipj_obj_t *const
	obj = ipj_objmap_provide(&objmap, name, size_local, is_new);

	if (obj && size)
		*size = ipj_obj_payload_size(obj);

	return obj;
}

void ipj_shfree(void *shm) {
	if (shm != NULL) {
		ipj_obj_t *const obj = shm;
		const char *const id = ipj_obj_id_const(obj);
		ipj_objmap_erase(&objmap, id);
	}
}


// <aux-impl>

int objmap_init(void) {
	int status = allocator_init();
	if (status)
		return status;

	bool is_new = false;
	mapbuf = ipj_shm_open(IPJ_OBJMAP_FNAME, IPJ_OBJMAP_SIZE, &is_new);
	if (mapbuf == NULL) {
		status = 1;
		goto final;
	}

	void *const shm_data = ipj_shm_data(mapbuf);
	struct ipj_objmap_state *const state = is_new
		? ipj_objmap_state_markup(IPJ_OBJMAP_SIZE, shm_data)
		: ipj_objmap_state_checkout(shm_data);
	if (state == NULL) {
		status = 1;
		goto shm_cleanup;
	}

	objmap = (struct ipj_objmap) {
		.allocator = allocator,
		.state = state };

	goto final;

shm_cleanup:
	ipj_shm_close(mapbuf);

final:
	return status;
}

int allocator_init(void) {
	int status = 0;

	bool is_new = false;
	arena = ipj_shm_open(IPJ_SHBUF_FNAME, IPJ_SHBUF_SIZE, &is_new);
	if (arena == NULL) {
		status = 1;
		goto final;
	}

	void *const shm_data = ipj_shm_data(arena);
	allocator = is_new
		? ipj_allocator_markup(IPJ_SHBUF_SIZE, shm_data)
		: ipj_allocator_checkout(shm_data);
	if (allocator == NULL) {
		status = 1;
		goto shm_cleanup;
	}

	goto final;

shm_cleanup:
	ipj_shm_close(arena);

final:
	return status;
}

void objmap_destroy(void) {
	ipj_shm_close(mapbuf);
	allocator_destroy();
}

void allocator_destroy(void) {
	ipj_shm_close(arena);
}

// </aux-impl>
