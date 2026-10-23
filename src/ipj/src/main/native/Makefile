FIND?=find
CFLAGS+=-std=c11 -Wall -Wpedantic -Iinclude -Iinclude-private

demo/subst: $(patsubst %.c,%.o,$(wildcard src/share/*.c))

test/%: CFLAGS+=-g $(shell pkg-config --cflags check)
test/%: LDLIBS+=$(shell pkg-config --libs check)

ifeq ($(TEST_RUN),yes)
TESTS:=$(basename $(filter %.c,$(wildcard test/*)))
.PHONY: $(TESTS)
$(TESTS):
	$@
endif

test/allocator: src/share/allocator.o
test/obj: src/share/obj.o

clean:
	$(FIND) . \( -name "*.o" -or -name "*.exe" \) -delete
