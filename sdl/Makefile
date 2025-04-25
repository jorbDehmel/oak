CC = gcc -O3
SUFFIX = -I/usr/include/SDL2 -D_GNU_SOURCE=1 -D_REENTRANT -L/usr/lib -lSDL2

sdl.o:	sdl.c | Makefile
	$(CC) -c -o $@ $< $(SUFFIX)
