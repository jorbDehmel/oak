################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
################################################################

CC := clang++

OBJS := build/lexer.o build/symbol_table.o \
	build/type_builder.o build/reconstruct.o build/macros.o \
	build/sequence.o build/packages.o build/sizer.o \
	build/op_sub.o build/mem.o build/acorn_resources.o \
	build/document.o build/rules.o build/mangler.o \
	build/generics.o build/sequence_resources.o

HEADS := lexer.hpp reconstruct.hpp symbol_table.hpp \
	type_builder.hpp macros.hpp tags.hpp sequence.hpp \
	packages.hpp sizer.hpp op_sub.hpp acorn_resources.hpp \
	document.hpp rules.hpp mangler.hpp generics.hpp \
	sequence_resources.hpp rule_structs.hpp options.hpp

FLAGS := -pedantic -Wall -O3

TEST := acorn

################################################################

.PHONY:	install
install: bin/acorn.out std_oak_header.h
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.h /usr/include/oak
	sudo cp packages_list.txt /usr/include/oak

	sudo cp bin/acorn.out /usr/bin/acorn

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

.PHONY:	test
test:
	$(TEST) -eTT

################################################################

.PHONY:	dirs
dirs:
	mkdir -p bin
	mkdir -p build

README.pdf:	README.md
	pandoc README.md -o README.pdf

build/%.o:	%.cpp $(HEADS)
	$(CC) $(FLAGS) -c -o $@ $<

bin/acorn.out:	dirs build/acorn.o $(OBJS) $(HEADS)
	$(CC) $(FLAGS) -o $@ build/acorn.o $(OBJS)

################################################################

.PHONY:	clean
clean:
	rm -rf bin/* build/* *.o *.out *.log

################################################################
