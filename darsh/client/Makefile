CC	= gcc
PROJECT	= darsh
PACKAGE = darsh-client
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
