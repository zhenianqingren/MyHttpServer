#ifndef UTIL_H
#define UTIL_H

#define PATHLEN 128
#define LISTENQ 1024
#define BUFLEN 8192
#define DELIM "="

#define CONF_OK 0
#define CONF_ERROR -1

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct conf
{
    char root[PATHLEN];
    int port;
    int thread_num;
} conf_t;

int read_conf(const char *fn, conf_t *conf);
void handle_sigpipe();
int socket_bl(int port);
int set_non_blocking(int fd);
void accept_connection(int lifd, int epfd, char *path);

#endif