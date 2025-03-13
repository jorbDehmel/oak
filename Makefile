.PHONY:	check
check:
	@echo "Checking for C++ 20 or greater..."
	@g++ -std=c++20 -c -o /dev/null unit_tests/assert_version.cpp -D DESIRED_VERSION=202000ULL

	@echo "Checking for /usr/bin and /usr/include..."
	@ls /usr/bin > /dev/null
	@ls /usr/include > /dev/null

	@echo "Checking for strip..."
	@which strip > /dev/null

	@echo "Checking for valgrind..."
	@which valgrind > /dev/null

	@echo "Checking for Doxygen..."
	@which doxygen > /dev/null

	@echo "Installation (make install) should succeed. If it does not, please submit a bug report!"

.PHONY:	install
install: check
	@echo "Compiling and installing acorn..."
	$(MAKE) -C src $@
	sudo mkdir -p /usr/include/oak/std
	sudo cp std/std_oak_header.h /usr/include/oak/std/std_oak_header.h

	@echo "Installing default Oak libraries..."
	acorn -yS ./std

	@echo "Chowning /usr/include/oak..."
	sudo mkdir -p /usr/include/oak
	sudo chmod 777 /usr/include/oak

.PHONY:	install-debug
install-debug: check
	@echo "Compiling and installing acorn in debug mode..."
	$(MAKE) -C src $@

	@echo "Installing default Oak libraries..."
	acorn-debug -yS ./std

.PHONY:	uninstall
uninstall:
	acorn -A

.PHONY:	test
test:
	@echo "Ensuring proper documentation..."
	doxygen -q

	@echo "Running unit tests..."
	$(MAKE) -C unit_tests $@

	@echo "Running integration tests..."
	acorn -TE std

.PHONY:	test-debug
test-debug:
	@echo "Running integration tests..."
	acorn-debug -TE std

.PHONY:	format
format:
	find . -type f \
		\( -iname "*.cpp" -or -iname "*.hpp" \) \
		-exec clang-format -i "{}" \;

.PHONY:	docs
docs:
	doxygen -q
	$(MAKE) -C latex
	cp latex/refman.pdf refman.pdf

.PHONY:	clean
clean:
	find . -type f \
		\( -iname "*.o" -or -iname "*.out" \
		-or -iname "*.so" -or -iname "*.oak.c" \) \
		-exec rm -f "{}" \;
	rm -rf html latex *.pdf
	acorn --clean --quit

################################################################
# Container launching stuff
################################################################

# For absolute path usage later
cwd := $(shell pwd)

.PHONY: docker
docker:
	docker build --tag 'oak' .
	docker run \
		--mount type=bind,source="${cwd}",target="/host" \
		-i -t oak:latest

.PHONY: podman
podman:
	podman build --tag 'oak' .
	podman run \
		--mount type=bind,source="${cwd}",target="/host" \
		-i -t oak:latest
