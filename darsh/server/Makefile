CC = gcc
CFLAGS = -g -O -Wall -W

all: darsh-server

darsh-server: main.o darsh-server.o darsh-peer.o
	${CC} ${CFLAGS} -o darsh-server main.o darsh-server.o darsh-peer.o

main.o: main.c darsh.h
darsh-server.o: darsh-server.c darsh.h
darsh-peer.o: darsh-peer.c darsh.h

clean:
	rm -f *.o
