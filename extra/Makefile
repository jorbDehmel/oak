FLAGS := -pedantic -Wall -O3 -fpic -static

all:
	$(MAKE) -j 2 regex_inter.o bit_print.o

%.o:	%.c
	clang $(FLAGS) -c -o $@ $<

%.o:	%.cpp
	clang++ $(FLAGS) -c -o $@ $<

clean:
	rm *.o
