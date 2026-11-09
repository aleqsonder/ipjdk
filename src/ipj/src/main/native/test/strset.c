#include <check.h>
#include "strset.h"

static uint8_t allocator_buffer[512], strset_buffer[512];

static struct ipj_allocator *const allocator = (void *)allocator_buffer;
static struct ipj_strset *const set = (void *)strset_buffer;

static const char *const KEYS[] = { "hello", "foo", "fizz", "key", };
static const size_t KEY_COUNT = sizeof(KEYS) / sizeof(*KEYS);

#define CELL_COUNT IPJ_STRSET_CELL_COUNT(sizeof(strset_buffer))
static int values[CELL_COUNT];

static void empty_set_setup(void) {
	*set = IPJ_STRSET_INITIAL(CELL_COUNT);
	*allocator = IPJ_ALLOCATOR_INITIAL(sizeof(allocator_buffer));
}

static void filled_set_setup(void) {
	empty_set_setup();

	for (size_t i = 0; i < KEY_COUNT; ++i) {
		const ssize_t idx = ipj_strset_provide(
			set, allocator, KEYS[i], NULL);
		ck_assert_int_ne(idx, -1);
		values[idx] = i;
	}
}

START_TEST(provide_new) {
	bool is_new = false;
	const ssize_t idx = ipj_strset_provide(
		set, allocator, "new", &is_new);
	ck_assert_int_ne(idx, -1);
	ck_assert(is_new);
} END_TEST

START_TEST(provide_existing) {
	bool is_new = true;
	const ssize_t idx = ipj_strset_provide(
		set, allocator, KEYS[0], &is_new);
	ck_assert(!is_new);
	ck_assert_int_eq(values[idx], 0);
} END_TEST

START_TEST(search_existing) {
	const ssize_t idx = ipj_strset_search(set, allocator, KEYS[1]);
	ck_assert_uint_eq(values[idx], 1);
} END_TEST

START_TEST(search_non_existing) {
	const ssize_t idx = ipj_strset_search(
		set, allocator, "non-existing");
	ck_assert_int_eq(idx, -1);
} END_TEST

START_TEST(erase) {
	const ssize_t idx1 = ipj_strset_search(set, allocator, KEYS[0]);
	ck_assert_int_ne(idx1, -1);

	ipj_strset_erase(set, allocator, idx1);

	const ssize_t idx2 = ipj_strset_search(set, allocator, KEYS[0]);
	ck_assert_int_eq(idx2, -1);
} END_TEST

START_TEST(erase_non_existing) {
	ipj_strset_erase(set, allocator, -1);

	for (size_t i = 0; i < KEY_COUNT; ++i) {
		const ssize_t idx = ipj_strset_search(set, allocator, KEYS[i]);
		ck_assert_int_ne(idx, -1);
		ck_assert_int_eq(values[idx], i);
	}
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("base");
	tcase_add_checked_fixture(tc, filled_set_setup, NULL);
	tcase_add_test(tc, provide_new);
	tcase_add_test(tc, provide_existing);
	tcase_add_test(tc, search_existing);
	tcase_add_test(tc, search_non_existing);
	tcase_add_test(tc, erase);
	tcase_add_test(tc, erase_non_existing);

	Suite *const s = suite_create("strset");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
