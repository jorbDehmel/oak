# Note: If you have an additional rule engine to include at
# compile-time, append its object file(s) to EXTRA_OBJECTS
# and append its -D values to EXTRA_FLAGS. For headers that
# need to be included to `rule.cpp`, use
# `-D EXTRA_RULE_ENGINE_INCLUDES`. For engine entries, use
# `-D EXTRA_RULE_ENGINE_PAIRS` (see ./rule.cpp for more info).

HEADERS := lexer.hpp oakc.hpp package.hpp rule.hpp type.hpp \
	parser.hpp sapling.hpp debug.hpp macro.hpp settings.hpp
OBJECTS := lexer.o oakc.o package.o rule.o type.o parser.o \
	sapling.o macro.o
CPP     := g++ -std=c++20 -pedantic -Wall -O3
CPP-DB  := g++ -std=c++20 -pedantic -Wall -g -D DEBUG

TARGET = /usr/bin

%.out:	%.o $(OBJECTS) $(EXTRA_OBJECTS)
	$(CPP) -o $@ $^

%-debug.out:	%-debug.o $(OBJECTS:%.o=%-debug.o) $(EXTRA_OBJECTS)
	$(CPP-DB) -g -o $@ $^

%.o:	%.cpp $(HEADERS)
	$(CPP) -c -o $@ $< $(EXTRA_FLAGS)

%-debug.o:	%.cpp $(HEADERS)
	$(CPP-DB) -c -o $@ $< $(EXTRA_FLAGS)

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
