#include <stdint.h>
#include <assert.h>

#include "obj.h"

struct ipj_obj {
	uint32_t payload_size;
	uint8_t payload_and_id[];
};

const size_t IPJ_OBJ_HEAD_SIZE = sizeof(struct ipj_obj);

ipj_obj_t *ipj_obj_markup(
	void *buf,
	size_t payload_size,
	const void *payload,
	const char *id
) {
	assert(buf);
	assert(id);

	ipj_obj_t *const obj = buf;
	*obj = (ipj_obj_t) { .payload_size = payload_size };

	if (payload)
		memcpy(obj->payload_and_id, payload, payload_size);

	char *const stored_id = ipj_obj_id(obj);
	strcpy(stored_id, id);

	return obj;
}

char *ipj_obj_id(ipj_obj_t *obj) {
	assert(obj);
	return (char *)obj->payload_and_id + obj->payload_size;
}

const char *ipj_obj_id_const(const ipj_obj_t *obj) {
	assert(obj);
	return (char *)obj->payload_and_id + obj->payload_size;
}

size_t ipj_obj_payload(ipj_obj_t *obj, void **payload) {
	assert(obj);

	if (payload)
		*payload = obj->payload_and_id;
	return obj->payload_size;
}

size_t ipj_obj_payload_const(
	const ipj_obj_t *obj,
	const void **payload
) {
	assert(obj);

	if (payload)
		*payload = obj->payload_and_id;
	return obj->payload_size;
}
