FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y build-essential g++ doxygen valgrind \
    sudo libsdl2-dev
WORKDIR /host
