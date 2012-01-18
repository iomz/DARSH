
CC = gcc
CFLAGS = -g -Wall -pthread
LFLAGS = -lssl -lcrypto
RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

#
OBJS =  certscommon.o \
	certsgen.o \
	certsreq.o \
	smime.o
#
BINS = certsgen certsreq smime
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
$(KEY): 
	

#
clean:
	$(RM) -f $(BINS) $(OBJS) *~
#
distclean: clean certclean