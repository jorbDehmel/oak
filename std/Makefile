all:	io.o float_math.o int_math.o misc_math.o conv.o misc_std.o

%.o:	%.cpp
	clang++ -c -o $@ $<

clean:
	rm *.o
