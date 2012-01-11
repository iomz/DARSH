/* rsautil.c */
#include <stdio.h>
#include <stdlib.h>
#include <openssl/pem.h>

static void callback(int p, int n, void *arg)
{
  char c='B';
  if (p == 0) c='.';
  if (p == 1) c='+';
  if (p == 2) c='*';
  if (p == 3) c='\n';
  fputc(c,stderr);
}

int main (int argc, char *argv[])
{
  char *file_pem = "priv.pem";
  char *file_pem_pub = "pub.pem";
  FILE *fp;
  
  int bits = 512;//512, 1024, 2048, 4096
  unsigned long exp = RSA_F4;//RSA_3
  
  RSA *rsa;
  EVP_PKEY *pkey;
  
  //GENERATE KEY
  rsa=RSA_generate_key(bits,exp,callback,NULL);
  if(RSA_check_key(rsa)!=1){
    fprintf(stderr, "Error whilst checking key.\n");
    exit(1);
  }
  
  //ADD KEY TO EVP
  pkey = EVP_PKEY_new();
  EVP_PKEY_assign_RSA(pkey, rsa);
  
  //WRITE PRIVATE KEY
  if(!(fp = fopen(file_pem, "w"))) {
    fprintf(stderr, "Error opening PEM file %s\n", file_pem);
    exit(1);
  }
  if(!PEM_write_PrivateKey(fp, pkey, NULL,NULL,0,NULL,NULL)){
    fprintf(stderr, "Error writing PEM file %s\n", file_pem);
    exit(1);
  }
  close(fp);
  
  //WRITE PUBLIC KEY
  if(!(fp = fopen(file_pem_pub, "w"))) {
    fprintf(stderr, "Error opening PEM file %s\n", file_pem_pub);
    exit(1);
  }
  if(!PEM_write_PUBKEY(fp, pkey)){
    fprintf(stderr, "Error writing PEM file %s\n", file_pem_pub);
    exit(1);
  }
  close(fp);
  
  //FREE
  RSA_free(rsa);
  
  return 0;
}
