#ifndef IPJ_LAYOUT_H
#define IPJ_LAYOUT_H

#include "strset.h"
#include "hotel.h"
#include "allocator.h"

struct ipj_layout {
	struct ipj_strset *names;
	uint32_t *named_unit_offsets;
	struct ipj_hotel *hotel;
	struct ipj_allocator *allocator;
};

struct ipj_layout_settings {
	float arena_size_ratio, named_unit_ratio;
};

#define IPJ_LAYOUT_SETTINGS_DEFAULT ((struct ipj_layout_settings) { \
	.arena_size_ratio = .8f, .named_unit_ratio = .5f })

#define IPJ_BUF_MIN_SIZE (sizeof(struct ipj_strset) \
	+ sizeof(struct ipj_hotel) + sizeof(struct ipj_allocator))

void ipj_layout_init(
	struct ipj_layout *layout,
	const struct ipj_layout_settings *settings,
	size_t bufsize,
	void *buf);

#endif
