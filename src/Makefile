################################################################
# Jordan Dehmel, 2023 - present
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
################################################################

CC := clang++ -std=c++17 -g

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
install: bin/acorn.out bin/oak2c.out \
		std_oak_header.h
	sudo mkdir -p /usr/include/oak

	sudo cp std_oak_header.h /usr/include/oak
	sudo cp packages_list.txt /usr/include/oak

	sudo cp bin/acorn.out /usr/bin/acorn
	sudo cp bin/oak2c.out /usr/bin/oak2c
	sudo cp ../std/*.sh /usr/include/oak/

.PHONY:	install-db
install-db:	install bin/acorn-db.out bin/oak2c-db.out \
		std_oak_header.h

	sudo cp bin/acorn-db.out /usr/bin/acorn-db
	sudo cp bin/oak2c-db.out /usr/bin/oak2c-db

# Shared object (static library) file for use in unit testing.
.PHONY:	so
so:	bin/oakc.so

.PHONY:	packages
packages:
	sudo rm -rf /usr/include/oak/sdl /usr/include/oak/extra \
		/usr/include/oak/stl /usr/include/oak/turtle \
		/usr/include/oak/std /usr/include/oak/cereal
	sudo acorn -S ../std -S ../sdl -S ../extra -S ../stl -S \
		../turtle -S ../cereal -e

.PHONY:	uninstall
uninstall:
	sudo rm -rf /usr/bin/acorn /usr/bin/oak2c /usr/include/oak \
		/usr/bin/acorn-db /usr/bin/oak2c-db \
		/usr/include/oak/std_oak_header.h

.PHONY:	uninstall-db
uninstall-db:
	sudo rm -rf /usr/bin/acorn-db /usr/bin/oak2c-db

.PHONY:	reinstall
reinstall:
	$(MAKE) -C . uninstall clean install

.PHONY:	fix-format
fix-format:
	clang-format -i *.cpp *.hpp unit_tests/*.cpp \
		unit_tests/*.hpp

.PHONY:	fix-style
fix-style:	fix-format

.PHONY:	ensure-format
ensure-format:
	clang-format --Werror --dry-run *.cpp *.hpp \
		unit_tests/*.cpp unit_tests/*.hpp

################################################################

.PHONY:	dirs
dirs:
	mkdir -p bin
	mkdir -p build

README.pdf:	README.md
	pandoc README.md -o README.pdf

build/%.o:	%.cpp $(HEADS) Makefile
	$(CC) $(FLAGS) -c -o $@ $<

build/%.o.pic:	%.cpp $(HEADS)
	$(CC) $(FLAGS) -fPIC -c -o $@ $<

build/%.o.g:	%.cpp $(HEADS)
	$(CC) $(FLAGS) -g -c -o $@ $<

bin/acorn.out:	dirs build/acorn.o $(OBJS) $(HEADS)
	$(CC) $(FLAGS) -o $@ build/acorn.o $(OBJS)

bin/oak2c.out:	dirs build/oak2c.o $(OBJS) $(HEADS)
	$(CC) $(FLAGS) -o $@ build/oak2c.o $(OBJS)

################################################################

PIC_OBJS := $(OBJS:.o=.o.pic)
bin/oakc.so:	dirs $(PIC_OBJS) $(HEADS)
	$(CC) $(FLAGS) -shared -o $@ $(PIC_OBJS)

G_OBJS := $(OBJS:.o=.o.g)
bin/acorn-db.out:	dirs build/acorn.o.g $(G_OBJS) $(HEADS)
	$(CC) $(FLAGS) -g -o $@ build/acorn.o.g $(G_OBJS)

bin/oak2c-db.out:	dirs build/oak2c.o.g $(G_OBJS) $(HEADS)
	$(CC) $(FLAGS) -g -o $@ build/oak2c.o.g $(G_OBJS)

################################################################

.PHONY:	clean
clean:
	rm -rf bin/* build/* *.o *.out *.log

################################################################
