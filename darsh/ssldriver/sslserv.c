#include "sslcommon.h"
 
/* Set up ephemeral RSA stuff */
RSA *rsa_512 = NULL;
RSA *rsa_1024 = NULL;

void init_rsakey(void)
{
  rsa_512 = RSA_generate_key(512,RSA_F4,NULL,NULL);
  if (rsa_512 == NULL)
    int_error("Error opening file rsa512.pem");
  rsa_1024 = RSA_generate_key(1024,RSA_F4,NULL,NULL);
  if (rsa_1024 == NULL)
    int_error("Error opening file rsa1024.pem");
}


RSA *tmp_rsa_callback(SSL *s, int is_export, int keylength)
{
  RSA *rsa_tmp=NULL;

  init_rsakey();

  switch (keylength) {
  case 512:
    if (rsa_512)
      rsa_tmp = rsa_512;
    else { /* generate on the fly, should not happen in this example */
      rsa_tmp = RSA_generate_key(keylength,RSA_F4,NULL,NULL);
      rsa_512 = rsa_tmp; /* Remember for later reuse */
    }
    break;
  case 1024:
    if (rsa_1024)
      rsa_tmp=rsa_1024;
    else
      ;
    break;
  default:
    /* Generating a key on the fly is very costly, so use what is there */
    if (rsa_1024)
      rsa_tmp=rsa_1024;
    else
      rsa_tmp=rsa_512; /* Use at least a shorter key */
  }
  return rsa_tmp;
}


#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
#define CAFILE "rootcert.pem"
#define CADIR NULL
#define CERTFILE "serv.pem"
SSL_CTX *setup_server_ctx(void)
{
    SSL_CTX *ctx;
 
    ctx = SSL_CTX_new(SSLv23_method());
    if (SSL_CTX_load_verify_locations(ctx, CAFILE, CADIR) != 1)
        int_error("Error loading CA file and/or directory");
    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        int_error("Error loading default CA file and/or directory");
    if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
        int_error("Error loading certificate from file");
    if (SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
        int_error("Error loading private key from file");
    SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |
			SSL_OP_EPHEMERAL_RSA); // Use ephemeral RSA key exchange 
    SSL_CTX_set_tmp_rsa_callback(ctx, tmp_rsa_callback);
    if (SSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != 1)
        int_error("Error setting cipher list (no valid ciphers)");
    return ctx;
}

int do_server_loop(SSL *ssl)
{
    int  err, nread;
    char buf[80];
 
    do
    {
        for (nread = 0;  nread < sizeof(buf);  nread += err)
        {
            err = SSL_read(ssl, buf + nread, sizeof(buf) - nread);
            if (err <= 0)
                break;
        }
        fprintf(stdout, "%s", buf);
    }
    while (err > 0);
    return (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN) ? 1 : 0;
}
  
void THREAD_CC server_thread(void *arg)
{
    SSL *ssl = (SSL *)arg;
    long err;
 
#ifndef WIN32
    pthread_detach(pthread_self());
#endif
    if (SSL_accept(ssl) <= 0)
        int_error("Error accepting SSL connection");

    fprintf(stderr, "SSL Connection opened\n");
    if (do_server_loop(ssl))
        SSL_shutdown(ssl);
    else
        SSL_clear(ssl);
    fprintf(stderr, "SSL Connection closed\n");
    SSL_free(ssl);
    ERR_remove_state(0);
#ifdef WIN32
    _endthread();
#endif
}
 
int main(int argc, char *argv[])
{
    BIO     *acc, *client;
    SSL     *ssl;
    SSL_CTX *ctx;
    THREAD_TYPE tid;

    init_OpenSSL();
    OpenSSL_add_all_algorithms();	// Avoid PBE algoithm not found
    seed_prng();
 
    ctx = setup_server_ctx();
 
    acc = BIO_new_accept(PORT);
    if (!acc)
        int_error("Error creating server socket");
 
    if (BIO_do_accept(acc) <= 0)
        int_error("Error binding server socket");
 
    for (;;)
    {
        if (BIO_do_accept(acc) <= 0)
            int_error("Error accepting connection");
 
        client = BIO_pop(acc);
        if (!(ssl = SSL_new(ctx)))
        int_error("Error creating SSL context");
        SSL_set_accept_state(ssl);
        SSL_set_bio(ssl, client, client);
        THREAD_CREATE(tid, (void *)server_thread, ssl);
    }
 
    SSL_CTX_free(ctx);
    BIO_free(acc);
    return 0;
}
