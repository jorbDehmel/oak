################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
#
# Passthrough Makefile. The real one is src/Makefile
################################################################

MAKE_SRC := $(MAKE) -C src
TEST := acorn -eTT && acorn -e
MEMCHECK := valgrind --track-origins=yes --error-exitcode=1 \
	-s --

################################################################

.PHONY: install
install:
	$(MAKE_SRC) install

.PHONY: packages
packages:
	$(MAKE_SRC) packages

.PHONY: uninstall
uninstall:
	$(MAKE_SRC) uninstall

.PHONY: reinstall
reinstall:
	$(MAKE_SRC) reinstall

.PHONY: test
test:
	cd std && $(TEST) && cd ..
	cd sdl && $(TEST) && cd ..
	cd extra && $(TEST) && cd ..
	cd stl && $(TEST) && cd ..
	cd turtle && $(TEST) && cd ..

.PHONY:	memcheck
memcheck:
	$(MEMCHECK) acorn std/tests/demo.oak

.PHONY: docs
docs: README.md docs/manual.md docs/source_code_guidelines.md
	pandoc README.md -o README.pdf
	cd docs && pandoc manual.md -o manual.pdf && cd ..
	cd docs && pandoc source_code_guidelines.md -o \
		source_code_guidelines.pdf && cd ..

.PHONY:	test_clean
test_clean:
	acorn -e
	rm */*.log ; rm */*.tlog ; rm *.log ; rm *.tlog ; rm *.out ; rm */a.out ; rm -r */.oak_build

.PHONY: clean
clean:
	$(MAKE_SRC) clean

################################################################
