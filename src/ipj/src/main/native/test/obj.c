#include <check.h>
#include "obj.h"

#define BUFSIZE 16
uint8_t buf[BUFSIZE];
ipj_obj_t *obj = NULL;

const char obj_id[] = "hello", obj_payload[] = "world";

void setup(void) {
	obj = ipj_obj_markup(buf, obj_id, sizeof(obj_payload));
}

START_TEST(size) {
	const size_t obj_size = ipj_obj_size(obj_id, sizeof(obj_payload));
	ck_assert_int_le(obj_size, BUFSIZE);
} END_TEST

START_TEST(unmarkup) {
	uint8_t *const buf_restored = ipj_obj_unmarkup(obj);
	ck_assert_ptr_eq(buf_restored, buf);
} END_TEST

START_TEST(payload_size) {
	const size_t size = ipj_obj_payload_size(obj);
	ck_assert_int_eq(size, sizeof(obj_payload));
} END_TEST

START_TEST(id) {
	char *const restored_id = ipj_obj_id(obj);
	ck_assert_str_eq(obj_id, restored_id);
} END_TEST

START_TEST(id_const) {
	const char *const restored_id = ipj_obj_id_const(obj);
	ck_assert_str_eq(obj_id, restored_id);
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("main");
	tcase_add_checked_fixture(tc, setup, NULL);
	tcase_add_test(tc, size);
	tcase_add_test(tc, unmarkup);
	tcase_add_test(tc, payload_size);
	tcase_add_test(tc, id);
	tcase_add_test(tc, id_const);

	Suite *const s = suite_create("main");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
