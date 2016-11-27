FROM ubuntu:xenial

RUN apt-get update
RUN apt-get install -y g++ scons cmake libgtest-dev liblz4-dev
RUN cd /usr/src/gtest && cmake . && make && mv libg* /usr/lib/
