################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
################################################################

CC := clang++

OBJS := build/lexer.o build/symbol_table.o \
	build/type_builder.o build/reconstruct.o \
	build/macros.o build/sequence.o \
	build/packages.o build/sizer.o build/op_sub.o \
	build/mem.o build/acorn_resources.o \
	build/document.o build/rules.o build/enums.o \
	build/mangler.o build/generics.o \
	build/sequence_resources.o

HEADS := lexer.hpp reconstruct.hpp symbol_table.hpp \
	type_builder.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp op_sub.hpp \
	acorn_resources.hpp document.hpp rules.hpp \
	enums.hpp mangler.hpp generics.hpp sequence_resources.hpp

FLAGS := -pedantic -Wall -O3

TEST := acorn

################################################################

install: bin/acorn.out std_oak_header.h
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.h /usr/include/oak
	sudo cp packages_list.txt /usr/include/oak
	sudo cp -r std /usr/include/oak

	sudo $(MAKE) -C /usr/include/oak/std
	sudo rm -f /usr/include/oak/std/*.cpp
	sudo rm -f /usr/include/oak/std/Makefile

	sudo cp bin/acorn.out /usr/bin/acorn

	sudo mv /usr/include/oak/std/*.sh /usr/include/oak

packages:
	acorn -S sdl -S experimental/extra

uninstall:
	sudo rm -rf /usr/bin/acorn /usr/include/oak \
		/usr/include/std_oak_header.hpp

reinstall:
	$(MAKE) -C . uninstall clean install

test:
	$(TEST) -eTT

################################################################

README.pdf:	README.md
	pandoc README.md -o README.pdf

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) $(FLAGS) -o $@ $< $(OBJS)

bin/acorn.out:	build/acorn.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) $(FLAGS) -o $@ $< $(OBJS)

################################################################

tclean:
	rm -rf *.out

clean:
	rm -rf bin/* build/* *.o *.out *.log

################################################################
