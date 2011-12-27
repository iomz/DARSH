
int accept_new_client(int sock);
int read_and_save_table(int sock);
int read_and_reply(int sock);
void write_to_db_file(char*content);
int read_line(int socket, char *p);
char *get_hostname(void);
char *get_ip(void);

