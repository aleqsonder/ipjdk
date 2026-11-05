#include <check.h>
#include "allocator.h"

#define BUFSIZE 1024
uint8_t buf[BUFSIZE];
struct ipj_allocator *const allocator = (struct ipj_allocator *)buf;

void setup(void) { *allocator = IPJ_ALLOCATOR_INITIAL(BUFSIZE); }

START_TEST(alloc) {
	void *const mem = ipj_alloc(allocator, BUFSIZE / 2);
	ck_assert_ptr_nonnull(mem);
} END_TEST

START_TEST(alloc0) {
	void *const mem = ipj_alloc(allocator, 0);
	ck_assert_ptr_null(mem);
} END_TEST

START_TEST(alloc_too_much) {
	void *const mem = ipj_alloc(allocator, BUFSIZE * 2);
	ck_assert_ptr_null(mem);
} END_TEST

int main(void) {
	TCase *const main_tc = tcase_create("main");
	tcase_add_checked_fixture(main_tc, setup, NULL);
	tcase_add_test(main_tc, alloc);
	tcase_add_test(main_tc, alloc0);
	tcase_add_test(main_tc, alloc_too_much);

	Suite *const s = suite_create("allocator");
	suite_add_tcase(s, main_tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
