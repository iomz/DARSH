
CC = gcc
CFLAGS = -Wall
LFLAGS = -lssl -lcrypto
RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

OBJS = xcertserv.o signcommon.o

XCERT = xcertserv

all: $(XCERT)

$(XCERT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o signcommon.o $(LFLAGS) 
#
$(OBJS): signcommon.h
#
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
#
clean:
	$(RM) -f $(XCERT) $(OBJS)*~