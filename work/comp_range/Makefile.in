CC = $CC
CFLAGS = -O3 -Wall -I../
LDFLAGS = -L../

all: rangecod.o qsmodel.o
clean:
	rm -f *.o

rangecod.o: rangecod.c rangecod.h port.h
qsmodel.o: qsmodel.c qsmodel.h
