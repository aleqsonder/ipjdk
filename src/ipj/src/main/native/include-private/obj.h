#ifndef IPJ_OBJ_H
#define IPJ_OBJ_H

#include <stddef.h>
#include <string.h>

typedef struct ipj_obj ipj_obj_t;
extern const size_t IPJ_OBJ_HEAD_SIZE;

#define ipj_obj_size(PAYLOAD_SIZE, ID) \
	(IPJ_OBJ_HEAD_SIZE + (PAYLOAD_SIZE) + strlen(ID)+1)

ipj_obj_t *ipj_obj_markup(
	void *buf,
	size_t payload_size,
	const void *payload,
	const char *id);

char *ipj_obj_id(ipj_obj_t *);
const char *ipj_obj_id_const(const ipj_obj_t *);

// Returns payload size and sets *payload (if payload!=NULL)
size_t ipj_obj_payload(ipj_obj_t *, void **payload);
size_t ipj_obj_payload_const(const ipj_obj_t *, const void **payload);

#endif
