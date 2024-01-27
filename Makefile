################################################################
# Jordan Dehmel, 2023
# jdehmel@outlook.com
# github.com/jorbDehmel/oak
#
# Passthrough Makefile. The real one is src/Makefile
################################################################

MAKE_SRC := $(MAKE) -C src

################################################################

install:
	$(MAKE_SRC) install

packages:
	$(MAKE_SRC) packages

uninstall:
	$(MAKE_SRC) uninstall

reinstall:
	$(MAKE_SRC) reinstall

test:
	$(MAKE_SRC) test
	cd extra && acorn -TT ; cd ..
	cd stl && acorn -TT ; cd ..
	cd turtle && acorn -TT ; cd ..

docs: README.md docs/manual.md docs/source_code_guidelines.md
	pandoc README.md -o README.pdf
	cd docs && pandoc manual.md -o manual.pdf && cd ..
	cd docs && pandoc source_code_guidelines.md -o \
		source_code_guidelines.pdf && cd ..

################################################################
