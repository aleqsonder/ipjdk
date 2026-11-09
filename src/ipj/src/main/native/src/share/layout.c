#include <assert.h>
#include "layout.h"

/* shared memory layout
struct memory {
	struct ipj_strset {
		uint32_t count;
		struct ipj_strset_cell {
			uint32_t key_offset;
		} cells[count];
	} names;

	uint32_t named_unit_offsets[names.count];

	struct ipj_hotel {
		uint32_t room_count, next_free_room_number, rooms[room_count];
	} hotel;

	struct ipj_allocator {
		uint32_t capacity, _bump;
		uint8_t arena[capacity];
	} allocator;
}
*/

void ipj_layout_init(
	struct ipj_layout *layout,
	const struct ipj_layout_settings *settings,
	size_t bufsize,
	void *buf
) {
	assert(layout);
	assert(buf);
	assert(bufsize > IPJ_BUF_MIN_SIZE);

	const struct ipj_layout_settings settings_local = settings
		? *settings
		: IPJ_LAYOUT_SETTINGS_DEFAULT;

	const size_t
		arena_size = settings_local.arena_size_ratio * bufsize,
		ctrl_size = bufsize - arena_size,
		unit_count = (ctrl_size
				- sizeof(*layout->names)
				- sizeof(*layout->hotel)
			) / (sizeof(*layout->names->cells)
				+ sizeof(*layout->named_unit_offsets)
				+ sizeof(*layout->hotel->rooms)),
		named_unit_count = settings_local.named_unit_ratio * unit_count,
		unnamed_unit_count = unit_count - named_unit_count;

	const ptrdiff_t
		names_offset = 0,
		named_unit_offsets_offset = names_offset
			+ sizeof(*layout->names)
			+ named_unit_count * sizeof(*layout->names->cells),
		hotel_offset = named_unit_offsets_offset
			+ named_unit_count * sizeof(*layout->named_unit_offsets),
		allocator_offset = hotel_offset
			+ unnamed_unit_count * sizeof(*layout->hotel->rooms);

	*layout = (struct ipj_layout) {
		.names = (void *)((intptr_t)buf + names_offset),
		.named_unit_offsets = (void *)((intptr_t)buf
			+ named_unit_offsets_offset),
		.hotel = (void *)((intptr_t)buf + hotel_offset),
		.allocator = (void *)((intptr_t)buf + allocator_offset) };

	*layout->names = IPJ_STRSET_INITIAL(named_unit_count);
	*layout->allocator = IPJ_ALLOCATOR_INITIAL(arena_size);

	ipj_hotel_init(layout->hotel, unnamed_unit_count);
}
