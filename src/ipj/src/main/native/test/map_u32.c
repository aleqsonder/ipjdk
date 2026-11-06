#include <check.h>
#include "map_u32.h"

uint8_t buf[512];
struct ipj_map_u32 map;

struct keyval { char *key; int value; };
static const struct keyval kvs[] = {
	{ "hello", 1 },
	{ "foo", 2 },
	{ "fizz", 3 },
	{ "key", 4 } };
static const size_t kv_count = sizeof(kvs) / sizeof(*kvs);

static void empty_map_setup(void) {
	struct ipj_allocator *const allocator = (void *)buf;
	*allocator = IPJ_ALLOCATOR_INITIAL(sizeof(buf));
	ck_assert(ipj_map_u32_init(&map, allocator, 16));
}

static void filled_map_setup(void) {
	empty_map_setup();

	for (size_t i = 0; i < kv_count; ++i)
		*ipj_map_u32_provide(&map, kvs[i].key, NULL) = kvs[i].value;
}

static void map_teardown(void) { ipj_map_u32_destroy(&map); }

START_TEST(provide) {
	uint32_t *const num = ipj_map_u32_provide(&map, "hello", NULL);
	ck_assert_ptr_nonnull(num);
} END_TEST

START_TEST(provide_new) {
	bool is_new = false;
	uint32_t *const num = ipj_map_u32_provide(&map, "new", &is_new);
	ck_assert(is_new);
	ck_assert_ptr_nonnull(num);
} END_TEST

START_TEST(provide_existing) {
	bool is_new = true;
	uint32_t *const existing_num = ipj_map_u32_provide(
		&map, kvs[0].key, &is_new);

	ck_assert(!is_new);
	ck_assert_uint_eq(*existing_num, kvs[0].value);
} END_TEST

START_TEST(search) {
	uint32_t *const num = ipj_map_u32_search(&map, kvs[1].key);
	ck_assert_uint_eq(*num, kvs[1].value);
} END_TEST

START_TEST(search_non_existing) {
	uint32_t *const num = ipj_map_u32_search(&map, "non-existing");
	ck_assert_ptr_null(num);
} END_TEST

START_TEST(erase) {
	ipj_map_u32_erase(&map, kvs[0].key);
	uint32_t *const num = ipj_map_u32_search(&map, kvs[0].key);
	ck_assert_ptr_null(num);
} END_TEST

START_TEST(erase_non_existing) {
	ipj_map_u32_erase(&map, "non-existing");
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("base");
	tcase_add_checked_fixture(tc, filled_map_setup, map_teardown);
	tcase_add_test(tc, provide);
	tcase_add_test(tc, provide_new);
	tcase_add_test(tc, provide_existing);
	tcase_add_test(tc, search);
	tcase_add_test(tc, search_non_existing);
	tcase_add_test(tc, erase);
	tcase_add_test(tc, erase_non_existing);

	Suite *const s = suite_create("map_u32");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
