/* shelldriver.c */
#include "darshell.h"

int main(int argc, char **argv, char **envp){
  char line[BUFSIZ], command[BUFSIZ], *args[MAXARGS], prompt[MAXPATHLEN];
  int comlen, status;
  pid_t pid, cpid;
  struct sigaction act;
  //int fdout, fd[2];
  //char buf[BUFSIZ];

  /* Set home as default path */
  sethome(envp);

  /* Signale hanlder initialization */
  act.sa_handler = sighandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if( sigaction(SIGINT, &act, NULL) < 0 ){
    perror("sigaction");
    exit(1);
  }
 
  /* Commad handler */
  for(;;){
    /*
    // Pipe initialization
    if( pipe(fd)!=0 ){
      perror("pipe");
      exit(1);
    }
    */

    if(sigsetjmp(env, 1)==0)
      ;
    else
      wait(&status);

    /* Prompting */
    memset(prompt, 0, MAXPATHLEN);
    snprintf(prompt, 11+strlen(dpath), "darsh [%s]> ", dpath);
    fprintf(stdout, "%s", prompt);

    fgets(line, BUFSIZ, stdin);
    comlen = strlen(line);
    strncpy(command, line, comlen-1);
    command[comlen-1] = '\0';

    comparse(command, args);

    if(args[0]==NULL) // no input
      continue;

    if( strcmp(args[0], "exit")==0 || strcmp(args[0], "quit")==0 ){
      fprintf(stderr,"exiting.\n");
      break;
    }
    /* chdir handler */
    else if( strcmp(args[0], "cd")==0 ){
      if(args[1] != NULL){
	if( chdir(args[1]) < 0 ){
	  fprintf(stderr, "%s: ", args[1]);
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

    /* Fork the processes */
    pid = fork();
    if( pid < 0 ){
      perror("fork");
      continue;
    }
    /* child process*/
    if( pid == 0 ){
      //close(fd[0]);
      //dupcheck(fd[1], STDOUT_FILENO);
      act.sa_handler = SIG_DFL;
      if(sigaction(SIGINT, &act, NULL) < 0)
	perror("sigaction");
      if ( execvp(args[0], args) < 0 ){
        perror("execvp");
	exit(1);
      }//inner if
      cpid=wait(&status);
      //close(fd[1]);
    }//outer if
    else{ /* parent process */
      /*
      memset(buf, 0, BUFSIZ);
      close(fd[1]);
      for(i=0; args[i]!=NULL; i++){
	fprintf(stdout, args[i], strlen(args[i]));
	fprintf(stdout, " ", 1);
      }
      fprintf(stdout, "\n", 1);
      read(fd[0], buf, BUFSIZ);
      fprintf(stdout, buf, BUFSIZ);
      close(fd[0]);
      */
      pid = wait(&status);
    }
  }//for
  return 0;
}
