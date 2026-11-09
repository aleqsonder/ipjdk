#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "shmmap.h"
#include "layout.h"


// <settings>

#ifndef IPJ_SHBUF_FNAME
	#define IPJ_SHBUF_FNAME "ipj-shbuf"
#endif

#ifndef IPJ_SHBUF_SIZE
	#define IPJ_SHBUF_SIZE (4<<20) /* 4MB */
#endif

#ifndef IPJ_ARENA_SIZE_RATIO
	#define IPJ_ARENA_SIZE_RATIO .8f
#endif

#ifndef IPJ_NAMED_UNIT_RATIO
	#define IPJ_NAMED_UNIT_RATIO .5f
#endif

// </settings>


// <globals>

static ipj_shm_t *shm = NULL;

static const struct ipj_layout_settings LAYOUT_SETTINGS = {
	.arena_size_ratio = IPJ_ARENA_SIZE_RATIO,
	.named_unit_ratio = IPJ_NAMED_UNIT_RATIO };
static struct ipj_layout layout;

// </globals>


// <utils>

struct shunit { uint32_t refcount, payload_size; uint8_t payload[]; };

static ptrdiff_t get_offset(const struct shunit *mem);
static struct shunit *resolve_offset(ptrdiff_t offset);

static ssize_t encode_handle_unnamed(size_t index);
static ssize_t encode_handle_named(size_t index);

enum handle_type { HANDLE_INVALID, HANDLE_NAMED, HANDLE_UNNAMED };
static enum handle_type decode_handle(ssize_t handle, size_t *index);

// </utils>


int ipj_shm_init(void) {
	bool is_new = false;
	shm = ipj_shm_open(IPJ_SHBUF_FNAME, IPJ_SHBUF_SIZE, &is_new);

	if (!shm)
		return 1;

	void *const shbuf = ipj_shm_data(shm);
	assert(shbuf);

	ipj_layout_init(&layout, &LAYOUT_SETTINGS, IPJ_SHBUF_SIZE, shbuf);

	return 0;
}

void ipj_shm_cleanup(void) { ipj_shm_close(shm); }

ssize_t ipj_shm_alloc(size_t size) {
	const size_t total_size = sizeof(struct shunit) + size;

	struct shunit *const unit = ipj_alloc(layout.allocator, total_size);
	if (!unit)
		return -1;

	const ssize_t index = ipj_hotel_book(layout.hotel);
	if (index == -1) {
		ipj_free(layout.allocator, unit);
		return -1;
	}

	layout.hotel->rooms[index] = get_offset(unit);
	*unit = (struct shunit) { .refcount = 1, .payload_size = size };
	return encode_handle_unnamed(index);
}

ssize_t ipj_shm_provide(const char *name, size_t *size, bool *is_new) {
	assert(name);

	bool is_new_local = false;
	const ssize_t index = ipj_strset_provide(
		layout.names, layout.allocator, name, &is_new_local);

	if (index == -1)
		return -1;

	if (is_new_local) {
		const size_t
			local_size = size ? *size : 0,
			total_size = sizeof(struct shunit) + local_size;

		struct shunit
			*const unit = ipj_alloc(layout.allocator, total_size);

		if (!unit) {
			ipj_strset_erase(layout.names, layout.allocator, index);
			return -1;
		}

		*unit = (struct shunit) {
			.payload_size = local_size,
			.refcount = 1 };

		layout.named_unit_offsets[index] = get_offset(unit);
	} else {
		struct shunit *const unit = resolve_offset(
			layout.named_unit_offsets[index]);
		assert(unit);
		unit->refcount++;

		if (size)
			*size = unit->payload_size;
	}

	if (is_new)
		*is_new = is_new_local;

	return encode_handle_named(index);
}

void ipj_shm_release(ssize_t handle) {
	struct shunit *unit = NULL;

	size_t index;
	switch (decode_handle(handle, &index)) {
	case HANDLE_UNNAMED:
		unit = resolve_offset(layout.hotel->rooms[index]);
		break;
	case HANDLE_NAMED:
		unit = resolve_offset(layout.named_unit_offsets[index]);
		ipj_strset_erase(layout.names, layout.allocator, index);
		break;
	default:
		unit = NULL;
	}

	if (unit)
		ipj_free(layout.allocator, unit);
}

void *ipj_shm_handle_resolve(ssize_t handle) {
	struct shunit *unit = NULL;

	size_t index;
	switch (decode_handle(handle, &index)) {
	case HANDLE_UNNAMED:
		unit = resolve_offset(layout.hotel->rooms[index]);
		break;
	case HANDLE_NAMED:
		unit = resolve_offset(layout.named_unit_offsets[index]);
		break;
	default:
		unit = NULL;
	}

	return unit ? unit->payload : NULL;
}


// <utils-impl>

ptrdiff_t get_offset(const struct shunit *mem) {
	assert(mem);
	return (intptr_t)mem - (intptr_t)layout.allocator;
}

struct shunit *resolve_offset(ptrdiff_t offset) {
	return (struct shunit *)((intptr_t)layout.allocator + offset);
}

ssize_t encode_handle_unnamed(size_t index) {
	return index + layout.names->count;
}

ssize_t encode_handle_named(size_t index) {
	return index;
}

enum handle_type decode_handle(ssize_t handle, size_t *index) {
	if (handle == -1)
		return HANDLE_INVALID;

	if (index)
		*index = handle % layout.names->count;

	return handle < layout.names->count
		? HANDLE_NAMED
		: HANDLE_UNNAMED;
}

// </utils-impl>
