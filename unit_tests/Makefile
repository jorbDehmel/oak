TESTS := test_lexer.out test_types.out test_parser.out \
	test_rule_order.out

CPP := g++ -pedantic -Wall -std=c++20 -O3 -g
OBJECT_NAMES := lexer.o macro.o oakc.o package.o parser.o \
	rule.o sapling.o type.o

VALGRIND := valgrind --leak-check=full --error-exitcode=128

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
