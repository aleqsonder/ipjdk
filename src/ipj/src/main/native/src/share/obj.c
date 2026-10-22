#include <assert.h>
#include <string.h>

#include "obj.h"


size_t ipj_obj_full_size(size_t payload_size, const char *id) {
	assert(id);

	if (payload_size == 0)
		return 0;

	return sizeof(struct ipj_obj) + payload_size + strlen(id)+1;
}

void ipj_obj_init(
	struct ipj_obj *obj,
	size_t payload_size,
	const char *id
) {
	assert(obj);
	assert(id);

	*obj = (struct ipj_obj) {
		.refcount = 1,
		.payload_size = payload_size };

	char *const stored_id = ipj_obj_id(obj);
	strcpy(stored_id, id);
}
