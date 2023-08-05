# Jordan Dehmel, 2023, jdehmel@outlook.com, github.com/jorbDehmel/oak

CC := clang++ -pedantic -Wall
OBJS := build/lexer.o build/symbol-table.o \
	build/type-builder.o build/reconstruct.o \
	build/macros.o build/sequence.o \
	build/packages.o build/sizer.o build/op-sub.o \
	build/mem.o build/acorn_resources.o \
	build/document.o build/rules.o

HEADS := lexer.hpp reconstruct.hpp symbol-table.hpp \
	type-builder.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp op-sub.hpp \
	acorn_resources.hpp document.hpp rules.hpp

# -O3 is best for actual installs, not for testing
FLAGS := -O3

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

reinstall:
	$(MAKE) -C . uninstall clean install

test: bin/acorn.out
	acorn -e oak_demos/access_test.oak
	acorn -e oak_demos/cond_test.oak
	acorn -e oak_demos/conv_test.oak
	acorn -e oak_demos/hello_world.oak
	acorn -e oak_demos/loop_test.oak
	acorn -e oak_demos/macro_test.oak
	acorn -e oak_demos/math_test.oak
	acorn -e oak_demos/mem_test.oak
	acorn -e oak_demos/quine.oak
	acorn -e oak_demos/rec_test.oak
	acorn -e oak_demos/rule_test.oak
	acorn -e oak_demos/rule_test_2.oak
	acorn -e oak_demos/sdl_test.oak
	acorn -e oak_demos/thread_test.oak
	acorn -e oak_demos/gen_struct_test.oak
	acorn -e oak_demos/generic_test.oak

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) $(FLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf bin/* build/* *.o *.out *.log
