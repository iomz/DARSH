CC	= gcc
CFLAGS  = -g -O -Wall -W
PROJECT	= darsh
PACKAGE = darsh-client
SRCS	= $(PACKAGE).c 
HEADS	= $(PROJECT).h
OBJS	= $(PACKAGE).o
PROGRAM	= $(PACKAGE)
#
all: $(PROGRAM)
#
$(PROGRAM):
	$(CC) ${CFLAGS} $(SRCS) darshell.c -o $(PROGRAM)

$(OBJS): $(HEADS) Makefile
darsh-client.o: darsh-client.c darsh-client.h darsh-common.h
darshell.o: darshell.c darshell.h
#
clean:
	rm -f *.o *~ $(PROGRAM)
