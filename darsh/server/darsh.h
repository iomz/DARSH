#define BUF_LEN 256
#define PROTO_DEVIDE ", "
#define PROTO_OFFSET strlen(PROTO_DEVIDE)

#define db_filename "client_table"

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

int darsh_server(void);
int darsh_peer(void);
