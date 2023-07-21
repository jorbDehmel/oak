CC := clang++ -pedantic -Wall
OBJS := build/lexer.o build/symbol-table.o \
	build/type-builder.o build/reconstruct.o \
	build/macros.o build/sequence.o \
	build/packages.o build/sizer.o build/op-sub.o \
	build/mem.o

HEADS := lexer.hpp reconstruct.hpp symbol-table.hpp \
	type-builder.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp op-sub.hpp

all: bin/acorn.out

install: bin/acorn.out std_oak_header.hpp
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.hpp /usr/include
	sudo cp packages_list.txt /usr/include/oak
	sudo cp -r std /usr/include/oak

	sudo $(MAKE) -C /usr/include/oak/std
	sudo rm -f /usr/include/oak/std/*.cpp
	sudo rm -f /usr/include/oak/std/Makefile

	sudo cp bin/acorn.out /usr/bin/acorn

uninstall:
	sudo rm -rf /usr/bin/acorn /usr/include/oak /usr/include/std_oak_header.hpp

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) -o $@ $< $(OBJS)

clean:
	rm -rf bin/* build/* *.o *.out *.log
