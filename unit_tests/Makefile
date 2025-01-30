TESTS := test_lexer.out test_oakc.out test_packages.out \
	test_rules.out  test_types.out test_scope.out

CPP := g++ -pedantic -Wall -std=c++20 -O3 -g
OBJECT_NAMES := lexer.o oakc.o package.o rule.o type.o \
	scope.o sapling.o

VALGRIND := valgrind --leak-check=full --error-exitcode=128

.PHONY:	test
test:	$(TESTS)

%.out:	%.o $(OBJECT_NAMES:%=../src/%)
	$(CPP) -o $@ $^

%.o:	%.cpp
	$(CPP) -c -o $@ $<

../src/%.o:
	$(MAKE) -C ../src/ $@

.PHONY:	clean
clean:
	rm -f *.out *.o
