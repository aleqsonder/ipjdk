#ifndef IPJ_OBJ_H
#define IPJ_OBJ_H

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

extern const size_t IPJ_OBJ_MIN_SIZE;
#define ipj_obj_size(ID, PAYLOAD_SIZE) \
	(IPJ_OBJ_MIN_SIZE + ((ID) ? strlen(ID) : 0) + (PAYLOAD_SIZE))

typedef void ipj_obj_t;

// returns pointer to the actual payload
ipj_obj_t *ipj_obj_markup(void *buffer, const char *id, size_t payload_size);

void *ipj_obj_uncheckout(ipj_obj_t *);

bool ipj_obj_has_name(const ipj_obj_t *);
size_t ipj_obj_payload_size(const ipj_obj_t *);

char *ipj_obj_id(ipj_obj_t *);
const char *ipj_obj_id_const(const ipj_obj_t *);

void ipj_obj_addref(ipj_obj_t *);
bool ipj_obj_release(ipj_obj_t *); // returns refcount==0

#endif
