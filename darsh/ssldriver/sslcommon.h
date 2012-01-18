#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reentrant.h"

#ifndef WIN32
#include <pthread.h>
#define THREAD_CC
#define THREAD_TYPE                    pthread_t
#define THREAD_CREATE(tid, entry, arg) pthread_create(&(tid), NULL, (entry), (arg))
#else
#include <windows.h>
#define THREAD_CC                      __cdecl
#define THREAD_TYPE                    DWORD
#define THREAD_CREATE(tid, entry, arg) do { _beginthread((entry), 0, (arg));\
                                            (tid) = GetCurrentThreadId();   \
                                       } while (0)
#endif

#define PORT            "16001"
#define SERVER          "localhost"
#define CLIENT          "localhost"

#define CA_FILE		"root.pem"
#define CA_KEY		"rootkey.pem"
#define CA_DIR		NULL
#define REQ_FILE	"newreq.pem"
#define PKEY_FILE	"privkey.pem"
#define CERT_FILE	"newcert.pem"

#define DAYS_TILL_EXPIRE 30
#define EXPIRE_SECS (60*60*24*DAYS_TILL_EXPIRE)
#define EXT_COUNT 5
#define ENTRY_COUNT 4

struct entry
{
  char *key;
  char *value;
};

#define int_error(msg)  handle_error(__FILE__, __LINE__, msg)
void handle_error(const char *file, int lineno, const char *msg);

void init_OpenSSL(void);

int verify_callback(int ok, X509_STORE_CTX *store);

long post_connection_check(SSL *ssl, char *host);

void seed_prng(void);

