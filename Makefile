CC=gcc
RM=rm
CFLAGS=-I/usr/include/SDL2 -D_REENTRANT
LDFLAGS=-lSDL2 -lSDL2_ttf

all: main onevent

main: source/main.o
	$(CC) -o main source/main.o $(LDFLAGS)

source/main.o: source/main.c
	$(CC) -o source/main.o -c source/main.c $(CFLAGS)

onevent: source/onevent.o
	$(CC) -o onevent source/onevent.o $(LDFLAGS)

source/onevent.o: source/onevent.c
	$(CC) -o source/onevent.o -c source/onevent.c $(CFLAGS)

.PHONY: clean mrproper

clean:
	$(RM) source/main.o source/onevent.o

mrproper: clean
	$(RM) main onevent
