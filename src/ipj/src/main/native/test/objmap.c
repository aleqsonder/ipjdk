#include <check.h>
#include <string.h>

#include "objmap.h"

#define ARENA_SIZE 512
#define MAP_BUFSIZE 256

uint8_t arena[ARENA_SIZE], map_buf[MAP_BUFSIZE];
struct ipj_objmap *objmap = NULL;

static void setup(void) {
	objmap = (struct ipj_objmap *)map_buf;
	*objmap = (struct ipj_objmap) {
		.allocator = ipj_allocator_markup(ARENA_SIZE, arena),
		.state = ipj_objmap_state_markup(MAP_BUFSIZE, map_buf) };
}

START_TEST(provide) {
	void *const obj = ipj_objmap_provide(objmap, "hello", 0);
	ck_assert_ptr_nonnull(obj);
} END_TEST

START_TEST(provide_multiple) {
	struct keyval { char *key, *value; };
	struct keyval kvs[] = {
		{ "hello", "world" },
		{ "foo", "bar" },
		{ "fizz", "buzz" } };
	const size_t kv_count = sizeof(kvs) / sizeof(*kvs);

	for (size_t i = 0; i < kv_count; ++i) {
		struct keyval kv = kvs[i];
		const size_t payload_size = strlen(kv.value)+1;
		void *const obj = ipj_objmap_provide(objmap, kv.key, payload_size);
		strcpy(obj, kv.value);
	}

	for (size_t i = 0; i < kv_count; ++i) {
		struct keyval kv = kvs[i];
		const void *const obj = ipj_objmap_provide(objmap, kv.key, 0);
		ck_assert_str_eq(obj, kv.value);
	}
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("base");
	tcase_add_checked_fixture(tc, setup, NULL);
	tcase_add_test(tc, provide);
	tcase_add_test(tc, provide_multiple);

	Suite *const s = suite_create("objmap");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
