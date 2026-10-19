#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "shmem.h"


// <settings>

#ifndef IPJ_SHMEM_SIZE
	// 4MB by default
	#define IPJ_SHMEM_SIZE 4<<20
#endif

#ifndef IPJ_FILE_MAPPING_NAME
	#define IPJ_FILE_MAPPING_NAME TEXT("ipj")
#endif

// </settings>


// <aux>

struct dict_item { uint32_t key_offset, obj_offset; };
struct shmem_obj { uint32_t size; unsigned char data[]; };

struct shmem {
	uint32_t top, item_count;
	struct dict_item dict_items[];
};

static size_t dict_capacity_get(void);

static ssize_t avl_search(
	size_t count,
	const struct dict_item *items,
	const char *name,
	uint32_t *path // nullable
);

static uint32_t avl_put(struct dict_item *items, const uint32_t *path);

// </aux>


// <globals>

static struct shmem *shmem = NULL;
#define SHMBUF ((unsigned char *)shmem->dict_items)

// </globals>


enum ipj_shmerror ipj_shm_init(void) {
	enum ipj_shmerror status = IPJ_SHM_SUCCESS;

	const HANDLE fmap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		IPJ_SHMEM_SIZE,
		IPJ_FILE_MAPPING_NAME);
	if (fmap == NULL) {
		status = IPJ_SHM_UNKNOWN;
		goto final;
	}

	const LPVOID buf = MapViewOfFile(fmap, FILE_MAP_WRITE, 0, 0, 0);
	if (buf == NULL) {
		status = IPJ_SHM_UNKNOWN;
		goto fmap_cleanup;
	}

	shmem = buf;
	*shmem = (struct shmem) { .top = IPJ_SHMEM_SIZE, .item_count = 0 };

	// no goto final

// cleanup anyway
fmap_cleanup:
	CloseHandle(fmap);

final:
	return status;
}


void ipj_shm_destroy(void) {
	UnmapViewOfFile(shmem);
}


enum ipj_shmerror ipj_shalloc(
	const char *name,
	size_t min_size,
	void **mem
) {
	assert(shmem != NULL);
	assert(name);

	const size_t dict_capacity = dict_capacity_get();
	const size_t avl_max_depth = log2(dict_capacity);
	uint32_t path[avl_max_depth];

	const ssize_t found = avl_search(
		shmem->item_count, shmem->dict_items, name, path);

	if (found != -1) {
		const struct dict_item item = shmem->dict_items[found];

		struct shmem_obj *const
		obj = (struct shmem_obj *)(SHMBUF + item.obj_offset);

		if (mem)
			*mem = obj->data;

		if (obj->size < min_size)
			return IPJ_SHM_EXIST_LESS;
	} else {
		const size_t name_len = strlen(name);

		const uint32_t new_top
			= shmem->top
			- min_size
			- sizeof(struct shmem_obj)
			- (name_len+1);

		const size_t extended_avl_size
			= (shmem->item_count + 1)
			* sizeof(*shmem->dict_items);

		if (new_top < extended_avl_size) 
			return IPJ_SHM_NOMEM;

		shmem->item_count++;
		shmem->top = new_top;

		const struct dict_item item = {
			.key_offset = new_top,
			.obj_offset = new_top + name_len+1 };

		const uint32_t new_dict_item_idx = avl_put(shmem->dict_items, path);
		shmem->dict_items[new_dict_item_idx] = item;

		memcpy((unsigned char *)shmem->dict_items + item.key_offset,
			name, name_len+1);

		struct shmem_obj *const
		obj = (struct shmem_obj *)(SHMBUF + item.obj_offset);

		obj->size = min_size;

		if (mem)
			*mem = obj->data;
	}

	return IPJ_SHM_SUCCESS;
}


// <aux-impl>

size_t dict_capacity_get(void) {
	return (shmem->top - sizeof(*shmem)) / sizeof(*shmem->dict_items);
}

// </aux-impl>
