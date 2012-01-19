
CC = gcc
CFLAGS = -g -Wall -pthread
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

BINS = sslclnt sslserv sv ed 
CGEN = cgen

CA = root.pem servCA.pem

SERVCERT = serv.pem
SREQ = servkey.pem servreq.pem
SV = serv.pem

CLNTCERT = clnt.pem 
CREQ = clntkey.pem clntreq.pem
CV = clnt.pem

#For the initial definition for certifications
#all: $(CA) 
#all: $(SREQ) $(CREQ)
#all: $(SV) $(CV)
#all: $(BINS)
all: $(CGEN)
#
$(CGEN): $(OBJS)
	$(CC) $(CFLAGS) -o cgen cgen.c sslcommon.o reentrant.o $(LFLAGS)
#
$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(@).o sslcommon.o reentrant.o $(LFLAGS)
#
$(OBJS): sslcommon.h reentrant.h
#
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
#
$(CA): $(CA:.pem=.cnf)
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout rootkey.pem -out rootreq.pem -config root.cnf
	$(OPENSSL) x509 -req -in rootreq.pem -sha1 -extfile root.cnf -extensions certificate_extensions -signkey rootkey.pem -out rootcert.pem
	$(CAT) rootcert.pem rootkey.pem > root.pem
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout servCAkey.pem -out servCAreq.pem -config servCA.cnf
	$(OPENSSL) x509 -req -in servCAreq.pem -sha1 -extfile servCA.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem -CAcreateserial -out servCAcert.pem
	$(CAT) servCAcert.pem servCAkey.pem rootcert.pem > servCA.pem
#
$(SREQ):
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout servkey.pem -out servreq.pem -config serv.cnf -reqexts req_extensions
#
$(SV):
	$(OPENSSL) x509 -req -in servreq.pem -sha1 -extfile serv.cnf -extensions certificate_extensions -CA servCA.pem -CAkey servCA.pem -CAcreateserial -out servcert.pem
	$(CAT) servcert.pem servkey.pem servCAcert.pem rootcert.pem > serv.pem
#
$(CREQ):
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout clntkey.pem -out clntreq.pem -config clnt.cnf -reqexts req_extensions
#
$(CV):
	$(OPENSSL) x509 -req -in clntreq.pem -sha1 -extfile clnt.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem -CAcreateserial -out clntcert.pem
	$(CAT) clntcert.pem clntkey.pem rootcert.pem > clnt.pem
#
certclean:
	$(RM) -f rootkey.pem rootreq.pem rootcert.pem root.pem root.srl
	$(RM) -f servCAkey.pem servCAreq.pem servCAcert.pem servCA.pem servCA.srl
	$(RM) -f servkey.pem servreq.pem servcert.pem serv.pem
	$(RM) -f clntkey.pem clntreq.pem clntcert.pem clnt.pem
#
clean:
	$(RM) -f $(BINS) $(OBJS) *~
#
distclean: clean certclean