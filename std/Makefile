CC = gcc -O3
OBJECTS = io.o system.o random.o panic.o

.PHONY:	all
all:	$(OBJECTS)

%.o:	%.c
	$(CC) -c -o $@ $<
