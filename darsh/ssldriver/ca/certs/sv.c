#include "sslcommon.h"

X509_STORE *
create_store (void)
{
  X509_STORE *store;
  X509_LOOKUP *lookup;

/* create the cert store and set the verify callback */
  if (!(store = X509_STORE_new ()))
    {
      fprintf (stderr, "Error creating X509_STORE_CTX object\n");
      goto err;
    }
  X509_STORE_set_verify_cb_func (store, verify_callback);

/* load the CA certificates and CRLs */
  if (X509_STORE_load_locations (store, CA_FILE, CA_DIR) != 1)
    {
      fprintf (stderr, "Error loading the CA file or directory\n");
      goto err;
    }

  if (X509_STORE_set_default_paths (store) != 1)
    {
      fprintf (stderr, "Error loading the system-wide CA certificates\n");
      goto err;
    }

  if (!(lookup = X509_STORE_add_lookup (store, X509_LOOKUP_file ())))
    {
      fprintf (stderr, "Error creating X509_LOOKUP object\n");
      goto err;
    }

  return store;

err:
  return NULL;
}

int
main (int argc, char *argv[])
{
  int sign;
  X509 *cert;
  EVP_PKEY *pkey;
  STACK_OF (X509) * chain = NULL;
  X509_STORE *store;
  PKCS7 *pkcs7;
  FILE *fp;
  BIO *in, *out, *pkcs7_bio;

  OpenSSL_add_all_algorithms ();
  ERR_load_crypto_strings ();
  seed_prng ();

  --argc, ++argv;
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s (sign|verify) [privkey.pem] cert.pem ...\n", argv[0]);
      goto err;
    }
  if (!strcmp (*argv, "sign"))
    sign = 1;
  else if (!strcmp (*argv, "verify"))
    sign = 0;
  else
    {
      fprintf (stderr, "Usage: %s (sign|verify) [privkey.pem] cert.pem ...\n", argv[0]);
      goto err;
    }
  --argc, ++argv;

/* setup the BIO objects for stdin and stdout */
  if (!(in = BIO_new_fp (stdin, BIO_NOCLOSE)) ||
      !(out = BIO_new_fp (stdout, BIO_NOCLOSE)))
    {
      fprintf (stderr, "Error creating BIO objects\n");
      goto err;
    }
  if (sign)
    {

/* read the signer private key */
      if (!(fp = fopen (*argv, "r")) ||
	  !(pkey = PEM_read_PrivateKey (fp, NULL, NULL, NULL)))
	{
	  fprintf (stderr, "Error reading signer private key in %s\n", *argv);
	  goto err;
	}
      fclose (fp);
      --argc, ++argv;
    }
  else
    {
/* create the cert store and set the verify callback */
      if (!(store = create_store ()))
	fprintf (stderr, "Error setting up X509_STORE object\n");
    }

/* read the signer certificate */
  if (!(fp = fopen (*argv, "r")) ||
      !(cert = PEM_read_X509 (fp, NULL, NULL, NULL)))
    {
      ERR_print_errors_fp (stderr);
      fprintf (stderr, "Error reading signer certificate in %s\n", *argv);
      goto err;
    }
  fclose (fp);
  --argc, ++argv;

  if (argc)
    chain = sk_X509_new_null ();
  while (argc)
    {
      X509 *tmp;

      if (!(fp = fopen (*argv, "r")) ||
	  !(tmp = PEM_read_X509 (fp, NULL, NULL, NULL)))
	{
	  fprintf (stderr, "Error reading chain certificate in %s\n", *argv);
	  goto err;
	}
      sk_X509_push (chain, tmp);
      fclose (fp);
      --argc, ++argv;
    }

  if (sign)
    {
      if (!(pkcs7 = PKCS7_sign (cert, pkey, chain, in, 0)))
	{
	  fprintf (stderr, "Error making the PKCS#7 object\n");
	  goto err;
	}
      if (SMIME_write_PKCS7 (out, pkcs7, in, 0) != 1)
	{
	  fprintf (stderr, "Error writing the S/MIME data\n");
	  goto err;
	}
    }
  else				/* verify */
    {
      if (!(pkcs7 = SMIME_read_PKCS7 (in, &pkcs7_bio)))
	{
	  fprintf (stderr, "Error reading PKCS#7 object\n");
	  goto err;
	}
      if (PKCS7_verify (pkcs7, chain, store, pkcs7_bio, out, 0) != 1)
	{
	  fprintf (stderr, "Error writing PKCS#7 object\n");
	  goto err;
	}
      else
	fprintf (stdout, "Certifiate and Signature verified!\n");
    }

  return 0;
err:
  return -1;
}
