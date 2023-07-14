CC := clang++ -pedantic -Wall
OBJS := build/lexer.o build/symbol-table.o \
	build/type-builder.o build/reconstruct.o \
	build/auto-oak.o build/macros.o build/sequence.o \
	build/packages.o build/sizer.o

HEADS := lexer.hpp reconstruct.hpp symbol-table.hpp \
	type-builder.hpp auto-oak.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp

all: bin/struct_test.out bin/reconstruct_test.out \
	bin/acorn.out bin/sequence_test.out

install: bin/acorn.out std_oak_header.hpp
	sudo mkdir -p /usr/include/oak
	
	sudo cp std_oak_header.hpp /usr/include/oak
	sudo cp -r std /usr/include/oak
	$(MAKE) -C /usr/include/oak/std
	sudo rm -f /usr/include/oak/std/*.cpp
	sudo rm -f /usr/include/oak/std/Makefile

	sudo cp bin/acorn.out /usr/bin/acorn

uninstall:
	sudo rm -rf /usr/bin/acorn /usr/include/oak

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) -o $@ $< $(OBJS)

bin/sequence_test.out:	build/sequence_test.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) -o bin/sequence_test.out build/sequence_test.o $(OBJS)

clean:
	rm -rf bin/* build/*
