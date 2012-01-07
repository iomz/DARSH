#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "darsh.h"

int main(int argc, char **argv)
{
	int status;
	pid_t pid;

	pid = fork();

	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		darsh_peer();
	} else {
		darsh_server();
	}

	return 0;
}
