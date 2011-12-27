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

#include "darsh.h"

int interval = 3;
char *interface_name = "br0";
char *host_id = "client01";

/* this affects to client_table
* if you change this protocol, 
* you need to change peer_server
*/
char *devide_letter = ", ";

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


int main(int argc, char **argv)
{
	int sock_fd, len;
	char buf[BUF_LEN];
	struct sockaddr_in serv;
	unsigned short port = 5000;
	char *hostname = "localhost";

	char *client_info = get_client_info();

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

	while (1) {
		printf("send:%s\n", client_info);
		len = send(sock_fd, client_info, strlen(client_info), 0);
		sleep(interval);
/*	
		printf("->");
		fgets(buf, BUF_LEN, stdin);

		len = strlen(buf);

		len = send(sock_fd, buf, len, 0);
		len = recv(sock_fd, buf, len, 0);

		buf[len] = '\0';
		printf("<- %s\n", buf);
*/
	}

	close(sock_fd);
	return 0;
}

