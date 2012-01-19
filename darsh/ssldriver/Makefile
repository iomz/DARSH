
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
	cgen.o		\
	sv.o 		\
	ed.o

BINS = sslclnt sslserv cgen

SERVCERT = serv.pem
SREQ = servkey.pem servreq.pem
SV = serv.pem

CLNTCERT = clnt.pem 
CREQ = clntkey.pem clntreq.pem
CV = clnt.pem

#For the initial definition for certifications
all: $(SREQ) $(CREQ) $(BINS)
#
$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o sslcommon.o reentrant.o $(LFLAGS)
#
$(OBJS): sslcommon.h reentrant.h
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
certclean:
	$(RM) -f servkey.pem servreq.pem servcert.pem serv.pem
	$(RM) -f clntkey.pem clntreq.pem clntcert.pem clnt.pem
#
clean:
	$(RM) -f $(BINS) $(OBJS) *~ cgen
#
distclean: clean certclean