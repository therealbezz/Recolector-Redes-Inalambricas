CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
CPPFLAGS += -Iinclude
PYTHON = python3

OBJECTS = build/main.o build/menu.o build/input.o build/collector.o build/display.o

.PHONY: all clean package

all: main

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

build:
	mkdir -p $@

build/%.o: src/%.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

package:
	$(PYTHON) tools/package.py

clean:
	rm -f main main.exe $(OBJECTS) $(OBJECTS:.o=.d)
