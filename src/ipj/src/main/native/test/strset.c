#include <check.h>
#include "strset.h"

static uint8_t allocator_buffer[512], strset_buffer[512];

static struct ipj_strset set = {
	.allocator = (void *)allocator_buffer,
	.cell_array = (void *)strset_buffer };

static const char *const keys[] = { "hello", "foo", "fizz", "key", };
static const size_t key_count = sizeof(keys) / sizeof(*keys);

static const size_t
	cell_count = IPJ_STRSET_CELL_COUNT(sizeof(strset_buffer));

static int values[cell_count];

static void empty_set_setup(void) {
	*set.allocator = IPJ_ALLOCATOR_INITIAL(sizeof(allocator_buffer));
	*set.cell_array = IPJ_STRSET_CELL_ARRAY_INITIAL(cell_count);
}

static void filled_set_setup(void) {
	empty_set_setup();

	for (size_t i = 0; i < key_count; ++i) {
		const ssize_t idx = ipj_strset_provide(&set, keys[i], NULL);
		ck_assert_int_ne(idx, -1);
		values[idx] = i;
	}
}

START_TEST(provide_new) {
	bool is_new = false;
	const ssize_t idx = ipj_strset_provide(&set, "new", &is_new);
	ck_assert_int_ne(idx, -1);
	ck_assert(is_new);
} END_TEST

START_TEST(provide_existing) {
	bool is_new = true;
	const ssize_t idx = ipj_strset_provide(&set, keys[0], &is_new);
	ck_assert(!is_new);
	ck_assert_int_eq(values[idx], 0);
} END_TEST

START_TEST(search_existing) {
	const ssize_t idx = ipj_strset_search(&set, keys[1]);
	ck_assert_uint_eq(values[idx], 1);
} END_TEST

START_TEST(search_non_existing) {
	const ssize_t idx = ipj_strset_search(&set, "non-existing");
	ck_assert_int_eq(idx, -1);
} END_TEST

START_TEST(erase) {
	const ssize_t idx1 = ipj_strset_search(&set, keys[0]);
	ck_assert_int_ne(idx1, -1);

	ipj_strset_erase(&set, idx1);

	const ssize_t idx2 = ipj_strset_search(&set, keys[0]);
	ck_assert_int_eq(idx2, -1);
} END_TEST

START_TEST(erase_non_existing) {
	ipj_strset_erase(&set, -1);

	for (size_t i = 0; i < key_count; ++i) {
		const ssize_t idx = ipj_strset_search(&set, keys[i]);
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
