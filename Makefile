################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
#
# Passthrough Makefile. The real one is src/Makefile
################################################################

MAKE_SRC := $(MAKE) -C src

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
	$(MAKE_SRC) test
	cd extra && acorn -TT && acorn -e && cd ..
	cd stl && acorn -TT && acorn -e && cd ..
	cd turtle && acorn -TT && acorn -e && cd ..

.PHONY: docs
docs: README.md docs/manual.md docs/source_code_guidelines.md
	pandoc README.md -o README.pdf
	cd docs && pandoc manual.md -o manual.pdf && cd ..
	cd docs && pandoc source_code_guidelines.md -o \
		source_code_guidelines.pdf && cd ..

.PHONY:	test_clean
test_clean:
	acorn -e
	rm */*.log ; rm */*.tlog ; rm *.log ; rm *.tlog ; rm *.out ; rm */a.out

.PHONY: clean
clean:
	$(MAKE_SRC) clean
	$(MAKE) test_clean

################################################################
