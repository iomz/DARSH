#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define CREQ "clntreq.pem"
#define SREQ "servreq.pem"

void HandleError(const char *err){
  perror(err);
  exit(1);
}

int main(void){
  
  int clntSock = -1;
  int error;
  struct addrinfo hints, *res, *res0;
  const char *xcertIP = "localhost";
  const char *xcertPort = "22222";
  /* Load up address structure */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int *pid;

  if((error = getaddrinfo(xcertIP, xcertPort, &hints, &res0)) != 0)
    HandleError(gai_strerror(error));

  // loop through addrinfos for a good socket, with storing error cause
  const char *cause = NULL;
  for(res = res0; res; res = res->ai_next){    
    clntSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(clntSock < 0){
      cause = "socket()";
      continue;
    }
    /* Try to connect() */
    if(connect(clntSock, res->ai_addr, res->ai_addrlen) < 0){
      cause = "connect()";
      close(clntSock);
      clntSock = -1;
      continue;
    }
    /* Got one socket, proceed */
    break;
  }
  if(clntSock < 0)
    HandleError(cause);
  freeaddrinfo(res0);
  printf("Connected to %s:%s\n>", xcertIP, xcertPort);

  FILE *fp = fopen(CREQ, "r");
  char tmp[BUFSIZ];
  while(fgets(tmp, BUFSIZ, fp) != NULL)
    if( (send(clntSock, tmp, strlen(tmp), 0)) < 0 )
	HandleError("send() failed");
  printf("clntreq sent!\n");
  fclose(fp);

  wait(pid);
  fp = fopen(SREQ, "r");
  char tmp2[BUFSIZ];
  while(fgets(tmp2, BUFSIZ, fp) != NULL)
    if( (send(clntSock, tmp2, strlen(tmp2), 0)) < 0 )
      HandleError("send() failed");
  printf("servreq sent!\n");
  fclose(fp);

  return 0;
}
