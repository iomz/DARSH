#include "darshell.h"

int main(int argc, char **argv, char **envp){

  int servSock;
  int clntSock;
  struct sockaddr_in servAddr;
  struct sockaddr_in clntAddr;
  unsigned short servPort;
  unsigned int clntLen;

  if(argc != 2){
    fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
    exit(1);
  }
  servPort = atoi(argv[1]);
  
  /* Create socket for incoming connections */
  if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    errorhandler("socket");
      
  /* Construct local address structure */
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);

  if(bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    errorhandler("bind");

  if(listen(servSock, MAXPENDING) < 0)
    errorhandler("listen");

  printf("SHELL server <%s> is waiting for clients at port %d\n", "localhost", servPort);

  for(;;){
      clntLen = sizeof(clntAddr);

      if((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0)
	errorhandler("accept");

      printf("Connected to client %s\n", inet_ntoa(clntAddr.sin_addr));

      HandleTCPClient(clntSock, envp);
    }

  /* NOT REACHED HERE*/
  return 0;
}
