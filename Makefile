# Jordan Dehmel, 2023, jdehmel@outlook.com, github.com/jorbDehmel/oak

CC := clang++ -pedantic -Wall
OBJS := build/lexer.o build/symbol-table.o \
	build/type-builder.o build/reconstruct.o \
	build/macros.o build/sequence.o \
	build/packages.o build/sizer.o build/op-sub.o \
	build/mem.o build/acorn_resources.o \
	build/document.o build/rules.o enums.o

HEADS := lexer.hpp reconstruct.hpp symbol-table.hpp \
	type-builder.hpp macros.hpp tags.hpp \
	sequence.hpp packages.hpp sizer.hpp op-sub.hpp \
	acorn_resources.hpp document.hpp rules.hpp \
	enums.hpp

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

test: install
	acorn -e oak_demos/enum_test.oak -o enum_test.out
	acorn -e oak_demos/access_test.oak -o access_test.out
	acorn -e oak_demos/cond_test.oak -o cond_test.out
	acorn -e oak_demos/conv_test.oak -o conv_test.out
	acorn -e oak_demos/hello_world.oak -o hello_world.out
	acorn -e oak_demos/loop_test.oak -o loop_test.out
	acorn -e oak_demos/macro_test.oak -o macro_test.out
	acorn -e oak_demos/math_test.oak -o math_test.out
	acorn -e oak_demos/mem_test.oak -o mem_test.out
	acorn -e oak_demos/quine.oak -o quine.out
	acorn -e oak_demos/rec_test.oak -o rec_test.out
	acorn -e oak_demos/rule_test.oak -o rule_test.out
	acorn -e oak_demos/rule_test_2.oak -o rule_test_2.out
	acorn -e oak_demos/sdl_test.oak -o sdl_test.out
	acorn -e oak_demos/gen_struct_test.oak -o gen_struct_test.out
	acorn -e oak_demos/generic_test.oak -o generic_test.out
	acorn -e oak_demos/file_test.oak -o file_test.out
	acorn -ue oak_demos/i_file_test.oak -o i_file_test.out
	acorn -e oak_demos/fn_ptr_test.oak -o fn_ptr_test.out
	acorn -e oak_demos/thread_test.oak -o thread_test.out
	acorn -e oak_demos/fn_ptr_test_2.oak -o fn_ptr_test_2.out
	acorn -e oak_demos/erase_test.oak -o erase_test.out
	rm -rf *.log .oak_build

build/%.o:	%.cpp $(HEADS)
	mkdir -p build
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.out:	build/%.o $(OBJS) $(HEADS)
	mkdir -p bin
	$(CC) $(FLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf bin/* build/* *.o *.out *.log
