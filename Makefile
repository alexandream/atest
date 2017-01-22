LIBATEST_SOURCES=src/atest.c
LIBATCHECK_SOURCES=src/atallocf.c src/atcheck.c

LIBATEST_OBJS=$(LIBATEST_SOURCES:src/%.c=build/%.o)
LIBATCHECK_OBJS=$(LIBATCHECK_SOURCES:src/%.c=build/%.o)

COMPILE_FLAGS=-pedantic -Wall $(CFLAGS)

.PHONY: all
all: dist/libatest.a dist/atest.h dist/libatcheck.a dist/atcheck.h


dist/libatest.a: $(LIBATEST_OBJS) dist
	ar rcs $@ $(LIBATEST_OBJS)


dist/atest.h: src/atest.h dist
	cp $< $@


dist/libatcheck.a: $(LIBATCHECK_OBJS) dist
	ar rcs $@ $(LIBATCHECK_OBJS)


dist/atcheck.h: src/atcheck.h
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

