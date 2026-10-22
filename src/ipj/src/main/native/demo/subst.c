#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "obj-map.h"

#define BUFSIZE 512

static uint8_t mapbuf[1024], objbuf[1024];

int main(int argc, char **argv) {
	struct ipj_allocator *allocator = (struct ipj_allocator *)objbuf;
	ipj_allocator_init(allocator, sizeof(objbuf));

	struct ipj_obj_map map = {
		.allocator = allocator,
		.shared_state = (struct ipj_obj_map_shared_state *)mapbuf };
	ipj_obj_map_shared_state_init(map.shared_state, sizeof(mapbuf));

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		char *const delim = strchr(arg, '=');
		if (delim == NULL) {
			fprintf(stderr,
				"%s: malformed argument (should be a=b)\n", arg);
			return EXIT_FAILURE;
		}

		*delim = '\0';

		const char
			*const key = arg,
			*const value = delim + 1;

		struct ipj_obj *const
			obj = ipj_obj_map_provide(&map, key, strlen(value)+1);

		strcpy(ipj_obj_payload(obj), value);
	}

	char buf[BUFSIZE];
	while (fgets(buf, BUFSIZE, stdin)) {
		const char *id = buf;
		for (size_t i = 0; i < BUFSIZE && buf[i]; ++i) {
			const char c = buf[i];
			switch ((id == NULL) + isalpha(c)) {
			case 0:
				buf[i] = '\0';

				const struct ipj_obj *const
				obj = ipj_obj_map_search(&map, id);

				const char *value = obj
					? (char *)ipj_obj_payload(obj)
					: id;

				printf("%s%c", value, c);
				id = NULL;

				buf[i] = c;

				break;
			case 2:
				id = buf + i;
				// no break
			default:
				fputc(c, stdout);
			}
		}
	}

	return EXIT_SUCCESS;
}
