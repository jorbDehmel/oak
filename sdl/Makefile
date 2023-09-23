all:	sdl_inter.o event_inter.o

%.o:	%.cpp
	clang++ -I/usr/include/SDL2 -D_REENTRANT -c -o $@ $<

tester.out:
	clang++ sdl_key_tester.cpp -o tester.out -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib -lSDL2

clean:
	rm *.o
