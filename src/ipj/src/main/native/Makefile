CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private

demo/subst: $(patsubst %.c,%.o,$(wildcard src/share/*.c))
