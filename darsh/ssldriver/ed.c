#include "sslcommon.h"

int
main (int argc, char *argv[])
{
  int encrypt;
  PKCS7 *pkcs7;
  const EVP_CIPHER *cipher;
  STACK_OF (X509) * certs;
  X509 *cert;
  EVP_PKEY *pkey;
  FILE *fp;
  BIO *pkcs7_bio, *in, *out;

  OpenSSL_add_all_algorithms ();
  ERR_load_crypto_strings ();
  seed_prng ();

  --argc, ++argv;
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s (encrypt|decrypt) [privkey.pem] cert.pem ...\n", argv[0]);
      goto err;
    }
  if (!strcmp (*argv, "encrypt"))
    encrypt = 1;
  else if (!strcmp (*argv, "decrypt"))
    encrypt = 0;
  else
    {
      fprintf (stderr, "Usage: %s (encrypt|decrypt) [privkey.pem] cert.pem ...\n", argv[0]);
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

  if (encrypt)
    {
/* choose cipher and read in all certificates as encryption targets */
      cipher = EVP_des_ede3_cbc ();
      certs = sk_X509_new_null ();

      while (argc)
	{
	  X509 *tmp;

	  if (!(fp = fopen (*argv, "r")) ||
	      !(tmp = PEM_read_X509 (fp, NULL, NULL, NULL)))
	    {
	      fprintf (stderr, "Error reading encryption certificate in %s\n",
		       *argv);
	      goto err;
	    }
	  sk_X509_push (certs, tmp);
	  fclose (fp);
	  --argc, ++argv;
	}

      if (!(pkcs7 = PKCS7_encrypt (certs, in, cipher, 0)))
	{
	  ERR_print_errors_fp (stderr);
	  fprintf (stderr, "Error making the PKCS#7 object\n");
	  goto err;
	}
      if (SMIME_write_PKCS7 (out, pkcs7, in, 0) != 1)
	{
	  fprintf (stderr, "Error writing the S/MIME data\n");
	  goto err;
	}
    }
  else
    {
      if (!(fp = fopen (*argv, "r")) ||
	  !(pkey = PEM_read_PrivateKey (fp, NULL, NULL, NULL)))
	{
	  fprintf (stderr, "Error reading private key in %s\n", *argv);
	  goto err;
	}
      fclose (fp);
      --argc, ++argv;
      if (!(fp = fopen (*argv, "r")) ||
	  !(cert = PEM_read_X509 (fp, NULL, NULL, NULL)))
	{
	  fprintf (stderr, "Error reading decryption certificate in %s\n",
		   *argv);
	  goto err;
	}
      fclose (fp);
      --argc, ++argv;

      if (argc)
	fprintf (stderr, "Warning: excess parameters specified. "
		 "Ignoring...\n");

      if (!(pkcs7 = SMIME_read_PKCS7 (in, &pkcs7_bio)))
	{
	  fprintf (stderr, "Error reading PKCS#7 object\n");
	  goto err;
	}
      if (PKCS7_decrypt (pkcs7, pkey, cert, out, 0) != 1)
	{
	  fprintf (stderr, "Error decrypting PKCS#7 object\n");
	  goto err;
	}
    }

  return 0;
err:
  return -1;
}
