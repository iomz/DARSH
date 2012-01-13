/* config */
#define CONF_DEVIDE "="
#define CONF_DEVIDE_LEN strlen(CONF_DEVIDE)
#define CONF_INTERVAL "CONF_INTERVAL"
#define CONF_INTERFACE "CONF_INTERFACE"
#define CONF_PEERHOST "CONF_PEERHOST"
#define CONF_HOSTID "CONF_HOSTID"

/* protocol for networking */
#define devide_letter ", "


int accept_new_client(int sock);
int read_and_save_table(int sock);
int read_and_reply(int sock);
void write_to_db_file(char*content);
int read_line(int socket, char *p);
//har *get_hostname(void);
//char *get_ip(void);

