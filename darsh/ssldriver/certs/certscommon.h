#include <stdio.h>
#include <stdlib.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>


#define int_error(msg) handle_error(__FILE__, __LINE__, msg)

#define SERVER "localhost"

#define CA_FILE		"CA.pem"
#define CA_KEY		"CAkey.pem"
#define REQ_FILE	"newreq.pem"
#define CERT_FILE	"newcert.pem"
#define PKEY_FILE	"privkey.pem"
#define REQ_FILE	"newreq.pem"

#define DAYS_TILL_EXPIRE 30
#define EXPIRE_SECS (60*60*24*DAYS_TILL_EXPIRE)
#define EXT_COUNT 5
#define ENTRY_COUNT 4

struct entry
{
  char *key;
  char *value;
};

void seed_prng(void);
