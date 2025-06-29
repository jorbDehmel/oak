CWD := $(shell pwd)

CPP := g++ -pedantic -Wall -std=c++20 -O3 -g
VALGRIND := valgrind --leak-check=full --error-exitcode=128

TEST_SRC := $(shell find $(CWD) -type f \
	-iname '*.cpp' -not -iname 'assert_version.cpp' \
	-printf "%f\n")
TESTS := $(TEST_SRC:%.cpp=%.out)

CPP_FILES := $(shell find ../src/ -type f \
	-iname '*.cpp' -not -iname 'acorn.cpp' \
	-not -iname 'oak-format.cpp' -not -iname 'oak2c.cpp')
OBJECT_NAMES := $(CPP_FILES:%.cpp=%.o)

.PHONY:	test
test:	$(TESTS)
	@for exe in $(TESTS) ; do \
		echo "Running $$exe..." ; \
		./$$exe ; \
	done

%.out:	%.o $(OBJECT_NAMES:%=../src/%)
	$(CPP) -o $@ $^

%.o:	%.cpp
	$(CPP) -c -o $@ $<

../src/%.o:	../src/%.cpp ../src/*.hpp
	$(MAKE) -C ../src/ $@

.PHONY:	clean
clean:
	rm -f *.out *.o
