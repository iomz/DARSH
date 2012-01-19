OPENSSL = /usr/bin/openssl
CAT = /bin/cat
RM = /bin/rm
CA = root.pem servCA.pem

#For the initial definition for certifications
all: $(CA) 

#
$(CA): $(CA:.pem=.cnf)
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout rootkey.pem -out rootreq.pem -config root.cnf
	$(OPENSSL) x509 -req -in rootreq.pem -sha1 -extfile root.cnf -extensions certificate_extensions -signkey rootkey.pem -out rootcert.pem
	$(CAT) rootcert.pem rootkey.pem > root.pem
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout servCAkey.pem -out servCAreq.pem -config servCA.cnf
	$(OPENSSL) x509 -req -in servCAreq.pem -sha1 -extfile servCA.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem -CAcreateserial -out servCAcert.pem
	$(CAT) servCAcert.pem servCAkey.pem rootcert.pem > servCA.pem
#
certclean:
	$(RM) -f rootkey.pem rootreq.pem rootcert.pem root.pem root.srl
	$(RM) -f servCAkey.pem servCAreq.pem servCAcert.pem servCA.pem servCA.srl
#

distclean: clean certclean