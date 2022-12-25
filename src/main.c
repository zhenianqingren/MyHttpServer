#include <stdio.h>
#include "../include/threadpool.h"
#include "../include/http.h"

const char *default_f = "server.conf";

extern struct epoll_event *events;
conf_t conf;

int main(int argc, char *argv[])
{

    read_conf(default_f, &conf);
    handle_sigpipe();
    int lifd = socket_bl(conf.port);
    int rc = set_non_blocking(lifd);

    int epfd = _epoll_create(0);
    http_request_t *request = (http_request_t *)malloc(sizeof(http_request_t));
    init_request_t(request, lifd, epfd, conf.root);
    _epoll_add(epfd, lifd, request, (EPOLLIN | EPOLLET));

    threadpool_t *tp = threadpool_init(conf.thread_num);

    timer_init();

    while (1)
    {
        int evn = _epoll_wait(epfd, events, MAXEVENTS, -1);

        handle_expire(); // timeout events

        _handle_events(epfd, lifd, events, evn, conf.root, tp);
    }

    return 0;
}