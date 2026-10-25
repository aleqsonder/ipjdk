#ifndef IPJ_OBJ_H
#define IPJ_OBJ_H

#include <stddef.h>
#include <string.h>

extern const size_t IPJ_OBJ_MIN_SIZE;
#define ipj_obj_size(ID, PAYLOAD_SIZE) \
	(IPJ_OBJ_MIN_SIZE + strlen(ID)+1 + (PAYLOAD_SIZE))

// returns pointer to the actual payload
void *ipj_obj_markup(void *buf, const char *id, size_t payload_size);

size_t ipj_obj_payload_size(const void *);

char *ipj_obj_id(void *);
const char *ipj_obj_id_const(const void *);

#endif
