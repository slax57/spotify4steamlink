CC=gcc
RM=rm
CFLAGS=-I/usr/include/SDL2 -D_REENTRANT
LDFLAGS=-lSDL2 -lSDL2_ttf
EXEC=main
SRC= $(wildcard source/*.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

main: $(OBJ) 
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	$(RM) source/*.o

mrproper: clean
	$(RM) $(EXEC) 
