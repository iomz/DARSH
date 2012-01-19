#include "certscommon.h"

struct entry entries[ENTRY_COUNT] = {
  {"countryName", "JP"},
  {"stateOrProvinceName", "Kanagawa"},
  {"localityName", "Fujisawa"},
  {"organizationName", "darsh"},
};

int
main(int argc, char *argv[])
{
  int i;
  X509_REQ *req;
  X509_NAME *subj;
  EVP_PKEY *pkey;
  EVP_MD *digest;
  FILE *fp;

  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
  seed_prng();

/* first read in the private key */
  if(!(fp = fopen(PKEY_FILE, "r")))
    int_error("Error reading private key file");
  if(!(pkey = PEM_read_PrivateKey(fp, NULL, NULL, "secret")))
    int_error("Error reading private key in file");
  fclose(fp);

/* create a new request and add the key to it */
  if(!(req = X509_REQ_new()))
    int_error("Failed to create X509_REQ object");
  X509_REQ_set_pubkey(req, pkey);

/* assign the subject name */
  if(!(subj = X509_NAME_new()))
    int_error("Failed to create X509_NAME object");

  for(i = 0; i < ENTRY_COUNT; i++)
    {
      int nid;
      X509_NAME_ENTRY *ent;

      if((nid = OBJ_txt2nid(entries[i].key)) == NID_undef)
	{
	  fprintf(stderr, "Error finding NID for %s\n", entries[i].key);
	  int_error("Error on lookup");
	}
      if(!(ent = X509_NAME_ENTRY_create_by_NID(NULL, nid, MBSTRING_ASC,
						 entries[i].value, -1)))
	int_error("Error creating Name entry from NID");
      if(X509_NAME_add_entry(subj, ent, -1, 0) != 1)
	int_error("Error adding entry to Name");
    }
  if(X509_REQ_set_subject_name(req, subj) != 1)
    int_error("Error padding subject to request");

  {
    X509_EXTENSION *ext;
    STACK_OF(X509_EXTENSION) * extlist;
    char *name = "subjectAltName";
    char *value = SERVER;

    extlist = sk_X509_EXTENSION_new_null();

    if(!(ext = X509V3_EXT_conf(NULL, NULL, name, value)))
      int_error("Error creating subjectAltName extension");

    sk_X509_EXTENSION_push(extlist, ext);

    if(!X509_REQ_add_extensions(req, extlist))
      int_error("Error adding subjectAltName to the request");
    sk_X509_EXTENSION_pop_free(extlist, X509_EXTENSION_free);
  }

/* pick the correct digest and sign the request */
  if(EVP_PKEY_type(pkey->type) == EVP_PKEY_DSA)
    digest = EVP_dss1();
  else if(EVP_PKEY_type(pkey->type) == EVP_PKEY_RSA)
    digest = EVP_sha1();
  else
    int_error("Error checking public key for a valid digest");
  if(!(X509_REQ_sign(req, pkey, digest)))
    int_error("Error signing request");

/* write the completed request */
  if(!(fp = fopen(REQ_FILE, "w")))
    int_error("Error writing to request file");
  if(PEM_write_X509_REQ(fp, req) != 1)
    int_error("Error while writing request");
  fclose(fp);

  EVP_PKEY_free(pkey);
  X509_REQ_free(req);
  return 0;
}
