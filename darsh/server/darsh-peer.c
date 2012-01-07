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
#include "darsh-common.h"

CLIENT_INFO client_info_peer[FD_SETSIZE];

int peer_listening_socket;
struct sockaddr_in p_sin;

char *search_host_ip(char *hostname)
{
	FILE *fp;
	char s[BUF_LEN];
	char *ret;
	char *ret_val;

	if ((fp = fopen(db_filename, "r")) == NULL) {
		printf("file open error.\n");
	}

	while (fgets(s, BUF_LEN, fp) != NULL) {

		ret = strstr(s, hostname);

		if (ret != NULL) {
			ret_val = ret+strlen(hostname)+PROTO_OFFSET;
			ret = strstr(ret_val, PROTO_DEVIDE);
			if (ret == NULL) {
				break;
			} else {
				ret_val = NULL;
			}
		} else {
			ret_val = NULL;
		}
	}
	fclose(fp);
	return ret_val;
}

int
peer_accept_new_client(int sock)
{
	int len;
	int new_socket;
	struct hostent *peer_host;
	struct sockaddr_in peer_sin;
	char wellcome[BUF_LEN] = "Wellcome to DARSH!! Please input a hostname you want to connect.\n";

	len = sizeof(p_sin);
	new_socket = accept(peer_listening_socket, (struct sockaddr *)&p_sin, &len);

	if (sock < 0) {
		printf("no sock error\n");
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

	strncpy(client_info_peer[new_socket].hostname, peer_host->h_name, sizeof client_info_peer[new_socket].hostname);
	strncpy(client_info_peer[new_socket].ipaddr, inet_ntoa(peer_sin.sin_addr),
			sizeof client_info_peer[new_socket].ipaddr);

	client_info_peer[new_socket].port = ntohs(peer_sin.sin_port);
	time(&client_info_peer[new_socket].last_access);

	printf("Connect: %s (%s) port %d  descriptor %d\n",
			client_info_peer[new_socket].hostname,
			client_info_peer[new_socket].ipaddr,
			client_info_peer[new_socket].port,
			new_socket);
	/* This Message is important for clean TCP connection. */
	write(new_socket, wellcome, strlen(wellcome));
	return new_socket;
}

int read_peer_sock(int sock)
{
	int read_size;
	char buf[BUF_LEN];
	char *ret;

	read_size = read(sock, buf, sizeof(buf)-1);

	if (read_size == 0 || read_size == -1) {
		printf("Connection from: %s (%s) port %d  descriptor %d\n",
			client_info_peer[sock].hostname,
			client_info_peer[sock].ipaddr,
			client_info_peer[sock].port,
			sock);
		close(sock);
		client_info_peer[sock].last_access = 0;
	} else {
		buf[read_size] = '\0';
		printf("Message from: %s (%s) port %d  descriptor %d:[ %s ]\n",
			client_info_peer[sock].hostname,
			client_info_peer[sock].ipaddr,
			client_info_peer[sock].port,
			sock,
			buf);
		ret = search_host_ip(buf);
		if (ret != NULL) {
			write(sock, ret, strlen(ret));
		} else {
			char *sorry = "Sorry, do not have the host ip.\n";
			write(sock, sorry, strlen(sorry));
		}
		time(&client_info_peer[sock].last_access);
	}
	return read_size;
}

int peer_read_line(int socket, char *p)
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
 
int darsh_peer()
{
	int ret;
	int sock_optval = 1;
	int port = PEER_SERVER_PORT;

	fd_set target_fds;
	fd_set org_target_fds;

	peer_listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (peer_listening_socket == -1) {
		perror("socket");
		return -1;
	}

	if (setsockopt(peer_listening_socket, SOL_SOCKET, SO_REUSEADDR,
			&sock_optval, sizeof(sock_optval)) == -1) {
		perror("setsockoptval");
		return -1;
	}

	p_sin.sin_family = AF_INET;
	p_sin.sin_port = htons(port);
	p_sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(peer_listening_socket, (struct sockaddr *)&p_sin, sizeof(p_sin)) < 0)
	{
		perror("bind");
		return -1;
	}

	ret = listen(peer_listening_socket, SOMAXCONN);
	if (ret == -1) {
		perror("listen");
		return -1;
	}

	printf("Peer Server process: watch port %d....\n", port);

	FD_ZERO(&org_target_fds);
	FD_SET(peer_listening_socket, &org_target_fds);

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
				if (i == peer_listening_socket) {
					int new_sock;

					new_sock = peer_accept_new_client(i);
					if (new_sock != -1) {
						FD_SET(new_sock, &org_target_fds);
					}
				} else {
					int read_size;
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

			if (i == peer_listening_socket) continue;
			/*
			if (now_time-10 > client_info_peer[i].last_access) {
				printf("No Access over 100sec from %s (%s) port %d  descriptor %d. Stop connection.\n",
					client_info_peer[i].hostname,
					client_info_peer[i].ipaddr,
					client_info_peer[i].port,
					i);
				close(i);

				FD_CLR(i, &org_target_fds);
			}
			*/
		}
	}
	close(peer_listening_socket);
	return 0;
}

