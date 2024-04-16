FLAGS := -pedantic -Wall -O3 -fpic -static

all:
	$(MAKE) -j 4 regex_inter.o bit_print.o math.o

%.o:	%.c
	clang -std=gnu11 $(FLAGS) -c -o $@ $<

%.o:	%.cpp
	clang++ -std=c++17 $(FLAGS) -c -o $@ $<

regex_inter.o:	regex_inter.cpp
	clang++ -std=c++17 $(FLAGS) -c -o regex_inter.o regex_inter.cpp

clean:
	rm *.o
