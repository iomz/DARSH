#include "darshell.h"

int shellserv(char **envp, unsigned short servPort){
  int servSock;
  int clntSock;
  struct sockaddr_in servAddr;
  struct sockaddr_in clntAddr;
  unsigned int clntLen;
  
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

      if( HandleTCPClient(clntSock, envp) != 0)
	break;
    }

  return 0;
}

int shellclnt(char * shhost, unsigned short shport){
  int shsock;
  int recvMsgSize, sendMsgSize;
  char buf[BUFSIZ];
  struct sockaddr_in shserv;

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

void sethome(char **envp){
  int i;
  for(i=0; envp[i]!=NULL; i++)
    if( strncmp(envp[i], "HOME=", strlen("HOME="))==0 ){
      sscanf( envp[i], "HOME=%s", dpath);
      if( chdir(dpath) != 0)
	perror("HOME directory not found");
      else
	printf("HOME is set to  %s\n", dpath);	      
      break;
    }
  return;
}

void sighandler(int sig){
  fprintf(stderr,"signal(%s) caught.\n", strsignal(sig));
  siglongjmp(env, 1);
  return;
}

void comparse(char *buf, char **args){
  int i=0, argc=0;
  
  while(*buf != '\0'){
    while(*buf==' ' || *buf == '\t'){ // Truncate leading spaces
      *buf = '\0';
      buf++;
    }
    if(i >= MAXARGS-1)
      break;
    /* NULL command */
    if(*buf != '\0'){
      args[i] = buf;
      i++;
    }
    while(*buf!=' ' && *buf!='\t' && *buf!='\0') // Skip one argument
      buf++;
    argc++;
  }//outer while
  args[i] = NULL;

  int fdout = open("/Users/iomz/darsh/darshell/out", O_WRONLY|O_APPEND|O_CREAT, 0644 );
  if(fdout == -1)
    errorhandler("open");
  dprintf(fdout, "Parsed arguments[%d]: ", argc);
  for(i=0; args[i]!=NULL; i++)
    dprintf(fdout, "%s ", args[i]);
  dprintf(fdout, "\n");
  close(fdout);

  return;
}

void errorhandler(char *err){
  perror(err);
  exit(1);
}

void dupcheck(int desc, int fileno){
  if(desc != fileno){
    if(dup2(desc, fileno) != fileno)
      errorhandler("dup2");
    close(desc);
  }
}

int HandleTCPClient(int clntSocket, char **envp){
  char buf[BUFSIZ];
  char *args[MAXARGS], prompt[MAXPATHLEN];
  int status, i, recvMsgSize=0, sendMsgSize=0;
  pid_t pid, cpid;
  struct sigaction act;
  int fd[2];

  /* Set home as default path */
  sethome(envp);

  /* Signale hanlder initialization */
  act.sa_handler = sighandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if( sigaction(SIGINT, &act, NULL) < 0 )
    errorhandler("sigaction");
 
  /* Commad handler */
  for(;;){
    /* Pipe initialization */
    if( pipe(fd)!=0 )
      errorhandler("pipe");

    if(sigsetjmp(env, 1)==0)
      ;
    else
      wait(&status);
    
    /* Prompting */
    memset(prompt, 0, MAXPATHLEN);
    snprintf(prompt, 11+strlen(dpath), "darsh [%s]> ", dpath);
    //fprintf(stdout, "%s", prompt);

    /* Send prompt to server */
    sendMsgSize = strlen(prompt);
    if(send(clntSocket, prompt, sendMsgSize, 0) != sendMsgSize)
      errorhandler("send");
    
    /****************************************/
    /* wait until the next message is typed */
    /****************************************/

    /* Receive message from client */
    memset(buf, 0, BUFSIZ);
    if((recvMsgSize = recv(clntSocket, buf, BUFSIZ, 0)) < 0)
      errorhandler("recv() failed");

    //printf("%d bytes recieved!\n", recvMsgSize);
    /* printf("Yes!!!\n"); */
    
    /* Format arg string list and parse the received message into it */
    for(i=0; args[i]!=NULL ; i++)
      args[i] = NULL;
    
    comparse(buf, args);

    /* Handle no input */
    if(args[0]==NULL)
      continue;

    if( strcmp(args[0], "exit")==0 || strcmp(args[0], "quit")==0 ){
      fprintf(stderr,"exiting.\n");
      break;
    }
    /* chdir handler */
    else if( strcmp(args[0], "cd")==0 ){
      if(args[1] != NULL){
	if( chdir(args[1]) < 0 ){
	  fprintf(stdout, "%s: ", args[1]);
	  perror("chdir");
	}
	else if( strncmp(args[1], "/", 1)==0 )
	  strcpy(dpath, args[1]);
	else if( strncmp(args[1], "..", 2)==0 )
	  strcpy(strrchr(dpath, '/'), args[2]);
	else{
	  strcat(dpath, "/");
	  strcat(dpath, args[1]);
	}
      }
      continue;
    }

    pid = fork();
    if( pid < 0 ){
      perror("fork");
      continue;
    }

    /* child process*/
    if( pid == 0 ){
      close(fd[0]);
      dupcheck(fd[1], STDOUT_FILENO);
      act.sa_handler = SIG_DFL;
      if(sigaction(SIGINT, &act, NULL) < 0)
	perror("sigaction");

      if ( execvp(args[0], args) < 0 )
	errorhandler("execvp");

      cpid=wait(&status);
      close(fd[1]);
    }//if
    else{ /* parent process */
      memset(buf, 0, BUFSIZ);
      close(fd[1]);
      read(fd[0], buf, BUFSIZ);

      /* Send message from server */
      if( (sendMsgSize = send(clntSocket, buf, BUFSIZ, 0)) < 0 )
	errorhandler("shell out");
      //printf("%d bytes transmitted.\n", sendMsgSize);

      pid = wait(&status);
      close(fd[0]);
    }//else
  }//for
    
  /* Close client socket */
  close(clntSocket);
  return 0;
}
