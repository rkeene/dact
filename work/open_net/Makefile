CC = gcc
LDFLAGS = -shared -rdynamic -ldl -lm
CFLAGS = -Wall

net.so: net.o parse.o
	$(CC) $(LDFLAGS) -o net.so net.o parse.o
net.o: net.c net.h
parse.o: parse.c parse.h
