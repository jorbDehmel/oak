CWD := $(shell pwd)

HEADERS	  := $(shell find $(CWD) -type f -iname '*.hpp')
CPP_FILES := $(shell find $(CWD) -type f -iname '*.cpp' -not \
	-iname 'acorn.cpp' -not -iname 'oak-format.cpp' \
	-not -iname 'oak2c.cpp')
OBJECTS   := $(CPP_FILES:%.cpp=%.o)

CPP    = g++ -std=c++20 -pedantic -Wall -O3
CPP-DB = g++ -std=c++20 -pedantic -Wall -g -D DEBUG
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
install:	acorn.out oak2c.out oak-format.out
	@echo "Installing regular binaries..."
	@cp acorn.out $(TARGET)/acorn
	@cp oak2c.out $(TARGET)/oak2c
	@cp oak-format.out $(TARGET)/oak-format

.PHONY:	install-debug
install-debug:	acorn-debug.out oak2c-debug.out oak-format-debug.out
	@echo "Installing debug binaries..."
	@cp acorn-debug.out $(TARGET)/acorn-debug
	@cp oak2c-debug.out $(TARGET)/oak2c-debug
	@cp oak-format-debug.out $(TARGET)/oak-format-debug

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
