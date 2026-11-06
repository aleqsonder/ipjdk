FIND?=find
CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private
JNI_CFLAGS:=-D_REENTRANT -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/win32

ipjshmem.dll: CFLAGS+=$(JNI_CFLAGS)
ipjshmem.dll: $(patsubst %.c,%.o,$(wildcard src/share/*.c)) src/windows/shmmap.o

%.dll:
	$(CC) $(CFLAGS) -shared -o $@ $(LDLIBS) $^

test/%: CFLAGS+=-g $(shell pkg-config --cflags check)
test/%: LDLIBS+=$(shell pkg-config --libs check)
test/allocator: src/share/allocator.o
test/map_u32: $(addprefix src/share/,map_u32.o allocator.o)
test/shmem: $(addprefix src/share/,map_u32.o allocator.o shmem.o) src/windows/shmmap.o

clean:
	$(FIND) . \( -name "*.o" -or -name "*.exe" \) -delete
