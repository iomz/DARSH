#
CC = gcc
CFLAGS = -g -O -Wall -W
#
all: shellserv shellclnt shelldriver
#
shellserv: shellserv.o darshell.o
	${CC} ${CFLAGS} -o shellserv shellserv.o darshell.o
shellclnt: shellclnt.o darshell.o
	${CC} ${CFLAGS} -o shellclnt shellclnt.o darshell.o
shelldriver: shelldriver.o darshell.o
	${CC} ${CFLAGS} -o shelldriver shelldriver.o darshell.o

shellserv.o: shellserv.c darshell.h
shellclnt.o: shellclnt.c darshell.h
shelldriver.o: shelldriver.c darshell.h
darshell.o: darshell.c darshell.h
#
clean:
	rm -f *.o shellserv shellclnt shelldriver