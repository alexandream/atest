LIBATEST_SOURCES=src/atest.c

LIBATEST_OBJS=$(LIBATEST_SOURCES:src/%.c=build/%.o)

COMPILE_FLAGS=-pedantic -Wall -std=c89 $(CFLAGS)

.PHONY: all
all: dist/libatest.a dist/atest.h

dist/libatest.a: $(LIBATEST_OBJS) dist
	ar rcs $@ $(LIBATEST_OBJS)


dist/atest.h: src/atest.h dist
	cp $< $@

build/%.o: src/%.c build
	$(CC) -c $< -o $@ $(COMPILE_FLAGS)

dist:
	mkdir -p dist

build:
	mkdir -p build

.PHONY: clean
clean:
	rm -f build/*
	rm -f dist/*

