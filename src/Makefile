HEADERS := gql.hpp lexer.hpp oakc.hpp package.hpp parser.hpp rule.hpp
OBJECTS := acorn.o lexer.o oakc.o package.o parser.o rule.o
CPP     := g++ -std=c++20 -pedantic -Wall -O3 -g

acorn.out:	acorn-debug.out
	cp $< $@
	strip --strip-all $@

acorn-debug.out:	$(OBJECTS)
	$(CPP) -o $@ $^

%.o:	%.cpp $(HEADERS)
	$(CPP) -c -o $@ $<

.PHONY:	install
install:	acorn.out acorn-debug.out
	sudo cp acorn.out /usr/bin/acorn
	sudo cp acorn-debug.out /usr/bin/acorn-debug

.PHONY:	clean
clean:
	rm -f *.out *.o

.PHONY:	format
format:
	find . -type f \( -iname "*.cpp" -or -iname "*.hpp" \) \
		-exec clang-format -i "{}" \;
