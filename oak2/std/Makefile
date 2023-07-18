all:	io.o mem.o float_math.o int_math.o misc_math.o

%.o:	%.cpp
	clang++ -c -o $@ $<

clean:
	rm *.o
