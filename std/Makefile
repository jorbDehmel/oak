all:	conv_inter.o file_inter.o io_inter.o \
		math_float_inter.o math_int_inter.o math_misc_inter.o \
		rand_inter.o sys_inter.o thread_inter.o time_inter.o \
		panic_inter.o

%.o:	%.cpp
	clang++ -c -o $@ $<

clean:
	rm *.o
