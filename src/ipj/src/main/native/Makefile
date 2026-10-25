FIND?=find
CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private

demo/subst: $(patsubst %.c,%.o,$(wildcard src/share/*.c))

test/%: CFLAGS+=-g $(shell pkg-config --cflags check)
test/%: LDLIBS+=$(shell pkg-config --libs check)
test/allocator: src/share/allocator.o
test/obj: src/share/obj.o
test/objmap: $(addprefix src/share/,objmap.o allocator.o obj.o)
test/shmem: $(addprefix src/share/,objmap.o allocator.o obj.o) src/windows/shmem.o

clean:
	$(FIND) . \( -name "*.o" -or -name "*.exe" \) -delete
