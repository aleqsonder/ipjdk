#include <check.h>
#include "shmem.h"

ssize_t handle = -1;
#define BUFSIZE 16

static void io_setup(void) {
	handle = ipj_shm_alloc(BUFSIZE);
	ck_assert(handle != -1);
}

static void io_teardown(void) { ipj_shm_release(handle); }

START_TEST(provide_new) {
	bool is_new = false;
	const ssize_t handle = ipj_shm_provide("hello", NULL, &is_new);
	ck_assert(handle != -1);
	ck_assert(is_new);

	ipj_shm_release(handle);
} END_TEST

START_TEST(double_provide) {
	static const char key[] = "hello";

	bool is_new = true;
	const ssize_t
		handle1 = ipj_shm_provide(key, NULL, NULL),
		handle2 = ipj_shm_provide(key, NULL, &is_new);

	ck_assert(handle1 != -1);
	ck_assert(handle1 == handle2);
	ck_assert(!is_new);

	ipj_shm_release(handle1);
	ipj_shm_release(handle2);
} END_TEST

START_TEST(provide_distinct) {
	bool is_new = false;

	const ssize_t handle1 = ipj_shm_provide("hello", NULL, &is_new);
	ck_assert(handle1 != -1);
	ck_assert(is_new);

	const ssize_t handle2 = ipj_shm_provide("world", NULL, &is_new);
	ck_assert(handle2 != -1);
	ck_assert(is_new);

	ck_assert(handle1 != handle2);

	ipj_shm_release(handle1);
	ipj_shm_release(handle2);
} END_TEST

START_TEST(provide_sized) {
	size_t size = 8;
	const ssize_t handle = ipj_shm_provide("hello", &size, NULL);
	ck_assert(handle != -1);
	ck_assert_uint_eq(size, 8);

	ipj_shm_release(handle);
} END_TEST

START_TEST(double_provide_sized) {
	static const char key[] = "hello";

	size_t size1 = 8, size2 = 16;
	const ssize_t
		handle1 = ipj_shm_provide(key, &size1, NULL),
		handle2 = ipj_shm_provide(key, &size2, NULL);

	ck_assert(handle1 != -1);
	ck_assert(handle1 == handle2);
	ck_assert(size1 == size2);

	ipj_shm_release(handle1);
	ipj_shm_release(handle2);
} END_TEST

START_TEST(alloc) {
	const ssize_t mem = ipj_shm_alloc(32);
	ck_assert(mem != -1);

	ipj_shm_release(mem);
} END_TEST

START_TEST(release) {
	static const char key[] = "hello";

	size_t size1 = 8;
	const ssize_t mem1 = ipj_shm_provide(key, &size1, NULL);
	ipj_shm_release(mem1);

	size_t size2 = 16;
	bool is_new2 = false;
	const ssize_t mem2 = ipj_shm_provide(key, &size2, &is_new2);

	ck_assert(mem2 != -1);
	ck_assert_uint_eq(size2, 16);
	ck_assert(is_new2);

	ipj_shm_release(mem2);
} END_TEST

START_TEST(resolve_valid) {
	void *const mem = ipj_shm_handle_resolve(handle);
	ck_assert_ptr_nonnull(mem);
} END_TEST

START_TEST(resolve_invalid) {
	void *const mem = ipj_shm_handle_resolve(-1);
	ck_assert_ptr_null(mem);
} END_TEST

int main(void) {
	ipj_shm_init();

	TCase *const base = tcase_create("base");
	tcase_add_test(base, provide_new);
	tcase_add_test(base, double_provide);
	tcase_add_test(base, provide_distinct);
	tcase_add_test(base, provide_sized);
	tcase_add_test(base, double_provide_sized);
	tcase_add_test(base, alloc);
	tcase_add_test(base, release);

	TCase *const io = tcase_create("io");
	tcase_add_checked_fixture(io, io_setup, io_teardown);
	tcase_add_test(io, resolve_valid);
	tcase_add_test(io, resolve_invalid);

	Suite *const s = suite_create("shmem");
	suite_add_tcase(s, base);
	suite_add_tcase(s, io);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	ipj_shm_cleanup();

	return fail_count;
}
