#include <stdint.h>
#include <assert.h>

#include "obj.h"

struct ipj_obj_head {
	bool has_name;
	uint32_t refcount, payload_size;
	uint8_t payload_and_id[];
};
const size_t IPJ_OBJ_MIN_SIZE = sizeof(struct ipj_obj_head) + 1;

#define OBJ_HEAD_GET(OBJ_PAYLOAD) ((struct ipj_obj_head *) \
	((uint8_t *)OBJ_PAYLOAD - sizeof(struct ipj_obj_head)))

#define OBJ_ID_GET(OBJ_HEAD) \
	((OBJ_HEAD)->payload_and_id + (OBJ_HEAD)->payload_size)

ipj_obj_t *ipj_obj_markup(
	void *buf,
	const char *id,
	size_t payload_size
) {
	assert(buf);

	const bool has_name = id != NULL;

	struct ipj_obj_head *const obj_head = buf;
	*obj_head = (struct ipj_obj_head) {
		.has_name = has_name,
		.refcount = 0,
		.payload_size = payload_size };

	if (has_name)
		strcpy((char *)OBJ_ID_GET(obj_head), id);
	else
		*OBJ_ID_GET(obj_head) = '\0';

	return obj_head->payload_and_id;
}

void *ipj_obj_uncheckout(ipj_obj_t *obj) {
	assert(obj);
	return OBJ_HEAD_GET(obj);
}

bool ipj_obj_has_name(const ipj_obj_t *obj) {
	assert(obj);
	return OBJ_HEAD_GET(obj)->has_name;
}

size_t ipj_obj_payload_size(const ipj_obj_t *obj) {
	assert(obj);
	return OBJ_HEAD_GET(obj)->payload_size;
}

char *ipj_obj_id(ipj_obj_t *obj) {
	assert(obj);
	struct ipj_obj_head *const head = OBJ_HEAD_GET(obj);
	return (char *)OBJ_ID_GET(head);
}

const char *ipj_obj_id_const(const ipj_obj_t *obj) {
	assert(obj);
	const struct ipj_obj_head *const head = OBJ_HEAD_GET(obj);
	return (const char *)OBJ_ID_GET(head);
}

void ipj_obj_addref(ipj_obj_t *obj) {
	assert(obj);
	OBJ_HEAD_GET(obj)->refcount++;
}

bool ipj_obj_release(ipj_obj_t *obj) {
	assert(obj);
	struct ipj_obj_head *const head = OBJ_HEAD_GET(obj);
	if (head->refcount > 0)
		--head->refcount;
	return head->refcount == 0;
}
