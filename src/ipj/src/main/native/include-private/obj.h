#ifndef IPJ_OBJ_H
#define IPJ_OBJ_H

#include <stddef.h>
#include <string.h>

extern const size_t IPJ_OBJ_MIN_SIZE;
#define ipj_obj_size(ID, PAYLOAD_SIZE) \
	(IPJ_OBJ_MIN_SIZE + strlen(ID)+1 + (PAYLOAD_SIZE))

typedef void ipj_obj_t;

// returns pointer to the actual payload
ipj_obj_t *ipj_obj_markup(void *buffer, const char *id, size_t payload_size);

// returns pointer to the buffer given to ipj_obj_markup
void *ipj_obj_unmarkup(ipj_obj_t *);

size_t ipj_obj_payload_size(const ipj_obj_t *);

char *ipj_obj_id(ipj_obj_t *);
const char *ipj_obj_id_const(const ipj_obj_t *);

#endif
