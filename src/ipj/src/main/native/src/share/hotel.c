#include <assert.h>
#include "hotel.h"

#define IPJ_HOTEL_ROOM_UNINITIALIZED UINT32_MAX

void ipj_hotel_init(struct ipj_hotel *hotel, size_t room_count) {
	assert(hotel);

	*hotel = (struct ipj_hotel) {
		.room_count = room_count,
		.next_free_room_number = 0 };

	if (room_count > 0)
		hotel->rooms[0] = IPJ_HOTEL_ROOM_UNINITIALIZED;
}

ssize_t ipj_hotel_book(struct ipj_hotel *hotel) {
	assert(hotel);

	if (hotel->next_free_room_number >= hotel->room_count)
		return -1;

	const uint32_t
		next_room_number = hotel->next_free_room_number,
		next_room = hotel->rooms[next_room_number];

	if (next_room != IPJ_HOTEL_ROOM_UNINITIALIZED) {
		hotel->next_free_room_number = next_room;
	} else if (++hotel->next_free_room_number < hotel->room_count) {
		hotel->rooms[hotel->next_free_room_number]
			= IPJ_HOTEL_ROOM_UNINITIALIZED;
	}

	return next_room_number;
}

void ipj_hotel_leave(struct ipj_hotel *hotel, ssize_t room_number) {
	assert(hotel);
	assert(room_number < hotel->room_count);

	if (room_number == -1)
		return;

	hotel->rooms[room_number] = hotel->next_free_room_number;
	hotel->next_free_room_number = room_number;
}
