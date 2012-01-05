#include "darshell.h"

char *hostname = "127.0.0.1";

int main(int argc, char **argv){
  int shsock;
  int recvMsgSize, sendMsgSize;
  char buf[BUFSIZ];
  struct sockaddr_in shserv;
  unsigned short shport;
  char *shhost = hostname;

  if(argc != 2){
    fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
    exit(1);
  }
  shport = atoi(argv[1]);

  if((shsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    errorhandler("socket");

  shserv.sin_family = PF_INET;
  shserv.sin_port = htons(shport);
  inet_aton(shhost, &shserv.sin_addr);

  if( connect(shsock, (struct sockaddr *)&shserv, sizeof(shserv)) < 0 )
    errorhandler("connect");

  printf("\v/*********************************************************************/\n");
  printf("\t/* darsh connection established with %s:%d */", shhost, shport);
  printf("\n/*********************************************************************/\n\n");

  for(;;){
    recvMsgSize = recv(shsock, buf, BUFSIZ, 0); //recv prompt
    printf("%s", buf);
    memset(buf, 0, BUFSIZ);
    fgets(buf, BUFSIZ, stdin);
    sendMsgSize = send(shsock, buf, strlen(buf), 0); //send command
    if( strncmp(buf, "exit", 4)==0 || strncmp(buf, "quit", 4)==0 )
      break;
    memset(buf, 0, BUFSIZ);
    recvMsgSize = recv(shsock, buf, BUFSIZ, 0);
    printf("%s", buf);
  }

  printf("\n\n...darsh connection closed by client...\n\n");
  close(shsock);
  return 0;
}
