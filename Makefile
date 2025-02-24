.PHONY:	check
check:
	@echo "Checking for C++ 20 or greater..."
	@g++ -std=c++20 -c -o /dev/null unit_tests/assert_version.cpp -D DESIRED_VERSION=202000ULL

	@echo "Checking for access to /usr/bin and /usr/include..."
	@ls /usr/bin > /dev/null
	@ls /usr/include > /dev/null

	@echo "Checking for strip..."
	@strip --version > /dev/null

	@echo "Checking for valgrind..."
	@valgrind --version > /dev/null

	@echo "Installation (make install) should succeed. If it does not, please submit a bug report!"

.PHONY:	install
install: check
	@echo "Compiling and installing acorn..."
	$(MAKE) -C src $@

	@echo "Installing default Oak libraries..."
	acorn -yS ./std
	acorn -yS ./stl
	acorn -yS ./sdl
	acorn -yS ./turtle
	acorn -yS ./cereal
	acorn -yS ./extra

.PHONY:	install-debug
install-debug: check
	@echo "Compiling and installing acorn..."
	$(MAKE) -C src $@

	@echo "Installing default Oak libraries..."
	acorn -yS ./std
	acorn -yS ./stl
	acorn -yS ./sdl
	acorn -yS ./turtle
	acorn -yS ./cereal
	acorn -yS ./extra

.PHONY:	uninstall
uninstall:
	acorn -A

.PHONY:	test
test:
	@echo "Running unit tests..."
	$(MAKE) -C unit_tests $@

	@echo "Running integration tests..."
	acorn -TTE std cereal turtle extra stl sdl

.PHONY:	format
format:
	find . -type f \
		\( -iname "*.cpp" -or -iname "*.hpp" \) \
		-exec clang-format -i "{}" \;

.PHONY:	clean
clean:
	find . -type f \
		\( -iname "*.o" -or -iname "*.out" \
		-or -iname "*.so" -or -iname "*.oak.c" \) \
		-exec rm -f "{}" \;
	acorn --clean --quit
