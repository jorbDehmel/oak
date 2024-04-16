all:	ptrcmp.o

%.o:	%.c
	clang -std=c17 -c -static -O3 -fpic $^ -o $@

clean:
	rm *.o
