CC := clang --pedantic -Wall --static -fpic -O3

all: hex_ops.o

%.o:	%.c
	$(CC) -c $< -o $@
