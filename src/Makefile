HEADERS := lexer.hpp oakc.hpp package.hpp rule.hpp type.hpp \
	parser.hpp sapling.hpp debug.hpp macro.hpp settings.hpp
OBJECTS := lexer.o oakc.o package.o rule.o type.o parser.o \
	sapling.o macro.o
CPP     := g++ -std=c++20 -pedantic -Wall -O3 -g

TARGET = /usr/bin

%.out:	%.o $(OBJECTS)
	$(CPP) -o $@ $^

%-debug.out:	%-debug.o $(OBJECTS:%.o=%-debug.o)
	$(CPP) -g -o $@ $^

%.o:	%.cpp $(HEADERS)
	$(CPP) -c -o $@ $<

%-debug.o:	%.cpp $(HEADERS)
	$(CPP) -D DEBUG -g -c -o $@ $<

.PHONY:	install
install:	acorn.out oak2c.out
	@echo "Installing regular binaries..."
	@cp acorn.out $(TARGET)/acorn
	@cp oak2c.out $(TARGET)/oak2c

.PHONY:	install-debug
install-debug:	acorn-debug.out oak2c-debug.out
	@echo "Installing debug binaries..."
	@cp acorn-debug.out $(TARGET)/acorn-debug
	@cp oak2c-debug.out $(TARGET)/oak2c-debug

.PHONY:	uninstall
uninstall:
	@echo "Uninstalling acorn..."
	acorn -A

.PHONY:	clean
clean:
	rm -f *.out *.o *.so

.PHONY:	format
format:
	find . -type f \( -iname "*.cpp" -or -iname "*.hpp" \) \
		-exec clang-format -i "{}" \;
