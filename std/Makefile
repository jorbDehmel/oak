FLAGS := -pedantic -Wall -O3 -fpic -static

all:
	$(MAKE) -j 8 conv_inter.o file_inter.o io_inter.o \
		math_float_inter.o math_int_inter.o math_misc_inter.o \
		rand_inter.o sys_inter.o thread_inter.o time_inter.o \
		panic_inter.o string.o sock_inter.o cstr.o

%.o:	%.c
	clang $(FLAGS) -c -o $@ $<

%.o:	%.cpp
	clang++ $(FLAGS) -c -o $@ $<

clean:
	rm *.o
