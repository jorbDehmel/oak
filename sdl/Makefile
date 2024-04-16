all:
	$(MAKE) -j 2 sdl_inter.o event_inter.o

%.o:	%.cpp
	clang++ -std=c++17 -I/usr/include/SDL2 -D_REENTRANT -c -o $@ $<

%.o:	%.c
	clang -std=gnu11 -I/usr/include/SDL2 -D_REENTRANT -c -o $@ $<

clean:
	rm *.o
