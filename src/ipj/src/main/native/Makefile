FIND?=find
CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private

demo/subst: $(patsubst %.c,%.o,$(wildcard src/share/*.c))

test/%: CFLAGS+=-g $(shell pkg-config --cflags check)
test/%: LDLIBS+=$(shell pkg-config --libs check)
test/allocator: src/share/allocator.o

clean:
	$(FIND) . \( -name "*.o" -or -name "*.exe" \) -delete
