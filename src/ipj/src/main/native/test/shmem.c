#include <check.h>
#include "shmem.h"

START_TEST(alloc_new) {
	bool is_new = false;
	void *const mem = ipj_shalloc("hello", NULL, &is_new);
	ck_assert(is_new);

	ipj_shfree(mem);
} END_TEST

START_TEST(double_alloc) {
	static const char key[] = "hello";

	bool is_new = true;
	void
		*const mem1 = ipj_shalloc(key, NULL, NULL),
		*const mem2 = ipj_shalloc(key, NULL, &is_new);

	ck_assert_ptr_eq(mem1, mem2);
	ck_assert(!is_new);

	ipj_shfree(mem1);
	ipj_shfree(mem2);
} END_TEST

START_TEST(alloc_sized) {
	size_t size = 8;
	void *const mem = ipj_shalloc("hello", &size, NULL);
	ck_assert_ptr_nonnull(mem);
	ck_assert_int_eq(size, 8);

	ipj_shfree(mem);
} END_TEST

START_TEST(double_alloc_sized) {
	static const char key[] = "hello";

	size_t size1 = 8;
	void *const mem1 = ipj_shalloc(key, &size1, NULL);

	size_t size2 = 16;
	void *const mem2 = ipj_shalloc(key, &size2, NULL);

	ck_assert_ptr_nonnull(mem1);
	ck_assert_ptr_eq(mem1, mem2);
	ck_assert_int_eq(size1, size2);

	ipj_shfree(mem1);
	ipj_shfree(mem2);
} END_TEST

START_TEST(free) {
	static const char key[] = "hello";

	size_t size1 = 8;
	void *const mem1 = ipj_shalloc(key, &size1, NULL);
	ipj_shfree(mem1);

	size_t size2 = 16;
	bool is_new2 = false;
	void *const mem2 = ipj_shalloc(key, &size2, &is_new2);

	ck_assert_ptr_nonnull(mem2);
	ck_assert_int_eq(size2, 16);
	ck_assert(is_new2);

	ipj_shfree(mem2);
} END_TEST


int main(void) {
	ipj_shm_init();

	TCase *const tc = tcase_create("base");
	tcase_add_test(tc, alloc_new);
	tcase_add_test(tc, double_alloc);
	tcase_add_test(tc, alloc_sized);
	tcase_add_test(tc, double_alloc_sized);
	tcase_add_test(tc, free);

	Suite *const s = suite_create("shmem");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	ipj_shm_cleanup();

	return fail_count;
}
