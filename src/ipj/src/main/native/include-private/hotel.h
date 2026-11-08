#ifndef IPJ_HOTEL_H
#define IPJ_HOTEL_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/param.h>

struct ipj_hotel {
	uint32_t room_count, next_free_room_number, rooms[];
};

void ipj_hotel_init(struct ipj_hotel *, size_t room_count);

// returns -1 if there are no free rooms left
ssize_t ipj_hotel_book(struct ipj_hotel *);

// no-op on `room_number==-1`
void ipj_hotel_leave(struct ipj_hotel *hotel, ssize_t room_number);

#endif
