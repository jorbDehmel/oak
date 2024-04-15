FLAGS := -pedantic -Wall -O3 -fpic -static

all:
	$(MAKE) -j 8 conv.o file.o io.o \
		atom_math.o rand.o sys.o thread.o \
		time.o panic.o sock.o cstr.o string.o

%.o:	%.c
	clang $(FLAGS) -c -o $@ $<

%.o:	%.cpp
	clang++ $(FLAGS) -c -o $@ $<

clean:
	rm *.o
