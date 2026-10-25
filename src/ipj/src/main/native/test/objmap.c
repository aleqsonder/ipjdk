#include <check.h>
#include <string.h>

#include "objmap.h"

#define ARENA_SIZE 512
#define MAP_BUFSIZE 256

uint8_t arena[ARENA_SIZE], map_buf[MAP_BUFSIZE];
struct ipj_objmap objmap;

struct keyval { char *key, *value; };
static const struct keyval kvs[] = {
	{ "hello", "world" },
	{ "foo", "bar" },
	{ "fizz", "buzz" },
	{ "key", "value" } };
static const size_t kv_count = sizeof(kvs) / sizeof(*kvs);

static void setup_empty(void) {
	objmap = (struct ipj_objmap) {
		.allocator = ipj_allocator_markup(ARENA_SIZE, arena),
		.state = ipj_objmap_state_markup(MAP_BUFSIZE, map_buf) };
}

static void setup_filled(void) {
	setup_empty();

	for (size_t i = 0; i < kv_count; ++i) {
		struct keyval kv = kvs[i];
		const size_t payload_size = strlen(kv.value)+1;
		ipj_obj_t *const obj = ipj_objmap_provide(
			&objmap, kv.key, payload_size, NULL);
		strcpy(obj, kv.value);
	}
}

START_TEST(provide) {
	ipj_obj_t *const obj = ipj_objmap_provide(&objmap, "hello", 0, NULL);
	ck_assert_ptr_nonnull(obj);
} END_TEST

START_TEST(provide_new) {
	const size_t payload_size = 6;
	bool is_new = false;
	ipj_obj_t *const obj = ipj_objmap_provide(
		&objmap, "provide_new", payload_size, &is_new);

	ck_assert(is_new);
	ck_assert_int_eq(ipj_obj_payload_size(obj), payload_size);
} END_TEST

START_TEST(provide_existing) {
	bool is_new = true;
	ipj_obj_t *const existing_obj = ipj_objmap_provide(
		&objmap, kvs[0].key, 0, &is_new);

	ck_assert(!is_new);
	ck_assert_str_eq(existing_obj, kvs[0].value);
} END_TEST

START_TEST(search) {
	ipj_obj_t *const obj = ipj_objmap_search(&objmap, kvs[1].key);
	ck_assert_str_eq(obj, kvs[1].value);
} END_TEST

START_TEST(search_non_existing) {
	ipj_obj_t *const obj = ipj_objmap_search(&objmap, "non-existing");
	ck_assert_ptr_null(obj);
} END_TEST

START_TEST(erase) {
	ipj_objmap_erase(&objmap, kvs[0].key);
	ipj_obj_t *const obj = ipj_objmap_search(&objmap, kvs[0].key);
	ck_assert_ptr_null(obj);
} END_TEST

START_TEST(erase_non_existing) {
	ipj_objmap_erase(&objmap, "non-existing");
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("base");
	tcase_add_checked_fixture(tc, setup_filled, NULL);
	tcase_add_test(tc, provide);
	tcase_add_test(tc, provide_new);
	tcase_add_test(tc, provide_existing);
	tcase_add_test(tc, search);
	tcase_add_test(tc, search_non_existing);
	tcase_add_test(tc, erase);
	tcase_add_test(tc, erase_non_existing);

	Suite *const s = suite_create("objmap");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
