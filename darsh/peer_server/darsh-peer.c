#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include "darsh.h"

char *interface_name = "eth0";
char *host_id = "client_002";

char *db_filename = "client_table";
char *table_split = "\n";

struct darsh_client {
	char *client_name;
	char *ip_address;
};

typedef struct CLIENT_INFO {
	char hostname[BUF_LEN];
	char ipaddr[BUF_LEN];
	int port;
	time_t last_access;
} CLIENT_INFO;

CLIENT_INFO client_info[FD_SETSIZE];

int listening_socket;
struct sockaddr_in sin;

void find_table(char *string, char *cmp) {
	char *str = string;
	regex_t preg;
	size_t nmatch = 5;
	regmatch_t pmatch[nmatch];
	int i, j;

	//if (regcomp(&preg, "([[:digit:]]+), ([[:digit:]]+), ([[:digit:]]+)", REG_EXTENDED | REG_NEWLINE) != 0) {
	if (regcomp(&preg, "(.+), (.+)", REG_EXTENDED | REG_NEWLINE) != 0) {
		printf("regex compile failed.\n");
	}

	printf("String = %s\n", str);

	if (regexec(&preg, str, nmatch, pmatch, 0) != 0) {
		printf("No match");
	} else {
		for (i = 0; i < nmatch; i++) {
			printf("Match position = %d, %d, str = ", (int)pmatch[i].rm_so, (int)pmatch[i].rm_eo);
			if (pmatch[i].rm_so >= 0 && pmatch[i].rm_eo >= 0) {
				for (j = pmatch[i].rm_so; j < pmatch[i].rm_eo; j++) {
					putchar(str[j]);
				}
			}
			printf("\n");
		}
	}
	regfree(&preg);
}

//struct darsh_client *parse_darsh_table(int num)
void parse_darsh_table(int num)
{
	struct darsh_client *client;
	char s[BUF_LEN];
	FILE *db;
	if ((db = fopen(db_filename, "r")) < 0) {
		printf("cannot open file");
	}

	while (fgets(s, BUF_LEN, db) != NULL) {
		printf("%s", s);
	}

	fclose(db);
}

char *search_host_ip(char *hostname)
{
	FILE *fp;
	char s[BUF_LEN];
	char *ret;
	char *ret_val;

	if ((fp = fopen(db_filename, "r")) == NULL) {
		printf("file open error.\n");
	}

	printf("[search_host_ip] hostname is %s is this\n", hostname);

	while (fgets(s, BUF_LEN, fp) != NULL) {
		printf("fgets is [%s]\n", s);
		printf("find .......");
		printf("find [%s]\n", hostname);
		ret = strstr(s, hostname);

		if (ret != NULL) {
			ret_val = ret+strlen(hostname)+PROTO_OFFSET;
			printf("ret_val = %s\n", ret_val);
			//strcpy(val ,ret_val);
			printf("hogehoge: %s", ret_val);
			break;
		} else {
			printf("ret is NULL\n");
			ret_val = NULL;
		}
	}
	fclose(fp);
	return ret_val;
}
/*
int main()
{
	char *host = "mobe";
	char val[BUF_LEN];
	char *ret = search_host_ip(host);
	if (ret != NULL)
		strncpy(val, ret, BUF_LEN);

	printf("val is %s\n", val);

	return 0;
}
*/
int
accept_new_client(int sock)
{
	int len;
	int new_socket;
	struct hostent *peer_host;
	struct sockaddr_in peer_sin;

	len = sizeof(sin);
	new_socket = accept(listening_socket, (struct sockaddr *)&sin, &len);

	if (new_socket == -1) {
		perror("accept");
		return -1;
	}

	if (new_socket > FD_SETSIZE-1) {
		return -1;
	}

	len = sizeof(peer_sin);
	getpeername(new_socket, (struct sockaddr *)&peer_sin, &len);

	peer_host = gethostbyaddr((char *)&peer_sin.sin_addr.s_addr, sizeof(peer_sin.sin_addr), AF_INET);

	strncpy(client_info[new_socket].hostname, peer_host->h_name, sizeof client_info[new_socket].hostname);
	strncpy(client_info[new_socket].ipaddr, inet_ntoa(peer_sin.sin_addr),
			sizeof client_info[new_socket].ipaddr);

	client_info[new_socket].port = ntohs(peer_sin.sin_port);
	time(&client_info[new_socket].last_access);

	printf("Connect: %s (%s) port %d  descriptor %d\n",
			client_info[new_socket].hostname,
			client_info[new_socket].ipaddr,
			client_info[new_socket].port,
			new_socket);
	return new_socket;
}

int read_and_save_table(int sock)
{
	int read_size;
	char buf[BUF_LEN];
	int i;
	read_size = read(sock, buf, sizeof(buf)-1);
	for (i = 0; i < sizeof(buf); i++) {
		if (buf[i] == '\n')
			buf[i] = '\0';
	}

	if (read_size == 0 || read_size == -1) {
		/*printf("Connection from: %s (%s) port %d  descriptor %d\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock);*/
		close(sock);
		client_info[sock].last_access = 0;
	} else {
		buf[read_size] = '\0';
		printf("Message from: %s (%s) port %d  descriptor %d:[ %s ]\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock,
			buf);
		//write(sock, buf, strlen(buf));
		write_to_db_file(buf);
		time(&client_info[sock].last_access);
	}
	return read_size;
}

int read_peer_sock(int sock)
{
	int read_size;
	char buf[BUF_LEN];
	char *ret;
	char val[BUF_LEN];

	read_size = read(sock, buf, sizeof(buf)-1);

	if (read_size == 0 || read_size == -1) {
		/*printf("Connection from: %s (%s) port %d  descriptor %d\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock);*/
		close(sock);
		client_info[sock].last_access = 0;
	} else {
		buf[read_size] = '\0';
		/*printf("Message from: %s (%s) port %d  descriptor %d:[ %s ]\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock,
			buf);*/
		//write(sock, buf, strlen(buf));
		printf("goto search_host_ip\n");
		printf("buf is %s is this ?\n", buf);
		ret = search_host_ip(buf);
		printf("ret is %s\n", ret);
		if (ret != NULL) {
			strncpy(val, ret, BUF_LEN);
			write(sock, val, strlen(buf));
		} else {
			char *sorry = "Sorry, do not have the host ip.\n";
			write(sock, sorry, strlen(sorry));
		}
		time(&client_info[sock].last_access);
	}
	return read_size;
}

int read_and_reply(int sock)
{
	int read_size;
	char buf[BUF_LEN];

	read_size = read(sock, buf, sizeof(buf)-1);

	if (read_size == 0 || read_size == -1) {
		printf("Connection from: %s (%s) port %d  descriptor %d\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock);
		close(sock);
		client_info[sock].last_access = 0;
	} else {
		buf[read_size] = '\0';
		printf("Message from: %s (%s) port %d  descriptor %d:[ %s ]\n",
			client_info[sock].hostname,
			client_info[sock].ipaddr,
			client_info[sock].port,
			sock,
			buf);
		write(sock, buf, strlen(buf));
		time(&client_info[sock].last_access);
	}
	return read_size;
}

void write_to_db_file(char *content)
{
	FILE *db;
	if ((db = fopen(db_filename, "a")) == NULL) {
		printf("file cannot open");
	}
	fputs(content, db);
	fputs(table_split, db);
	fclose(db);
	return;
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
		p++;
		len++;
	}
	*p = '\0';
	return len;
}

char *get_hostname(void) {

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

int main(int argc, char **argv)
{
	int connected_socket;
	//int listening_socket;
	//struct sockaddr_in sin;
	int len, ret;
	int sock_optval = 1;
	int port = 5000;

	fd_set target_fds;
	fd_set org_target_fds;

	listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_socket == -1) {
		perror("socket");
		return -1;
	}

	if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
			&sock_optval, sizeof(sock_optval)) == -1) {
		perror("setsockoptval");
		return -1;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listening_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("bind");
		return -1;
	}

	ret = listen(listening_socket, SOMAXCONN);
	if (ret == -1) {
		perror("listen");
		return -1;
	}

	printf("watch port %d....\n", port);

	FD_ZERO(&org_target_fds);
	FD_SET(listening_socket, &org_target_fds);

	while (1) {
		int i;
		time_t now_time;
		struct timeval waitval;
		waitval.tv_sec = 2;
		waitval.tv_usec = 500;

		memcpy(&target_fds, &org_target_fds, sizeof(org_target_fds));

		select(FD_SETSIZE, &target_fds, NULL, NULL, &waitval);

		for (i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &target_fds)) {
				//printf("Descriptor %d can read\n", i);
				if (i == listening_socket) {
					int new_sock;

					new_sock = accept_new_client(i);
					if (new_sock != -1) {
						FD_SET(new_sock, &org_target_fds);
					}
				} else {
					int read_size;
					//read_size = read_and_save_table(i);
					read_size = read_peer_sock(i);
					if (read_size == -1 || read_size == 0) {
						FD_CLR(i, &org_target_fds);
					}
				}
			}
		}
		time(&now_time);

		for (i = 0; i < FD_SETSIZE; i++) {
			if (!FD_ISSET(i, &org_target_fds)) continue;

			if (i == listening_socket) continue;

			if (now_time-10 > client_info[i].last_access) {
				printf("No Access over 10sec from %s (%s) port %d  descriptor %d. Stop connection.\n",
					client_info[i].hostname,
					client_info[i].ipaddr,
					client_info[i].port,
					i);
				close(i);

				FD_CLR(i, &org_target_fds);
			}
		}
	}
	close(listening_socket);


	while (1) {
		struct hostent *peer_host;
		struct sockaddr_in peer_sin;

		len = sizeof(peer_sin);

		connected_socket = accept(listening_socket, (struct sockaddr *)&peer_sin, &len);
		if (connected_socket == -1) {
			perror("accep");
			return -1;
		}

		peer_host = gethostbyaddr((char *)&peer_sin.sin_addr.s_addr,
					sizeof(peer_sin.sin_addr), AF_INET);
		if (peer_host == NULL) {
			printf("gethostbyname failed\n");
			return -1;
		}

		printf("connect: %s [%s] port %d\n",
			peer_host->h_name,
			inet_ntoa(peer_sin.sin_addr),
			ntohs(peer_sin.sin_port)
			);

		while (1) {
			int read_size;
			char buf[BUF_LEN];
			char answer[BUF_LEN];

			read_size = read_line(connected_socket, buf);
			if (read_size == 0) break;

			//write_to_db_file(buf);

			//write(connected_socket, buf, strlen(buf));
		}

		printf("connection is interrupted.\ncontinue to watch port %d\n", port);
		}
		ret = close(connected_socket);
		if (ret == -1) {
			perror("close");
			return -1;
	}

	return 0;
}

