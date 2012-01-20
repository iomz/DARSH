
CC = gcc
CFLAGS = -Wall -pthread
LFLAGS = -lssl -lcrypto
RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

OBJS = reentrant.o	\
       sslcommon.o	\
       sslclnt.o	\
       sslserv.o	\
	   darsh-client.o
OBJS2 = xcertclnt.o signcommon.o

BIN = darsh-client
XCERT = xcertclnt

SREQ = servkey.pem servreq.pem
CREQ = clntkey.pem clntreq.pem

#For the initial definition for certifications
#all: $(SREW) $(CREQ) $(BIN)
host: $(SREW) $(CREQ) $(BIN) $(XCERT)
client: $(BIN)
#
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o sslcommon.o reentrant.o sslclnt.o sslserv.o $(LFLAGS)
#
$(XCERT): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(@).o signcommon.o $(LFLAGS) 
#
$(OBJS): sslcommon.h reentrant.h
#
$(OBJS2): signcommon.h
#
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
#
$(SREQ):
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout servkey.pem -out servreq.pem -config serv.cnf -reqexts req_extensions
#
$(CREQ):
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout clntkey.pem -out clntreq.pem -config clnt.cnf -reqexts req_extensions
#
clean:
	$(RM) -f $(BIN) $(XCERT) $(OBJS) $(OBJS2) *~