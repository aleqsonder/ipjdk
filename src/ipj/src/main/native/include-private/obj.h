#ifndef IPJ_OBJ_H
#define IPJ_OBJ_H

#include <stdint.h>
#include <stdbool.h>

struct ipj_obj {
	uint16_t refcount, payload_size;
	uint8_t payload_and_id[];
};

size_t ipj_obj_full_size(size_t payload_size, const char *id);
void ipj_obj_init(struct ipj_obj *, size_t payload_size, const char *id);

struct ipj_obj ipj_obj_add_ref(struct ipj_obj *);

// returns true on refcount==0
bool ipj_obj_release(struct ipj_obj *);

char *ipj_obj_id(const struct ipj_obj *obj);

#endif
