CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
OBJECTS = main.o input.o collector.o display.o
PYTHON = python3

.PHONY: all clean test package

all: main

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.c main.h collector.h input.h display.h
input.o: input.c input.h collector.h
collector.o: collector.c collector.h input.h
display.o: display.c display.h collector.h

build/test_collector: tests/test_collector.c tests/test_collector.h \
		collector.c collector.h input.c input.h
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -I. -o $@ tests/test_collector.c \
		collector.c input.c $(LDLIBS)

test: main build/test_collector
	$(PYTHON) tests/run_tests.py

package:
	$(PYTHON) tools/package.py

clean:
	rm -f main main.exe $(OBJECTS) build/test_collector
