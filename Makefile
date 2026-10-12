CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
OBJECTS = main.o input.o collector.o display.o

.PHONY: all clean test package

all: main

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.c main.h collector.h input.h display.h
input.o: input.c input.h collector.h
collector.o: collector.c collector.h input.h
display.o: display.c display.h collector.h

clean:
	rm -f main main.exe $(OBJECTS)
