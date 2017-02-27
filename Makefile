LIBATEST_SOURCES=src/atest.c
LIBATCHECK_SOURCES=src/atallocf.c src/atcheck.c
ATREPORT_SOURCES=src/atreport.c

LIBATEST_OBJS=$(LIBATEST_SOURCES:src/%.c=build/%.o)
LIBATCHECK_OBJS=$(LIBATCHECK_SOURCES:src/%.c=build/%.o)
ATREPORT_OBJS=$(ATREPORT_SOURCES:src/%.c=build/%.o)

COMPILE_FLAGS=-pedantic -Wall $(CFLAGS)

.PHONY: all
all: dist build dist/libatest.a dist/atest.h \
     dist/libatcheck.a dist/atcheck.h dist/atreport


dist/libatest.a: $(LIBATEST_OBJS)
	ar rcs $@ $(LIBATEST_OBJS)


dist/atest.h: src/atest.h
	cp $< $@


dist/libatcheck.a: $(LIBATCHECK_OBJS)
	ar rcs $@ $(LIBATCHECK_OBJS)


dist/atcheck.h: src/atcheck.h
	cp $< $@


dist/atreport: $(ATREPORT_OBJS)
	$(CC) -o $@ $(ATREPORT_OBJS)

build/%.o: src/%.c
	$(CC) -c $< -o $@ $(COMPILE_FLAGS)


dist:
	mkdir -p dist


build:
	mkdir -p build


.PHONY: clean
clean:
	rm -f build/*
	rm -f dist/*

