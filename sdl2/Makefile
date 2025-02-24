CC = gcc -O3
OBJECTS = sdl.o

.PHONY:	all
all:	$(OBJECTS)

%.o:	%.c
	$(CC) -c -o $@ $<
