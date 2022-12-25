#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "../include/util.h"
#include "../include/http_request.h"
#include "../include/epoll.h"

int read_conf(const char *fn, conf_t *conf)
{
    FILE *fp = fopen(fn, "r");
    if (!fp)
        return CONF_ERROR;

    char buf[BUFLEN];
    memset(buf, 0, BUFLEN);

    char *cur = buf;
    char *delim = NULL;
    int i = 0;
    int pos = 0;
    int llen = 0;
    while (fgets(cur, BUFLEN - pos, fp))
    {
        delim = strstr(cur, DELIM);
        if (!delim)
            return CONF_ERROR;
        if (cur[strlen(cur) - 1] == '\n')
            cur[strlen(cur) - 1] = 0;
        if (strncmp("root", cur, 4) == 0)
        {
            delim = delim + 1;
            while (*delim != '#')
            {
                conf->root[i++] = *delim;
                ++delim;
            }
        }

        if (strncmp("port", cur, 4) == 0)
            conf->port = atoi(delim + 1);
        if (strncmp("thread_num", cur, 9) == 0)
            conf->thread_num = atoi(delim + 1);
        llen = strlen(cur);
        cur += llen;
        pos += llen;
    }
    fclose(fp);
    return CONF_OK;
}

void handle_sigpipe()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
}

int socket_bl(int port) // bind and listen
{
    port = ((port <= 1024) || (port >= 65535)) ? 6666 : port;
    int lifd;
    if ((lifd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    int optval = 1;
    if (setsockopt(lifd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) == -1)
        return -1;

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(lifd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        return -1;

    if (listen(lifd, LISTENQ) == -1)
        return -1;
    return lifd;
}

int set_non_blocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        return -1;
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

void accept_connection(int lifd, int epfd, char *path)
{
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    socklen_t addrl = 0;
    int acfd = accept(lifd, (struct sockaddr *)&client, &addrl);
    if (acfd == -1)
        perror("accept\n");
    if (set_non_blocking(acfd) == -1)
        goto err;

    http_request_t *request = (http_request_t *)malloc(sizeof(http_request_t));
    init_request_t(request, acfd, epfd, path);

    _epoll_add(epfd, acfd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));
    add_timer(request, TIMEOUT_DEFAULT, http_close_conn);
    return;
err:
    perror("non blocking\n");
    close(acfd);
}