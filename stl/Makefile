all:	ptrcmp.o

%.o:	%.c
	clang -std=gnu11 -c -static -O3 -fpic $^ -o $@

clean:
	rm *.o
