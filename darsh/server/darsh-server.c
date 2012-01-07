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

#include "darsh.h"
#include "darsh-common.h"

char *table_split = "\n";

CLIENT_INFO client_info_server[FD_SETSIZE];

int server_listening_socket;
struct sockaddr_in server_sin;

int
server_accept_new_client(int sock)
{
	int len;
	int new_socket;
	struct hostent *peer_host;
	struct sockaddr_in peer_sin;

	len = sizeof(server_sin);
	new_socket = accept(server_listening_socket, (struct sockaddr *)&server_sin, &len);

	if (sock < 0) {
		printf("no socket error\n");
	}

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

	strncpy(client_info_server[new_socket].hostname, peer_host->h_name, sizeof client_info_server[new_socket].hostname);
	strncpy(client_info_server[new_socket].ipaddr, inet_ntoa(peer_sin.sin_addr),
			sizeof client_info_server[new_socket].ipaddr);

	client_info_server[new_socket].port = ntohs(peer_sin.sin_port);
	time(&client_info_server[new_socket].last_access);

	printf("Connect: %s (%s) port %d  descriptor %d\n",
			client_info_server[new_socket].hostname,
			client_info_server[new_socket].ipaddr,
			client_info_server[new_socket].port,
			new_socket);
	return new_socket;
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

int read_and_save_table(int sock)
{
	int read_size;
	char buf[BUF_LEN];

	read_size = read(sock, buf, sizeof(buf)-1);

	if (read_size == 0 || read_size == -1) {
		printf("Connection from: %s (%s) port %d  descriptor %d\n",
			client_info_server[sock].hostname,
			client_info_server[sock].ipaddr,
			client_info_server[sock].port,
			sock);
		close(sock);
		client_info_server[sock].last_access = 0;
	} else {
		buf[read_size] = '\0';
		printf("Message from: %s (%s) port %d  descriptor %d:[ %s ]\n",
			client_info_server[sock].hostname,
			client_info_server[sock].ipaddr,
			client_info_server[sock].port,
			sock,
			buf);
		write_to_db_file(buf);
		time(&client_info_server[sock].last_access);
	}
	return read_size;
}

int darsh_server()
{
	int ret;
	int sock_optval = 1;
	int port = TABLE_SERVER_PORT;

	fd_set target_fds;
	fd_set org_target_fds;

	server_listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_listening_socket == -1) {
		perror("socket");
		return -1;
	}

	if (setsockopt(server_listening_socket, SOL_SOCKET, SO_REUSEADDR,
			&sock_optval, sizeof(sock_optval)) == -1) {
		perror("setsockoptval");
		return -1;
	}

	server_sin.sin_family = AF_INET;
	server_sin.sin_port = htons(port);
	server_sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_listening_socket, (struct sockaddr *)&server_sin, sizeof(server_sin)) < 0)
	{
		perror("bind");
		return -1;
	}

	ret = listen(server_listening_socket, SOMAXCONN);
	if (ret == -1) {
		perror("listen");
		return -1;
	}

	printf("Table Server process: watch port %d....\n", port);

	FD_ZERO(&org_target_fds);
	FD_SET(server_listening_socket, &org_target_fds);

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
				printf("Descriptor %d can read\n", i);
				if (i == server_listening_socket) {
					int new_sock;

					new_sock = server_accept_new_client(i);
					if (new_sock != -1) {
						FD_SET(new_sock, &org_target_fds);
					}
				} else {
					int read_size;
					read_size = read_and_save_table(i);

					if (read_size == -1 || read_size == 0) {
						FD_CLR(i, &org_target_fds);
					}
				}
			}
		}
		time(&now_time);

		for (i = 0; i < FD_SETSIZE; i++) {
			if (!FD_ISSET(i, &org_target_fds)) continue;

			if (i == server_listening_socket) continue;
			/*
			if (now_time-10 > client_info_server[i].last_access) {
				printf("No Access over 10sec from %s (%s) port %d  descriptor %d. Stop connection.\n",
					client_info_server[i].hostname,
					client_info_server[i].ipaddr,
					client_info_server[i].port,
					i);
				close(i);

				FD_CLR(i, &org_target_fds);
			}
			*/
		}
	}
	close(server_listening_socket);

	return 0;
}

