FROM ubuntu:xenial

RUN apt-get update
RUN apt-get install -y g++ clang scons cmake libgtest-dev liblz4-dev
RUN cd /usr/src/gtest && cmake . && make && mv libg* /usr/lib/

CMD cd lz4_stream && scons && ./lz4_stream_test
