
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

BINS = sslclnt sslserv

SERVCERT = serv.pem
SREQ = servkey.pem servreq.pem
SV = serv.pem

CLNTCERT = clnt.pem 
CREQ = clntkey.pem clntreq.pem
CV = clnt.pem

DHPARAMS = dh512.pem dh1024.pem

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
dhparam: $(DHPARAMS)
#
certclean:
	$(RM) -f servkey.pem servreq.pem servcert.pem serv.pem
	$(RM) -f clntkey.pem clntreq.pem clntcert.pem clnt.pem
#
dh512.pem:
	$(OPENSSL) dhparam -check -text -5 512 -out dh512.pem

dh1024.pem:
	$(OPENSSL) dhparam -check -text -5 1024 -out dh1024.pem

dhparamclean:
	$(RM) -f $(DHPARAMS)
#
clean:
	$(RM) -f $(BINS) $(OBJS) *~
#
distclean: clean certclean dhparamclean