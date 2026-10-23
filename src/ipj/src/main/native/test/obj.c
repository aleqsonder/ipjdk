#include <check.h>
#include "obj.h"

#define BUFSIZE 16
uint8_t buf[BUFSIZE];
ipj_obj_t *obj = NULL;

const char obj_id[] = "hello", obj_payload[] = "world";

void setup(void) {
	obj = ipj_obj_markup(buf, sizeof(obj_payload), obj_payload, obj_id);
}

START_TEST(size) {
	const size_t obj_size = ipj_obj_size(sizeof(obj_payload), obj_id);
	ck_assert_int_le(obj_size, BUFSIZE);
} END_TEST

START_TEST(id) {
	char *const restored_id = ipj_obj_id(obj);
	ck_assert_str_eq(obj_id, restored_id);
} END_TEST

START_TEST(id_const) {
	const char *const restored_id = ipj_obj_id_const(obj);
	ck_assert_str_eq(obj_id, restored_id);
} END_TEST

START_TEST(payload) {
	char *restored_payload = NULL;
	const size_t restored_payload_size = ipj_obj_payload(obj,
		(void **)&restored_payload);

	ck_assert_int_eq(sizeof(obj_payload), restored_payload_size);
	ck_assert_str_eq(obj_payload, restored_payload);
} END_TEST

START_TEST(payload_const) {
	const char *restored_payload = NULL;
	const size_t restored_payload_size = ipj_obj_payload_const(obj,
		(const void **)&restored_payload);

	ck_assert_int_eq(sizeof(obj_payload), restored_payload_size);
	ck_assert_str_eq(obj_payload, restored_payload);
} END_TEST

int main(void) {
	TCase *const tc = tcase_create("main");
	tcase_add_checked_fixture(tc, setup, NULL);
	tcase_add_test(tc, size);
	tcase_add_test(tc, id);
	tcase_add_test(tc, id_const);
	tcase_add_test(tc, payload);
	tcase_add_test(tc, payload_const);

	Suite *const s = suite_create("main");
	suite_add_tcase(s, tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
