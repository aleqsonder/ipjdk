#include <stdint.h>
#include <assert.h>

#include "obj.h"

struct ipj_obj_head {
	uint32_t payload_size;
	uint8_t payload_and_id[];
};

const size_t IPJ_OBJ_MIN_SIZE = sizeof(struct ipj_obj_head);

#define OBJ_HEAD_GET(OBJ_PAYLOAD) \
	((struct ipj_obj_head *)((uint8_t *)OBJ_PAYLOAD \
		- offsetof(struct ipj_obj_head, payload_and_id)))

#define OBJ_ID_GET(OBJ_HEAD) \
	((OBJ_HEAD)->payload_and_id + (OBJ_HEAD)->payload_size)

void *ipj_obj_markup(
	void *buf,
	const char *id,
	size_t payload_size
) {
	assert(buf);
	assert(id);

	struct ipj_obj_head *const obj_head = buf;
	*obj_head = (struct ipj_obj_head) { .payload_size = payload_size };

	strcpy((char *)OBJ_ID_GET(obj_head), id);
	return obj_head->payload_and_id;
}

char *ipj_obj_id(void *obj) {
	assert(obj);
	return (char *)OBJ_ID_GET(OBJ_HEAD_GET(obj));
}

const char *ipj_obj_id_const(const void *obj) {
	assert(obj);
	return (const char *)OBJ_ID_GET(OBJ_HEAD_GET(obj));
}
