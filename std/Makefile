CC = gcc -O3
CXX = g++ -O3
OBJECTS = io.o system.o random.o panic.o math.o operators.o \
	fs.o c_str.o

.PHONY:	all
all:	$(OBJECTS)

%.o:	%.c
	$(CC) -c -o $@ $<

%.o:	%.cpp
	$(CXX) -c -o $@ $<
