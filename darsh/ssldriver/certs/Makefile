
CC = gcc
CFLAGS = -g -Wall -pthread
LFLAGS = -lssl -lcrypto
RM = /bin/rm
OPENSSL = /usr/bin/openssl

#
OBJS =  certscommon.o \
	certsgen.o \
	sv.o \
	ed.o
#
BINS = certsgen sv ed
#
all: $(BINS)
#
$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o certscommon.o $(LFLAGS)
#
$(OBJS): certscommon.h
#
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
#
clean:
	$(RM) -f $(BINS) $(OBJS) *~
#
distclean: clean
