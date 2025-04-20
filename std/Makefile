CWD := $(shell pwd)
CC = gcc -O3 -I $(CWD)
CXX = g++ -O3 -I $(CWD)
OBJECTS = io.o system.o random.o panic.o math.o operators.o \
	fs.o c_str.o casts.o ofile.o ifile.o thread.o

.PHONY:	all
all:	$(OBJECTS)

%.o:	%.c
	$(CC) -c -o $@ $<

%.o:	%.cpp
	$(CXX) -c -o $@ $<
