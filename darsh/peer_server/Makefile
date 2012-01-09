CC	= gcc
PROJECT	= darsh
PACKAGE = darsh-peer
SRCS	= $(PACKAGE).c
HEADS	= $(PROJECT).h
OBJS	= $(PACKAGE).o
PROGRAM	= $(PACKAGE)

all: $(PROGRAM)

$(PROGRAM):
	$(CC) $(SRCS) -o $(PROGRAM)

$(OBJS): $(HEADS) Makefile

clean:
	rm -f *.o *~ $(PROGRAM)
