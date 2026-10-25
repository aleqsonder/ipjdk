#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "shmem.h"
#include "objmap.h"

// <settings>

#ifndef IPJ_OBJMAP_SIZE
	// 1MB
	#define IPJ_OBJMAP_SIZE 1<<20
#endif

#ifndef IPJ_OBJMAP_FNAME
	#define IPJ_OBJMAP_FNAME TEXT("ipj-objmap")
#endif

#ifndef IPJ_SHBUF_SIZE
	// 4MB
	#define IPJ_SHBUF_SIZE 4<<20
#endif

#ifndef IPJ_SHBUF_FNAME
	#define IPJ_SHBUF_FNAME TEXT("ipj-shbuf")
#endif

// </settings>


// <globals>

static ipj_allocator_t *allocator = NULL;
static struct ipj_objmap objmap;

// </globals>


// <aux>

static int allocator_init(void);
static void allocator_destroy(void);

static int objmap_init(void);
static void objmap_destroy(void);

static LPVOID shmopen(LPCSTR name, SIZE_T size);
static void shmclose(LPVOID shm);

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

	const LPVOID shm = shmopen(IPJ_OBJMAP_FNAME, IPJ_OBJMAP_SIZE);
	if (shm == NULL) {
		status = 1;
		goto final;
	}

	struct ipj_objmap_state *const
	state = ipj_objmap_state_markup(IPJ_OBJMAP_SIZE, shm);
	if (state == NULL) {
		status = 1;
		goto shm_cleanup;
	}

	objmap = (struct ipj_objmap) {
		.allocator = allocator,
		.state = state };

	goto final;

shm_cleanup:
	shmclose(shm);

final:
	return status;
}

int allocator_init(void) {
	int status = 0;

	const LPVOID shm = shmopen(IPJ_SHBUF_FNAME, IPJ_SHBUF_SIZE);
	if (shm == NULL) {
		status = 1;
		goto final;
	}

	allocator = ipj_allocator_markup(IPJ_SHBUF_SIZE, shm);
	if (allocator == NULL) {
		status = 1;
		goto shm_cleanup;
	}

	goto final;

shm_cleanup:
	shmclose(shm);

final:
	return status;
}

LPVOID shmopen(LPCSTR name, SIZE_T size) {
	const DWORD
		size_hi = size >> (8 * sizeof(DWORD)),
		size_lo = size & 0xffffffff;

	const HANDLE fmap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		size_hi,
		size_lo,
		name);
	if (fmap == NULL)
		return NULL;

	const LPVOID mem = MapViewOfFile(fmap, FILE_MAP_WRITE, 0, 0, 0);

	CloseHandle(fmap);
	return mem;
}

void shmclose(LPVOID shm) {
	if (shm)
		UnmapViewOfFile(shm);
}

void objmap_destroy(void) {
	void *const shm = ipj_objmap_state_unmarkup(objmap.state);
	shmclose(shm);

	allocator_destroy();
}

void allocator_destroy(void) {
	void *const shm = ipj_allocator_unmarkup(allocator);
	shmclose(shm);
}

// </aux-impl>
