#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "shmem.h"
#include "shmmap.h"
#include "strset.h"

// <settings>

#ifndef IPJ_SHBUF_FNAME
	#define IPJ_SHBUF_FNAME "ipj-shbuf"
#endif

#ifndef IPJ_SHBUF_SIZE
	// 4MB
	#define IPJ_SHBUF_SIZE (4<<20)
#endif

#ifndef IPJ_ARENA_SIZE_RATIO
	#define IPJ_ARENA_SIZE_RATIO 0.8
#endif

// </settings>


// <globals>

static ipj_shm_t *shm = NULL;
static struct ipj_strset set;
static uint32_t *obj_offsets = NULL;

static ipj_shm_handle_t get_handle(void *mem);
static void *resolve_handle(ipj_shm_handle_t handle);

// </globals>


// <obj>

struct obj { uint32_t refcount, payload_size; uint8_t payload[]; };

// </obj>


// <utils>

static size_t minz(size_t l, size_t r) { return l < r ? l : r; }

static size_t get_io_byte_count(
	const struct obj *,
	size_t offset,
	size_t requested_size);

// </utils>


int ipj_shm_init(void) {
	bool is_new = false;
	shm = ipj_shm_open(IPJ_SHBUF_FNAME, IPJ_SHBUF_SIZE, &is_new);

	if (!shm)
		return 1;

	void *const shbuf = ipj_shm_data(shm);
	assert(shbuf);

	static const size_t
		arena_size = IPJ_SHBUF_SIZE * IPJ_ARENA_SIZE_RATIO,
		map_size = IPJ_SHBUF_SIZE - arena_size,
		obj_count_limit
			= (map_size - sizeof(struct ipj_strset_cell_array))
			/ (sizeof(struct ipj_strset_cell) + sizeof(*obj_offsets));

	set = (struct ipj_strset) {
		.allocator = shbuf,
		.cell_array = (void *)((uint8_t *)shbuf + arena_size) };

	obj_offsets = (void *)((uint8_t *)shbuf + arena_size
		+ obj_count_limit * sizeof(struct ipj_strset_cell));
	

	if (is_new) {
		*set.cell_array = IPJ_STRSET_CELL_ARRAY_INITIAL(obj_count_limit),
		*set.allocator = IPJ_ALLOCATOR_INITIAL(arena_size);
	}

	return 0;
}

void ipj_shm_cleanup(void) { ipj_shm_close(shm); }

ipj_shm_handle_t ipj_shm_alloc(size_t size) {
	const size_t total_size = sizeof(struct obj) + size;

	struct obj *const obj = ipj_alloc(set.allocator, total_size);
	if (!obj)
		return IPJ_SHM_HANDLE_INVALID;

	*obj = (struct obj) { .refcount = 1, .payload_size = size };
	return get_handle(obj);
}

ipj_shm_handle_t ipj_shm_provide(
	const char *name,
	size_t *size,
	bool *is_new
) {
	assert(name);

	bool is_new_local = false;
	const ssize_t index = ipj_strset_provide(&set, name, &is_new_local);

	if (index == -1)
		return IPJ_SHM_HANDLE_INVALID;

	if (is_new_local) {
		const ipj_shm_handle_t handle = ipj_shm_alloc(size ? *size : 0);
		if (handle == IPJ_SHM_HANDLE_INVALID)
			goto map_cleanup;
		*handle_mapped = handle;
	} else {
		struct obj *const obj = resolve_handle(*handle_mapped);
		assert(obj);
		obj->refcount++;
	}

	goto finish;

map_cleanup:
	ipj_map_u32_erase(map, allocator, name);

finish:
	return *handle_mapped;
}

void ipj_shm_release(ipj_shm_handle_t handle) {
	if (handle != IPJ_SHM_HANDLE_INVALID) {
		struct obj *const obj = resolve_handle(handle);
		ipj_obj_t *const obj = ipj_offset_resolve(allocator, handle);
		if (ipj_obj_release(obj)) {
			if (ipj_obj_has_name(obj)) {
				const char *const id = ipj_obj_id_const(obj);
				ipj_objmap_erase(&map, id);
			} else {
				ipj_free(allocator, ipj_obj_uncheckout(obj));
			}
		}
	}
}

ssize_t ipj_shm_read(
	ipj_shm_handle_t handle,
	size_t offset,
	void *dest,
	size_t size
) {
	assert(size == 0 || dest);

	if (handle == IPJ_SHM_HANDLE_INVALID)
		return -1;

	ipj_obj_t *const obj = resolve_handle(handle);
	const size_t count = get_io_byte_count(obj, offset, size);

	if (count > 0)
		memcpy(dest, (uint8_t *)obj + offset, count);

	return size - count;
}

ssize_t ipj_shm_write(
	ipj_shm_handle_t handle,
	size_t offset,
	const void *src,
	size_t size
) {
	assert(size == 0 || src);

	if (handle == IPJ_SHM_HANDLE_INVALID)
		return -1;

	ipj_obj_t *const obj = resolve_handle(handle);
	const size_t write_count = get_io_byte_count(obj, offset, size);

	if (write_count > 0)
		memcpy((uint8_t *)obj + offset, src, write_count);

	return size - write_count;
}


// <globals-impl>

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

ipj_shm_handle_t get_handle(void *mem) {
	if (!mem)
		return IPJ_SHM_HANDLE_INVALID;
	return ipj_mem_offset(allocator, mem);
}

void *resolve_handle(ipj_shm_handle_t handle) {
	if (handle == IPJ_SHM_HANDLE_INVALID)
		return NULL;
	return ipj_offset_resolve(allocator, handle);
}

// </globals-impl>


// <utils-impl>

size_t get_io_byte_count(
	const ipj_obj_t *obj,
	size_t offset,
	size_t requested_size
) {
	const size_t
		obj_payload_size = ipj_obj_payload_size(obj),
		limit = minz(obj_payload_size, offset + requested_size);

	return limit > offset ? limit - offset : 0;
}

// </utils-impl>
