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
#include "sslcommon.h"

char *permute_str(const char *a, const char *b, const char *c)
{
	int i = 0, max_a, max_b;
	char result[BUF_LEN];
	result[0] = "\0";

	max_a = strlen(a);
	max_b = strlen(b);
	for (i = 0; i <= max_a; i++) {
		if (0 == strncmp(&a[i], b, max_b)) {
			strcat(result, c);
			i += max_b - 1;
		} else {
			strncat(result, &a[i], 1);
		}
	}
	return result;
}

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

char *get_ip(char *interface_name) {
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *get_client_info(char *host_id, char *interface_name)
{

	//char *client_info = get_host();
	char client_info[BUF_LEN];
	strcpy(client_info, host_id);
	char *client_ip = get_ip(interface_name);

	strcat(client_info, devide_letter);
	strcat(client_info, client_ip);
	strcat(client_info, "\n");

	return client_info;
}

char *get_client_info_FQDN(char *host_id)
{
	char client_info[BUF_LEN];
	char host_name[BUF_LEN];
	strcpy(client_info, host_id);
	strcpy(host_name, get_host());

	strcat(client_info, devide_letter);
	strncat(client_info, host_name, strlen(host_name));

	strcat(client_info, "\n");

	return client_info;
}

int server(char **envp, char *peer_host, int interval, char *interface_name, char *host_id)
{
	printf("******* Info Server *******\n");

	int sock_fd, len, status;
	struct sockaddr_in serv;
	unsigned short info_port = TABLE_SERVER_PORT;
	unsigned short port = WAIT_SERVER_PORT;
	char *hostname = peer_host;
	//char *client_info;
	pid_t pid, cpid;

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("socket");
	}

	serv.sin_family = PF_INET;
	serv.sin_port = htons(info_port);
	inet_aton(hostname, &serv.sin_addr);

	printf("try to connect to %s\n", hostname);
	if (connect(sock_fd, (struct sockaddr*)&serv, sizeof(struct sockaddr_in)) < 0)
	{
		perror("connect");
		return -2;
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid  != 0) {
		/* insert darshell process function */
		//ShellHandler(envp);
		sslserv();
		cpid = wait(&status);
	} else {
		while (1) {
			//client_info = get_client_info(host_id, interface_name);
			char client_info[BUF_LEN];
			strcpy(client_info, get_client_info_FQDN(host_id));
			printf("send: %s\n", client_info);
			len = send(sock_fd, client_info, strlen(client_info), 0);
			sleep(interval);
		}
	}

	close(sock_fd);
	return 0;
}

int client(char *peer_host)
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
		char fqdn[BUF_LEN];
		strcpy(fqdn, buf);
		fqdn[strlen(fqdn)-1] = '\0';
		printf("%s found: %s\n", get, fqdn);
		sslclnt(fqdn);
		//shellclnt(buf, port);
	}

	close(sock_fd);
	return 0;
}

void usage_server(void)
{
	printf("Usage Server Mode: darsh-client s host interface client_id\n");
}

void usage_client(void)
{
	printf("Usage Client Mode: darsh-client c host\n");
}

void usage(void)
{
	usage_server();
	usage_client();
}

#define INFO_SERVER_MODE "s"
#define CLIENT_MODE "c"
/*
int main()
{
	printf("%s\n", get_client_info_FQDN("hoge"));
	return 0;
}
*/

int main(int argc, char **argv, char **envp)
{
	int ret;
	char *opt = argv[1];

	char *peer_host;
	int interval = 3;
	char *host_id;
	char *interface_name;

	if (argc <= 2) {
		usage();
		return 0;
	}

	if (strncmp(INFO_SERVER_MODE, opt, strlen(INFO_SERVER_MODE)) == 0) {

		if (argc <= 4) {
			usage_server();
			return 0;
		} else {
			peer_host = argv[2];
			interface_name = argv[3];
			host_id = argv[4];
		}
		peer_host = argv[2];
		ret = server(envp, peer_host, interval, interface_name, host_id);
	} else if (strncmp(CLIENT_MODE, opt, strlen(CLIENT_MODE)) == 0) {
		if (argc <= 2) {
			usage_client();
			return 0;
		} else {
			peer_host = argv[2];
		}
		ret = client(peer_host);
	}


	return 0;
}

