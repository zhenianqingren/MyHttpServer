#ifndef RIO_H
#define RIO_H

#include <sys/types.h>
#define RIO_BUFSIZE 8192

typedef struct
{
    int io_fd;                /* descriptor for this internal buf */
    ssize_t io_cnt;           /* unread bytes in internal buf */
    char *io_bufptr;          /* next unread byte in internal buf */
    char io_buf[RIO_BUFSIZE]; /* internal buffer */
} io;

ssize_t readn(int fd, void *usrbuf, size_t n);
ssize_t writen(int fd, void *usrbuf, size_t n);
void readinitb(io *rp, int fd);
ssize_t readnb(io *rp, void *usrbuf, size_t n);
ssize_t readlineb(io *rp, void *usrbuf, size_t maxlen);

#endif