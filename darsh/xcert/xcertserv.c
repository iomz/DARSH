#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include "signcommon.h"

#define     CREQ_FILE "clntreq.pem"
#define     CCA_FILE "root.pem"
#define     CCA_KEY "rootkey.pem"
#define     CCERT_FILE "clntcert.pem"

#define     SREQ_FILE  "servreq.pem"
#define     SCA_FILE  "servCA.pem"
#define     SCA_KEY  "servCAkey.pem"
#define     SCERT_FILE "servcert.pem"

#define MAXPENDING 5

struct
entry ext_ent[EXT_COUNT] = {
  {"basicConstraints", "CA:FALSE"},
  {"nsComment", "\"OpenSSL Generated Certificate\""},
  {"subjectKeyIdentifier", "hash"},
  {"authorityKeyIdentifier", "keyid,issuer:always"},
  {"keyUsage", "nonRepudiation,digitalSignature,keyEncipherment"}
};

void ClntSign(){
  long serial = 1;
  EVP_PKEY *pkey, *CApkey;
  const EVP_MD *digest;
  X509 *cert, *CAcert;
  X509_REQ *req;
  X509_NAME *name;
  FILE *fp;
  BIO *out;

  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
  seed_prng();

/* open stdout */
  if(!(out = BIO_new_fp(stdout, BIO_NOCLOSE)))
    int_error("Error creating stdout BIO");

/* read in the request */
  if(!(fp = fopen(CREQ_FILE, "r")))
    int_error("Error reading request file");
  if(!(req = PEM_read_X509_REQ(fp, NULL, NULL, NULL)))
    int_error("Error reading request in file");
  fclose(fp);

/* verify signature on the request */
  if(!(pkey = X509_REQ_get_pubkey(req)))
    int_error("Error getting public key from request");
  if(X509_REQ_verify(req, pkey) != 1)
    int_error("Error verifying signature on certificate");

/* read in the CA certificate */
  if(!(fp = fopen(CCA_FILE, "r")))
    int_error("Error reading CA certificate file");
  if(!(CAcert = PEM_read_X509(fp, NULL, NULL, NULL)))
    int_error("Error reading CA certificate in file");
  fclose(fp);

/* read in the CA private key */
  if(!(fp = fopen(CCA_KEY, "r")))
    int_error("Error reading CA private key file");
  if(!(CApkey = PEM_read_PrivateKey(fp, NULL, NULL, "root")))
    int_error("Error reading CA private key in file");
  fclose(fp);

/* print out the subject name and subject alt name extension */
  if(!(name = X509_REQ_get_subject_name(req)))
    int_error("Error getting subject name from request");
  X509_NAME_print(out, name, 0);
  fputc('\n', stdout);

/*   fprintf(stderr, "\nContinue? [y/n]: "); */
/*   char yn = getchar(); */
/*   if( yn  == 'n' || yn == 'N') */
/*     return 0; */

/* create new certificate */
  if(!(cert = X509_new()))
    int_error("Error creating X509 object");

/* set version number for the certificate(X509v3) and the serial number */
  if(X509_set_version(cert, 2L) != 1)
    int_error("Error settin certificate version");
  ASN1_INTEGER_set(X509_get_serialNumber(cert), serial++);

/* set issuer and subject name of the cert from the req and the CA */
  if(!(name = X509_REQ_get_subject_name(req)))
    int_error("Error getting subject name from request");
  if(X509_set_subject_name(cert, name) != 1)
    int_error("Error setting subject name of certificate");
  if(!(name = X509_get_subject_name(CAcert)))
    int_error("Error getting subject name from CA certificate");
  if(X509_set_issuer_name(cert, name) != 1)
    int_error("Error setting issuer name of certificate");

/* set public key in the certificate */
  if(X509_set_pubkey(cert, pkey) != 1)
    int_error("Error setting public key of the certificate");

/* set duration for the certificate */
  if(!(X509_gmtime_adj(X509_get_notBefore(cert), 0)))
    int_error("Error setting beginning time of the certificate");
  if(!(X509_gmtime_adj(X509_get_notAfter(cert), EXPIRE_SECS)))
    int_error("Error setting ending time of the certificate");

/* sign the certificate with the CA private key */
  if(EVP_PKEY_type(CApkey->type) == EVP_PKEY_DSA)
    digest = EVP_dss1();
  else if(EVP_PKEY_type(CApkey->type) == EVP_PKEY_RSA)
    digest = EVP_sha1();
  else
    int_error("Error checking CA private key for a valid digest");
  if(!(X509_sign(cert, CApkey, digest)))
    int_error("Error signing certificate");

/* write the completed certificate */
  if(!(fp = fopen(CCERT_FILE, "w")))
    int_error("Error writing to certificate file");
  if(PEM_write_X509(fp, cert) != 1)
    int_error("Error while writing certificate");
  fclose(fp);
}

void ServSign(){
  long serial = 1;
  EVP_PKEY *pkey, *CApkey;
  const EVP_MD *digest;
  X509 *cert, *CAcert;
  X509_REQ *req;
  X509_NAME *name;
  FILE *fp;
  BIO *out;

  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
  seed_prng();

/* open stdout */
  if(!(out = BIO_new_fp(stdout, BIO_NOCLOSE)))
    int_error("Error creating stdout BIO");

/* read in the request */
  if(!(fp = fopen(SREQ_FILE, "r")))
    int_error("Error reading request file");
  if(!(req = PEM_read_X509_REQ(fp, NULL, NULL, NULL)))
    int_error("Error reading request in file");
  fclose(fp);

/* verify signature on the request */
  if(!(pkey = X509_REQ_get_pubkey(req)))
    int_error("Error getting public key from request");
  if(X509_REQ_verify(req, pkey) != 1)
    int_error("Error verifying signature on certificate");

/* read in the CA certificate */
  if(!(fp = fopen(SCA_FILE, "r")))
    int_error("Error reading CA certificate file");
  if(!(CAcert = PEM_read_X509(fp, NULL, NULL, NULL)))
    int_error("Error reading CA certificate in file");
  fclose(fp);

/* read in the CA private key */
  if(!(fp = fopen(SCA_KEY, "r")))
    int_error("Error reading CA private key file");
  if(!(CApkey = PEM_read_PrivateKey(fp, NULL, NULL, "serv")))
    int_error("Error reading CA private key in file");
  fclose(fp);

/* print out the subject name and subject alt name extension */
  if(!(name = X509_REQ_get_subject_name(req)))
    int_error("Error getting subject name from request");
  X509_NAME_print(out, name, 0);
  fputc('\n', stdout);

/*   fprintf(stderr, "\nContinue? [y/n]: "); */
/*   char yn = getchar(); */
/*   if( yn  == 'n' || yn == 'N') */
/*     return 0; */

/* create new certificate */
  if(!(cert = X509_new()))
    int_error("Error creating X509 object");

/* set version number for the certificate(X509v3) and the serial number */
  if(X509_set_version(cert, 2L) != 1)
    int_error("Error settin certificate version");
  ASN1_INTEGER_set(X509_get_serialNumber(cert), serial++);

/* set issuer and subject name of the cert from the req and the CA */
  if(!(name = X509_REQ_get_subject_name(req)))
    int_error("Error getting subject name from request");
  if(X509_set_subject_name(cert, name) != 1)
    int_error("Error setting subject name of certificate");
  if(!(name = X509_get_subject_name(CAcert)))
    int_error("Error getting subject name from CA certificate");
  if(X509_set_issuer_name(cert, name) != 1)
    int_error("Error setting issuer name of certificate");

/* set public key in the certificate */
  if(X509_set_pubkey(cert, pkey) != 1)
    int_error("Error setting public key of the certificate");

/* set duration for the certificate */
  if(!(X509_gmtime_adj(X509_get_notBefore(cert), 0)))
    int_error("Error setting beginning time of the certificate");
  if(!(X509_gmtime_adj(X509_get_notAfter(cert), EXPIRE_SECS)))
    int_error("Error setting ending time of the certificate");


/* sign the certificate with the CA private key */
  if(EVP_PKEY_type(CApkey->type) == EVP_PKEY_DSA)
    digest = EVP_dss1();
  else if(EVP_PKEY_type(CApkey->type) == EVP_PKEY_RSA)
    digest = EVP_sha1();
  else
    int_error("Error checking CA private key for a valid digest");
  if(!(X509_sign(cert, CApkey, digest)))
    int_error("Error signing certificate");

/* write the completed certificate */
  if(!(fp = fopen(SCERT_FILE, "w")))
    int_error("Error writing to certificate file");
  if(PEM_write_X509(fp, cert) != 1)
    int_error("Error while writing certificate");
  fclose(fp);
}

void HandleError(const char *err){
  perror(err);
  exit(1);
}

void ReceiveReq(int clntSock, char *reqfile){
  char buf[BUFSIZ];
  int recved;

  int fdout = open(reqfile, O_WRONLY|O_CREAT, 0644 );
  if(fdout == -1)
    HandleError("open");

  if( (recved = recv(clntSock, buf, BUFSIZ, 0)) < 0)
    HandleError("recv() failed");
  
  dprintf(fdout, "%s", buf);
  close(fdout);
  printf("%s received!\n", reqfile);
}    

void SendCert(int clntSock, char *certfile){
  FILE *fp = fopen(certfile, "r");
  char tmp[256],tmp2[BUFSIZ];
  while(fgets(tmp, 256, fp) != NULL){
    strcat(tmp2, tmp);
  }
  fclose(fp);
  if( (send(clntSock, tmp2, strlen(tmp2), 0)) < 0 )
    HandleError("send() failed");
  printf("%s sent!\n", certfile);
}

int main(void){

  int servSock;
  int clntSock;
  struct sockaddr_in xcertServ;
  struct sockaddr_in xcertClnt;
  unsigned short xcertPort = 22222;
  unsigned int clntLen;

  /* Create socket for incoming connections */
  if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    HandleError("socket() failed");
      
  /* Construct local address structure */
  memset(&xcertServ, 0, sizeof(xcertServ));
  xcertServ.sin_family = AF_INET;
  xcertServ.sin_addr.s_addr = htonl(INADDR_ANY);
  xcertServ.sin_port = htons(xcertPort);

  if(bind(servSock, (struct sockaddr *) &xcertServ, sizeof(xcertServ)) < 0)
    HandleError("bind() failed");

  if(listen(servSock, MAXPENDING) < 0)
    HandleError("listen() failed");

  for(;;){
      clntLen = sizeof(xcertClnt);

      if((clntSock = accept(servSock, (struct sockaddr *) &xcertClnt, &clntLen)) < 0)
	HandleError("accept() failed");

      printf("Receiving request from %s\n", inet_ntoa(xcertClnt.sin_addr));

      ReceiveReq(clntSock, "clntreq.pem");
      ClntSign();
      SendCert(clntSock, "clntcert.pem");
      ReceiveReq(clntSock, "servreq.pem");
      ServSign();
      SendCert(clntSock, "servcert.pem");
    }

  return 0;
}
