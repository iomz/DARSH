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

void HandleError(const char *err){
  perror(err);
  exit(1);
}
void SendReq(int clntSock, char *reqfile){
  FILE *fp = fopen(reqfile, "r");
  char tmp[256],tmp2[BUFSIZ];
  memset(tmp, 0, 256);
  while(fgets(tmp, 256, fp) != NULL){
    printf("%s", tmp);  
    strcat(tmp2, tmp);
  }
  fclose(fp);
  if( (send(clntSock, tmp2, strlen(tmp2), 0)) < 0 )
    HandleError("send() failed");
  printf("%s sent!\n", reqfile);
}

void ReceiveCert(int clntSock, char *certfile){
  char buf[BUFSIZ];
  int recved;

  if( (recved = recv(clntSock, buf, BUFSIZ, 0)) < 0)
    HandleError("recv() failed");

  int fdout = open(certfile, O_WRONLY|O_CREAT, 0644 );
  if(fdout == -1)
    HandleError("open");
  dprintf(fdout, "%s", buf);
  close(fdout);
  printf("%s received!\n", certfile);
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

  SendReq(clntSock, "clntreq.pem");
  ReceiveCert(clntSock, "clntcert.pem");
  sleep(1);
  SendReq(clntSock, "servreq.pem");
  ReceiveCert(clntSock, "servcert.pem");

  return 0;
}
