FIND?=find
CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private

JNI_CFLAGS:=-D_REENTRANT -I"$(JAVA_HOME)"/include
JNI_CFLAGS_DARWIN:=$(JNI_CFLAGS) -I"$(JAVA_HOME)"/include/darwin
JNI_CFLAGS_WIN32:=-$(JNI_CFLAGS) -I"$(JAVA_HOME)"/include/win32

libipjshmem.dylib ipjshmem.dll: $(patsubst %.c,%.o,$(wildcard src/share/*.c))

libipjshmem.dylib: CFLAGS+=$(JNI_CFLAGS_DARWIN)
libipjshmem.dylib: src/posix/shmmap.o

ipjshmem.dll: CFLAGS+=$(JNI_CFLAGS_WIN32)
ipjshmem.dll: src/windows/shmmap.o

%.dylib %.dll:
	$(CC) $(CFLAGS) -shared -o $@ $(LDLIBS) $^

test/%: CFLAGS+=-g $(shell pkg-config --cflags check)
test/%: LDLIBS+=$(shell pkg-config --libs check)
test/allocator: src/share/allocator.o
test/hotel: src/share/hotel.o
test/strset: $(addprefix src/share/,strset.o allocator.o)

test/shmem test/shmem.exe: $(addprefix src/share/,strset.o hotel.o allocator.o shmem.o layout.o)
test/shmem: src/posix/shmmap.o
test/shmem.exe: src/windows/shmmap.o

clean:
	$(FIND) . \( -name "*.o" -or -name "*.exe" \) -delete
