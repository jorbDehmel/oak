################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
################################################################

CC := clang++

OBJS := build/acorn_resources.o \
	build/fn_resources.o build/generics.o build/lexer.o \
	build/op_sub.o build/packages.o build/reconstruct.o \
	build/rules.o build/sequence_resources.o build/sequence.o \
	build/type_builder.o

HEADS := lexer.hpp oakc_fns.hpp oakc_structs.hpp options.hpp \
	tags.hpp

FLAGS := -pedantic -Wall -O3

TEST := acorn

################################################################

.PHONY:	install
install: bin/acorn.out bin/oak2c.out std_oak_header.h
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.h /usr/include/oak
	sudo cp packages_list.txt /usr/include/oak

	sudo cp bin/acorn.out /usr/bin/acorn
	sudo cp bin/oak2c.out /usr/bin/oak2c
	sudo cp ../std/*.sh /usr/include/oak/

.PHONY:	packages
packages:
	sudo rm -rf /usr/include/oak/sdl /usr/include/oak/extra \
		/usr/include/oak/stl /usr/include/oak/turtle \
		/usr/include/oak/std
	sudo acorn -S ../std -S ../sdl -S ../extra -S ../stl -S \
		../turtle -e

.PHONY:	uninstall
uninstall:
	sudo rm -rf /usr/bin/acorn /usr/include/oak \
		/usr/include/std_oak_header.hpp

.PHONY:	reinstall
reinstall:
	$(MAKE) -C . uninstall clean install

################################################################

.PHONY:	dirs
dirs:
	mkdir -p bin
	mkdir -p build

README.pdf:	README.md
	pandoc README.md -o README.pdf

build/%.o:	%.cpp $(HEADS) Makefile
	$(CC) $(FLAGS) -c -o $@ $<

bin/acorn.out:	dirs build/acorn.o $(OBJS) $(HEADS) Makefile
	$(CC) $(FLAGS) -o $@ build/acorn.o $(OBJS)

bin/oak2c.out:	dirs build/oak2c.o $(OBJS) $(HEADS) Makefile
	$(CC) $(FLAGS) -o $@ build/oak2c.o $(OBJS)

################################################################

.PHONY:	clean
clean:
	rm -rf bin/* build/* *.o *.out *.log

################################################################
