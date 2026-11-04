#include <check.h>
#include "shmem.h"

ipj_shm_handle_t handle = IPJ_SHM_INVALID_HANDLE;
#define BUFSIZE 16

static void io_setup(void) { handle = ipj_shm_alloc(BUFSIZE); }
static void io_teardown(void) { ipj_shm_release(handle); }


START_TEST(provide_new) {
	bool is_new = false;

	const ipj_shm_handle_t
	handle = ipj_shm_provide("hello", NULL, &is_new);

	ck_assert(is_new);

	ipj_shm_release(handle);
} END_TEST

START_TEST(double_provide) {
	static const char key[] = "hello";

	bool is_new = true;
	const ipj_shm_handle_t
		mem1 = ipj_shm_provide(key, NULL, NULL),
		mem2 = ipj_shm_provide(key, NULL, &is_new);

	ck_assert(mem1 == mem2);
	ck_assert(!is_new);

	ipj_shm_release(mem1);
	ipj_shm_release(mem2);
} END_TEST

START_TEST(provide_distinct) {
	bool is_new = false;

	const ipj_shm_handle_t mem1 = ipj_shm_provide("hello", NULL, &is_new);
	ck_assert(is_new);

	const ipj_shm_handle_t mem2 = ipj_shm_provide("world", NULL, &is_new);
	ck_assert(is_new);

	ck_assert(mem1 != mem2);

	ipj_shm_release(mem1);
	ipj_shm_release(mem2);
} END_TEST

START_TEST(provide_sized) {
	size_t size = 8;
	const ipj_shm_handle_t mem = ipj_shm_provide("hello", &size, NULL);
	ck_assert(mem != IPJ_SHM_INVALID_HANDLE);
	ck_assert_uint_eq(size, 8);

	ipj_shm_release(mem);
} END_TEST

START_TEST(double_provide_sized) {
	static const char key[] = "hello";

	size_t size1 = 8, size2 = 16;
	const ipj_shm_handle_t
		mem1 = ipj_shm_provide(key, &size1, NULL),
		mem2 = ipj_shm_provide(key, &size2, NULL);

	ck_assert(mem1 != IPJ_SHM_INVALID_HANDLE);
	ck_assert(mem1 == mem2);
	ck_assert(size1 == size2);

	ipj_shm_release(mem1);
	ipj_shm_release(mem2);
} END_TEST

START_TEST(alloc) {
	const ipj_shm_handle_t mem = ipj_shm_alloc(32);
	ck_assert(mem != IPJ_SHM_INVALID_HANDLE);

	ipj_shm_release(mem);
} END_TEST

START_TEST(release) {
	static const char key[] = "hello";

	size_t size1 = 8;
	const ipj_shm_handle_t mem1 = ipj_shm_provide(key, &size1, NULL);
	ipj_shm_release(mem1);

	size_t size2 = 16;
	bool is_new2 = false;
	const ipj_shm_handle_t mem2 = ipj_shm_provide(key, &size2, &is_new2);

	ck_assert(mem2 != IPJ_SHM_INVALID_HANDLE);
	ck_assert_uint_eq(size2, 16);
	ck_assert(is_new2);

	ipj_shm_release(mem2);
} END_TEST

START_TEST(write_and_read) {
	static const char hello[] = "hello";

	ck_assert_int_eq(0, ipj_shm_write(handle, 0, hello, sizeof(hello)));

	char llo[4];
	ck_assert_int_eq(0, ipj_shm_read(handle, 2, llo, sizeof(llo)));

	ck_assert_str_eq(hello + 2, llo);
} END_TEST

START_TEST(write_by_invalid_handle) {
	ck_assert_int_eq(-1, ipj_shm_write(IPJ_SHM_INVALID_HANDLE, 0, NULL, 0));
} END_TEST

START_TEST(read_by_invalid_handle) {
	ck_assert_int_eq(-1, ipj_shm_read(IPJ_SHM_INVALID_HANDLE, 0, NULL, 0));
} END_TEST

START_TEST(write_too_long) {
	static const char long_string[] = "aasdgdfgha;rhtgpashdg;laskdhgjv";
	const ssize_t unwrite_count = ipj_shm_write(
		handle, 0, long_string, sizeof(long_string));
	ck_assert_int_eq(unwrite_count, sizeof(long_string) - BUFSIZE);
} END_TEST

START_TEST(read_too_long) {
	char long_string[BUFSIZE * 2];
	const ssize_t unread_count = ipj_shm_read(
		handle, 0, long_string, sizeof(long_string));
	ck_assert_int_eq(unread_count, sizeof(long_string) - BUFSIZE);
} END_TEST

START_TEST(write_by_huge_offset) {
	static const char hello[] = "hello";
	ck_assert_int_eq(sizeof(hello),
		ipj_shm_write(handle, BUFSIZE, hello, sizeof(hello)));
} END_TEST

START_TEST(read_by_huge_offset) {
	char buf[BUFSIZE];
	ck_assert_int_eq(sizeof(buf),
		ipj_shm_read(handle, BUFSIZE, buf, sizeof(buf)));
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
	tcase_add_test(io, write_and_read);
	tcase_add_test(io, write_by_invalid_handle);
	tcase_add_test(io, read_by_invalid_handle);
	tcase_add_test(io, write_too_long);
	tcase_add_test(io, read_too_long);
	tcase_add_test(io, write_by_huge_offset);
	tcase_add_test(io, read_by_huge_offset);

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
