#include "certscommon.h"

void seed_prng(void)
{
  RAND_load_file("/dev/urandom", 1024);
}

void
handle_error (const char *file, int lineno, const char *msg)
{
  fprintf (stderr, "** %s:%i %s\n", file, lineno, msg);
  ERR_print_errors_fp (stderr);
  exit (-1);
}
