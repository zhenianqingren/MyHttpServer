#include "../include/io.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static ssize_t io_read(io *rp, char *usrbuf, size_t n)
{
    size_t cnt;
    while (rp->io_cnt <= 0)
    {
        rp->io_cnt = read(rp->io_fd, rp->io_buf, sizeof(rp->io_buf));
        if (rp->io_cnt < 0)
        {
            if (errno == EAGAIN)
                return -EAGAIN;
            if (errno != EINTR)
                return -1;
        }
        else if (rp->io_cnt == 0)
            return 0;
        else
            rp->io_bufptr = rp->io_buf;
    }
    cnt = n;
    if (rp->io_cnt < (ssize_t)n)
        cnt = rp->io_cnt;
    memcpy(usrbuf, rp->io_buf, cnt);
    rp->io_bufptr += cnt;
    rp->io_cnt -= cnt;
    return cnt;
}

ssize_t readn(int fd, void *usrbuf, size_t n)
{
    size_t r_left = n;
    size_t n_read;
    char *bufp = (char *)usrbuf;
    while (r_left > 0)
    {
        if ((n_read = read(fd, bufp, r_left)) < 0)
        {
            if (errno == EINTR)
                n_read = 0;
            else
                return -1;
        }
        else if (n_read == 0)
            break;
        r_left -= n_read;
        bufp += n_read;
    }
    return n - r_left;
}

ssize_t writen(int fd, void *usrbuf, size_t n)
{
    size_t w_left = n;
    size_t n_write;
    char *bufp = (char *)usrbuf;
    while (w_left > 0)
    {
        if ((n_write = write(fd, bufp, w_left)) < 0)
        {
            if (errno == EINTR)
                n_write = 0;
            else
                return -1;
        }
        w_left -= n_write;
        bufp += n_write;
    }
    return n - w_left;
}

void readinitb(io *rp, int fd)
{
    rp->io_fd = fd;
    rp->io_cnt = 0;
    rp->io_bufptr = rp->io_buf;
}
ssize_t readnb(io *rp, void *usrbuf, size_t n)
{
    size_t n_read;
    if ((n_read = readn(rp->io_fd, usrbuf, n)) < 0)
        return -1;
    else
        return n - n_read;
}

ssize_t readlineb(io *rp, void *usrbuf, size_t maxlen)
{
    size_t n;
    ssize_t rc;
    char c;
    char *bufp = (char *)usrbuf;
    for (n = 1; n < maxlen; ++n)
    {
        if ((rc = io_read(rp, &c, 1)) == 1)
        {
            *bufp++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
            if (n == 1)
                return 0;
            else
                break;

        else if (rc == -EAGAIN)
            return rc;
        else
            return -1;
    }
    *bufp = 0;
    return n;
}