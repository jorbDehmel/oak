all:	ptrcmp.o

%.o:	%.c
	clang -c -static -O3 -fpic $^ -o $@

clean:
	rm *.o
