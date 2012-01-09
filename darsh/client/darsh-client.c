#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <netdb.h>
#include <sys/ioctl.h>

#include "darsh-client.h"
#include "darsh-common.h"
#include "darshell.h"

char *peer_host = "localhost";

int interval = 3;
char *interface_name = "eth0";
char *host_id = "client01";

/* this affects to client_table
* if you change this protocol, 
* you need to change peer_server
*/
char *devide_letter = ", ";

int read_line(int socket, char *p)
{
	int len = 0;
	while (1) {
		int ret;
		ret = read(socket, p, 1);
		if (ret == -1) {
			perror("read");
			return -1;
		} else if (ret == 0) {
			break;
		}
		if (*p == '\n') {
			p++;
			len++;
			break;
		}
		if (*p == '\0') {
			len++;
			break;
		}
		p++;
		len++;
	}
	*p = '\0';
	return len;
}

char *get_host(void)
{
	struct utsname *uname_info;
	uname_info = (struct utsname *)malloc(sizeof(struct utsname));
	if (uname(uname_info) < 0) {
		fprintf(stderr, "failed to uname()");
	}

	return uname_info->nodename;
}

char *get_ip(void) {
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *get_client_info()
{

	char *client_info = get_host();
	char *client_ip = get_ip();

	strcat(client_info, devide_letter);
	strcat(client_info, client_ip);
	strcat(client_info, "\n");

	return client_info;
}


int server(char **envp)
{
	printf("******* Info Server *******\n");

	int sock_fd, len, status;
	//char buf[BUF_LEN]; // unused
	struct sockaddr_in serv;
	unsigned short port = TABLE_SERVER_PORT;
	char *hostname = peer_host;
	//char *client_info = get_client_info(); // change for refactor
	char *client_info;
	pid_t pid, cpid;

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
	}

	serv.sin_family = PF_INET;
	serv.sin_port = htons(port);
	inet_aton(hostname, &serv.sin_addr);

	if (connect(sock_fd, (struct sockaddr*)&serv, sizeof(struct sockaddr_in)) < 0)
	{
		perror("connect");
		return -2;
	}

	/* Fork server functions to two; update and remote access */

	pid = fork();
	if(pid<0){
	  perror("fork");
	  exit(-1);
	}

	/* Child process: remote access */

	if( pid==0 ){
	  shellserv(envp, port);
	  cpid = wait(&status);
	}

	/* Parent process: updating info */
	else{
	  for(;;){
	  	char hoge[BUF_LEN] = {0};
	  	client_info = get_client_info();
	  	printf("send:%s\n", client_info);
		len = send(sock_fd, client_info, strlen(client_info), 0);
		sleep(interval);
	  }
	 }

	close(sock_fd);
	return 0;
}

int client(void)
{
	printf("******* Client Mode *******\n");

	int sock_fd, len;
	char buf[BUF_LEN];
	struct sockaddr_in serv;
	unsigned short port = PEER_SERVER_PORT;
	char *hostname = peer_host;

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
	}

	serv.sin_family = PF_INET;
	serv.sin_port = htons(port);
	inet_aton(hostname, &serv.sin_addr);

	if (connect(sock_fd, (struct sockaddr*)&serv, sizeof(struct sockaddr_in)) < 0)
	{
		perror("connect");
		return -2;
	}

	/* This First Peer Server Message is important for clean TCP connection */
	len = read_line(sock_fd, buf);
	printf("First Peer Server Message: %s\n", buf);

	while (1) {
		char get[BUF_LEN] = {0};
		printf("-> %s", get);
		scanf("%s", get);

		len = send(sock_fd, get, strlen(get), 0);
		len = read_line(sock_fd, buf);
		printf("%s found: %s\n", get, buf);
		shellclnt(buf, port);
	}

	close(sock_fd);
	return 0;
}

void usage(void)
{
	printf("usage: darsh-client\n");
	printf("Server Mode: darsh-client s\n");
	printf("Client Mode: darsh-client c\n");
}

#define INFO_SERVER_MODE "s"
#define CLIENT_MODE "c"
int main(int argc, char **argv, char **envp)
{
	int ret;
	char *opt = argv[1];

	if (argc <= 1) {
		usage();
		return 0;
	}

	if (strncmp(INFO_SERVER_MODE, opt, strlen(INFO_SERVER_MODE)) == 0) {
		ret = server(envp);
	} else if (strncmp(CLIENT_MODE, opt, strlen(CLIENT_MODE)) == 0) {
		ret = client();
	}

	return 0;
}
