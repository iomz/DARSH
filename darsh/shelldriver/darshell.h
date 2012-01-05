/* darshell.h */
#include <arpa/inet.h>
#include <errno.h>
#include <fts.h>
#include <fcntl.h>
#include <paths.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXARGS 256
#define MAXPATHLEN 512
#define MAXPENDING 5

extern char **environ;
extern int errno;

sigjmp_buf env;
char dpath[MAXPATHLEN];

void sethome(char **);
void sighandler(int);
void comparse(char *, char **);
void dupcheck(int, int);
void errorhandler(char *);
int HandleTCPClient(int, char **);
