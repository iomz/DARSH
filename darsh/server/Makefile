CC = gcc
CFLAGS = -g -O -Wall -W -Wno-pointer-sign
PACKAGE = darsh-server
PROGRAM = $(PACKAGE)

all: $(PROGRAM)

darsh-server: main.o darsh-server.o darsh-peer.o
	${CC} ${CFLAGS} -o darsh-server main.o darsh-server.o darsh-peer.o

main.o: main.c darsh.h darsh-common.h
darsh-server.o: darsh-server.c darsh.h darsh-common.h
darsh-peer.o: darsh-peer.c darsh.h darsh-common.h

clean:
	rm -f *.o *~ $(PROGRAM)
