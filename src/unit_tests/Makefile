# Unit testing Makefile for Oak
# Jordan Dehmel, 2024
# jdehmel@outlook.com
# github.com/jorbDehmel/oak

TARGETS := test_acorn_resources.out test_fn_resources.out \
	test_generics.out test_lexer.out test_op_sub.out \
	test_packages.out test_reconstruct.out test_rules.out \
	test_sequence_resources.out test_sequence.out \
	test_type_builder.out
CC := clang++ -std=c++17 -g
LIB_HEAD := ../oakc_fns.hpp
LIB_OBJ := ../bin/oakc.so
FLAGS := -O3 -g -Wall -pedantic

.PHONY:	test
all:	$(TARGETS)

%.out:	%.cpp $(LIB_OBJ)
	$(CC) $(FLAGS) -o $@ $^ && ./$@ && rm $@

$(LIB_OBJ):
	$(MAKE) -C .. so

.PHONY:	clean
clean:
	rm *.out
