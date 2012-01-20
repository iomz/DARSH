#
CC = gcc
CFLAGS = -g -Wall
LFLAGS = -lssl -lcrypto
SVED = sv ed rsautl

#
all: shelldriver $(SVED) 
#
shelldriver: shelldriver.o darshell.o
	$(CC) $(CFLAGS) -o shelldriver shelldriver.o darshell.o

$(SVED): rsautl.o sslcommon.o reentrant.o sv.o ed.o 
	$(CC) $(CFLAGS) -o $@ $(@).o reentrant.o sslcommon.o $(LFLAGS)

shelldriver.o: shelldriver.c darshell.h
darshell.o: darshell.c darshell.h
rsautl.o: rsautl.c
sslcommon.o: sslcommon.c sslcommon.h
reentrant.o: reentrant.c reentrant.h
sc.o: sc.c sslcommon.h
ed.o: ed.c sslcommon.h

#
clean:
	rm -f *.o shelldriver $(SVED)