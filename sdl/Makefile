all:	sdl_interface.o events.o

%.o:	%.cpp
	clang++ -I/usr/include/SDL2 -D_REENTRANT -c -o $@ $<

clean:
	rm *.o
