#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "./http.h"
#include "./threadpool.h"

#define MAXEVENTS 1024

extern struct epoll_event *events;

int _epoll_create(int flags);
int _epoll_add(int epoll_fd, int fd, http_request_t *request, int events);
int _epoll_mod(int epoll_fd, int fd, http_request_t *request, int events);
int _epoll_del(int epoll_fd, int fd, http_request_t *request);
int _epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);
void _handle_events(int epoll_fd, int listen_fd, struct epoll_event *events,
                    int events_num, char *path, threadpool_t *tp);

#endif