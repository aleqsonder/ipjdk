#include <check.h>
#include "obj.h"

#define BUFSIZE 32
uint8_t buf[BUFSIZE];
ipj_obj_t *obj = NULL;

const char obj_id[] = "hello", obj_payload[] = "world";

void setup_named(void) {
	obj = ipj_obj_markup(buf, obj_id, sizeof(obj_payload));
	ipj_obj_addref(obj);
	strcpy(obj, obj_payload);
}

void setup_anon(void) {
	obj = ipj_obj_markup(buf, NULL, sizeof(obj_payload));
	ipj_obj_addref(obj);
	strcpy(obj, obj_payload);
}

START_TEST(size) {
	const size_t obj_size = ipj_obj_size(obj_id, sizeof(obj_payload));
	ck_assert_int_le(obj_size, BUFSIZE);
} END_TEST

START_TEST(uncheckout) {
	uint8_t *const buf_restored = ipj_obj_uncheckout(obj);
	ck_assert_ptr_eq(buf_restored, buf);
} END_TEST

START_TEST(has_name) {
	ck_assert(ipj_obj_has_name(obj));
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

START_TEST(anon_has_name) {
	ck_assert(!ipj_obj_has_name(obj));
} END_TEST

int main(void) {
	TCase *const named_tc = tcase_create("named");
	tcase_add_checked_fixture(named_tc, setup_named, NULL);
	tcase_add_test(named_tc, size);
	tcase_add_test(named_tc, uncheckout);
	tcase_add_test(named_tc, has_name);
	tcase_add_test(named_tc, payload_size);
	tcase_add_test(named_tc, id);
	tcase_add_test(named_tc, id_const);

	TCase *const anon_tc = tcase_create("anon");
	tcase_add_checked_fixture(anon_tc, setup_anon, NULL);
	tcase_add_test(anon_tc, anon_has_name);

	Suite *const s = suite_create("main");
	suite_add_tcase(s, named_tc);
	suite_add_tcase(s, anon_tc);

	SRunner *const sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	const int fail_count = srunner_ntests_failed(sr);
	srunner_free(sr);

	return fail_count;
}
