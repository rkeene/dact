CC = gcc
CFLAGS = -O3 -Wall -I../../ -L../../
LDFLAGS = -lm -lncurses

all: parse-test crc-test sort

parse-test: parse-test.o ../../parse.o ../../crc.o ../../buffer.o ../../net.o ../../ui.o
crc-test: crc-test.o ../../crc.o
sort: sort.o ../../sort.o
sort.o: sort.c
crc-test.o: crc-test.c
parse-test.o: parse-test.c

clean:
	rm sort.o sort crc-test.o crc-test
