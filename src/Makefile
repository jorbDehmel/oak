HEADERS := lexer.hpp oakc.hpp package.hpp rule.hpp type.hpp \
	scope.hpp sapling.hpp
OBJECTS := lexer.o oakc.o package.o rule.o type.o scope.o \
	sapling.o
CPP     := g++ -std=c++20 -pedantic -Wall -O3 -g

acorn.out:	acorn-debug.out
	cp $< $@
	strip --strip-all $@

acorn-debug.out:	acorn.o $(OBJECTS)
	$(CPP) -o $@ $^

%.o:	%.cpp $(HEADERS)
	$(CPP) -c -o $@ $<

.PHONY:	install
install:	acorn.out acorn-debug.out
	sudo cp acorn.out /usr/bin/acorn
	sudo cp acorn-debug.out /usr/bin/acorn-debug

.PHONY:	clean
clean:
	rm -f *.out *.o *.so

.PHONY:	format
format:
	find . -type f \( -iname "*.cpp" -or -iname "*.hpp" \) \
		-exec clang-format -i "{}" \;
