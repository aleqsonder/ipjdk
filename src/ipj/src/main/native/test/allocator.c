#include <check.h>
#include "allocator.h"

#define BUFSIZE 1024
uint8_t buf[BUFSIZE];
ipj_allocator_t *allocator = NULL;

void setup(void) {
	allocator = ipj_allocator_markup(BUFSIZE, buf);
}

START_TEST(insufficient_buf_markup) {
	static const size_t INSUF_BUFSIZE = 2;
	uint8_t insuf_buf[INSUF_BUFSIZE];

	ipj_allocator_t *const
	a = ipj_allocator_markup(INSUF_BUFSIZE, insuf_buf);

	ck_assert_ptr_null(a);
} END_TEST

START_TEST(sufficient_buf_markup) {
	ck_assert_ptr_nonnull(allocator);
} END_TEST

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

START_TEST(offset) {
	void *const mem = ipj_alloc(allocator, BUFSIZE / 2);
	const ptrdiff_t offset = ipj_mem_offset(allocator, mem);

	void *const restored_mem = ipj_offset_resolve(allocator, offset);
	ck_assert_ptr_eq(mem, restored_mem);

	const void *const
	restored_const_mem = ipj_offset_resolve_const(allocator, offset);
	ck_assert_ptr_eq(mem, restored_const_mem);
} END_TEST


int main(void) {
	TCase *const fixtureless_tc = tcase_create("fixtureless");
	tcase_add_test(fixtureless_tc, insufficient_buf_markup);

	TCase *const main_tc = tcase_create("main");
	tcase_add_checked_fixture(main_tc, setup, NULL);
	tcase_add_test(main_tc, sufficient_buf_markup);
	tcase_add_test(main_tc, alloc);
	tcase_add_test(main_tc, alloc0);
	tcase_add_test(main_tc, alloc_too_much);
	tcase_add_test(main_tc, offset);

	Suite *const s = suite_create("allocator");
	suite_add_tcase(s, main_tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
