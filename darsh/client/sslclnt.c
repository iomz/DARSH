#include "sslcommon.h"
#include "darsh-common.h"
#include <stdio.h>
#include <stdlib.h>

#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
#define CAFILE "rootcert.pem"
#define CADIR NULL
#define CERTFILE "clnt.pem"
SSL_CTX *setup_client_ctx(void)
{
    SSL_CTX *ctx;
 
    ctx = SSL_CTX_new(SSLv23_method(  ));
    if (SSL_CTX_load_verify_locations(ctx, CAFILE, CADIR) != 1)
        int_error("Error loading CA file and/or directory");
    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        int_error("Error loading default CA file and/or directory");
    if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
        int_error("Error loading certificate from file");
    if (SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
        int_error("Error loading private key from file");
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
    SSL_CTX_set_verify_depth(ctx, 4);
    SSL_CTX_set_options(ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);
    if (SSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != 1)
        int_error("Error setting cipher list (no valid ciphers)");
    return ctx;
}

int do_client_loop(SSL *ssl)
{
    int  err, nwritten;
    char buf[80];
 
    for (;;)
    {
        if (!fgets(buf, sizeof(buf), stdin))
            break;
        for (nwritten = 0;  nwritten < sizeof(buf);  nwritten += err)
        {
            err = SSL_write(ssl, buf + nwritten, sizeof(buf) - nwritten);
            if (err <= 0)
                return 0;
        }
    }
    return 1;
}
 
//int main(int argc, char *argv[])
int sslclnt(char *server)
{
    BIO     *conn;
    SSL     *ssl;
    SSL_CTX *ctx;
    long    err;
    char remote_server[BUF_LEN];
    strcpy(remote_server, server);

    init_OpenSSL(  );
    OpenSSL_add_all_algorithms();	// Avoid PBE algoithm not found
    seed_prng(  );			// Pseudorandom Number generation for OpenSSL
 
    ctx = setup_client_ctx(  );		// Offer the correct certification
 
    strcat(remote_server, ":");
    strcat(remote_server, PORT);
    printf("remote_server: '%s'\n", remote_server);
    //conn = BIO_new_connect(SERVER ":" PORT);
    conn = BIO_new_connect(remote_server);
    /*
    char r_serv[BUF_LEN] = "\0";
    strcat(r_serv, "dali.ht.sfc.keio.ac.jp");
    strcat(r_serv, ":");
    strcat(r_serv, PORT);
    printf("r_serv: %s\n", r_serv);
    conn = BIO_new_connect(r_serv);
    */
    if (!conn)
        int_error("Error creating connection BIO");
 
    if (BIO_do_connect(conn) <= 0)
        int_error("Error connecting to remote machine");
 
    ssl = SSL_new(ctx);			// Create new SSL Object
    SSL_set_bio(ssl, conn, conn);
    if (SSL_connect(ssl) <= 0)
        int_error("Error connecting SSL object");

    fprintf(stderr, "SSL Connection opened\n");
    if (do_client_loop(ssl))
        SSL_shutdown(ssl);
    else
        SSL_clear(ssl);
    fprintf(stderr, "SSL Connection closed\n");
 
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
}
