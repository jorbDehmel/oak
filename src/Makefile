HEADERS := lexer.hpp oakc.hpp package.hpp rule.hpp type.hpp \
	scope.hpp sapling.hpp
OBJECTS := lexer.o oakc.o package.o rule.o type.o scope.o \
	sapling.o
CPP     := g++ -std=c++20 -pedantic -Wall -O3 -g

TARGET = /usr/bin

%.out:	%-debug.out
	cp $< $@
	strip --strip-all $@

%-debug.out:	%.o $(OBJECTS)
	$(CPP) -o $@ $^

%.o:	%.cpp $(HEADERS)
	$(CPP) -c -o $@ $<

.PHONY:	install
install:	acorn.out acorn-debug.out oak2c.out oak2c-debug.out
	@echo "Installing debug binaries..."
	@cp acorn-debug.out $(TARGET)/acorn-debug
	@cp oak2c-debug.out $(TARGET)/oak2c-debug
	@echo "Installing regular binaries..."
	@cp acorn.out $(TARGET)/acorn
	@cp oak2c.out $(TARGET)/oak2c

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
