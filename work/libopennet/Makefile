# Generated automatically from Makefile.in by configure.
CC = gcc
CFLAGS = -g -O2 -DHAVE_CONFIG_H
CPPFLAGS = 

LD = gcc
LOADLIBES =
LDLIBS += -lm 
LDFLAGS += 

VER = 0.8.1

prefix = /usr/local
mandir = ${prefix}/man
sysconfdir = ${prefix}/etc
datadir = ${prefix}/share
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

LIBS = libopennet.so
OBJS = buffer.o crc.o opennet.o parse.o 

all: $(LIBS)

libopennet.so: $(OBJS)
	$(CC) -o libopennet.so -shared $(LDFLAGS) $(OBJS)

install: $(LIBS)
	cp libopennet.so $(prefix)/lib/libopennet.so.$(VER)
	ln -fs $(prefix)/lib/libopennet.so.$(VER) $(prefix)/lib/libopennet.so
	cp opennet.h $(prefix)/include/

clean:
	rm -f $(OBJS) core

distclean: clean
	rm -f $(LIBS) config.h config.cache config.log config.status Makefile


parse.o: parse.c parse.h
crc.o: crc.c crc.h
opennet.o: opennet.c opennet.h
buffer.o: buffer.c buffer.h
strsep.o: strsep.c strsep.h
