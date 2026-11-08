#include <check.h>
#include "hotel.h"

static uint8_t buf[64];
static struct ipj_hotel *const hotel = (void *)buf;

static const size_t ROOM_COUNT
	= (sizeof(buf) - sizeof(*hotel))
	/ sizeof(*hotel->rooms);

static const size_t OCCUPIED_ROOM_COUNT = (ROOM_COUNT + 1) / 2;
static unsigned occupied_room_numbers[OCCUPIED_ROOM_COUNT];

static void setup_hotel_empty(void) {
	ipj_hotel_init(hotel, ROOM_COUNT);
}

static void setup_hotel_half_filled(void) {
	setup_hotel_empty();

	for (size_t i = 0; i < OCCUPIED_ROOM_COUNT; ++i) {
		const ssize_t room_number = ipj_hotel_book(hotel);
		ck_assert_int_ne(room_number, -1);
		occupied_room_numbers[i] = room_number;
	}
}

START_TEST(leave_valid_room) {
	const unsigned room_number = occupied_room_numbers[0];
	ck_assert_int_ne(room_number, -1);
	ipj_hotel_leave(hotel, room_number);
} END_TEST

START_TEST(leave_invalid_room) {
	ipj_hotel_leave(hotel, -1);
} END_TEST

START_TEST(rebook) {
	static const size_t
		ROOM_TO_FREE_COUNT = (OCCUPIED_ROOM_COUNT + 1) / 2,
		ROOM_REMAIN_COUNT
			= ROOM_COUNT
			+ ROOM_TO_FREE_COUNT
			- OCCUPIED_ROOM_COUNT;

	for (size_t i = 0; i < ROOM_TO_FREE_COUNT; ++i) {
		const size_t index = OCCUPIED_ROOM_COUNT - i;
		const unsigned room_number = occupied_room_numbers[index];
		ipj_hotel_leave(hotel, room_number);
	}

	for (size_t i = 0; i < ROOM_REMAIN_COUNT; ++i) {
		const ssize_t room_number = ipj_hotel_book(hotel);
		ck_assert_int_ne(room_number, -1);
	}
} END_TEST

START_TEST(overbook) {
	static const size_t
		ROOM_REMAIN_COUNT = ROOM_COUNT - OCCUPIED_ROOM_COUNT;

	for (size_t i = 0; i < ROOM_REMAIN_COUNT; ++i) {
		const ssize_t room_number = ipj_hotel_book(hotel);
		ck_assert_int_ne(room_number, -1);
	}
	ck_assert_int_eq(ipj_hotel_book(hotel), -1);

	ipj_hotel_leave(hotel, occupied_room_numbers[0]);
	ck_assert_int_eq(ipj_hotel_book(hotel), occupied_room_numbers[0]);
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("base");
	tcase_add_checked_fixture(tc, setup_hotel_half_filled, NULL);
	tcase_add_test(tc, leave_valid_room);
	tcase_add_test(tc, leave_invalid_room);
	tcase_add_test(tc, rebook);
	tcase_add_test(tc, overbook);

	Suite *const s = suite_create("hotel");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
