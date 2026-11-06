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

START_TEST(realloc_NULL) {
	void *const mem = ipj_realloc(allocator, NULL, 32);
	ck_assert_ptr_nonnull(mem);
} END_TEST

START_TEST(realloc_less) {
	void *const mem1 = ipj_alloc(allocator, 32);
	ck_assert_ptr_nonnull(mem1);

	void *const mem2 = ipj_realloc(allocator, mem1, 16);
	ck_assert_ptr_eq(mem1, mem2);
} END_TEST

START_TEST(realloc_edge) {
	void *const mem1 = ipj_alloc(allocator, 6);
	strcpy(mem1, "hello");

	void *const mem2 = ipj_realloc(allocator, mem1, 64);
	ck_assert_ptr_eq(mem1, mem2);

	strcat(mem2, " world");

	void *const mem3 = ipj_alloc(allocator, 32);
	strcpy(mem3, "override");

	ck_assert_str_eq(mem2, "hello world");
} END_TEST

START_TEST(realloc_non_edge) {
	void *const mem1 = ipj_alloc(allocator, 32);
	strcpy(mem1, "hello");

	void *const mem2 = ipj_alloc(allocator, 16);
	strcpy(mem2, "world");

	void *const mem3 = ipj_realloc(allocator, mem1, 64);
	ck_assert_ptr_ne(mem1, mem3);
	ck_assert_str_eq(mem1, mem3);
} END_TEST

START_TEST(realloc_too_much) {
	void *const mem1 = ipj_alloc(allocator, 32);
	ck_assert_ptr_nonnull(mem1);

	void *const mem2 = ipj_realloc(allocator, mem1, BUFSIZE);
	ck_assert_ptr_null(mem2);	
} END_TEST

START_TEST(realloc_out_of_range) {
	char *const mem1 = ipj_alloc(allocator, 16);
	ck_assert_ptr_nonnull(mem1);

	void *const mem2 = ipj_realloc(allocator, mem1 + BUFSIZE, 12);
	ck_assert_ptr_null(mem2);
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("main");
	tcase_add_checked_fixture(tc, setup, NULL);
	tcase_add_test(tc, alloc);
	tcase_add_test(tc, alloc0);
	tcase_add_test(tc, alloc_too_much);
	tcase_add_test(tc, realloc_NULL);
	tcase_add_test(tc, realloc_less);
	tcase_add_test(tc, realloc_edge);
	tcase_add_test(tc, realloc_non_edge);
	tcase_add_test(tc, realloc_too_much);
	tcase_add_test(tc, realloc_out_of_range);

	Suite *const s = suite_create("allocator");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
