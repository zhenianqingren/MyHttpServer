#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "../include/http_request.h"

static int http_process_ignore(http_request_t *request, http_out_t *out, char *data, int len)
{
    /*ignore*/
    /*not any process temporarily*/
    return 0;
}

static int http_process_connection(http_request_t *request, http_out_t *out, char *data, int len)
{
    if (strncasecmp("keep-alive", data, len) == 0)
        out->keep_alive = 1;
    return 0;
}

static int http_process_if_modified_since(http_request_t *request, http_out_t *out, char *data, int len)
{
    struct tm tm;
    if (strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char *)NULL)
        return 0;

    time_t clientt = mktime(&tm);
    double tdiff = difftime(out->mtime, clientt);
    if (fabs(tdiff) < 1e-6)
    {
        out->modified = 0;
        out->status = HTTP_NOT_MODIFIED;
    }
    return 0;
}

http_header_handle_t headers[] = {
    {"Host", http_process_ignore},
    {"Connection", http_process_connection},
    {"If-Modified-Since", http_process_if_modified_since},
    {"", http_process_ignore}};

void http_handle_header(http_request_t *request, http_out_t *out)
{
    list_head *pos;
    http_header_t *hd;
    http_header_handle_t *header_h;
    int len;
    LIST_FOR_EACH(pos, &(request->list))
    {
        hd = LIST_ENTRY(pos, http_header_t, list);
        for (header_h = headers; strlen(header_h->name) > 0; ++header_h)
        {
            if (strncmp(hd->key_start, header_h->name, hd->key_end - hd->key_start) == 0)
            {
                len = hd->value_end - hd->value_start;
                header_h->handler(request, out, hd->value_start, len);
                break;
            }
        }
        list_del(pos);
        free(hd);
    }
}

int http_close_conn(http_request_t *request)
{
    _epoll_del(request->epoll_fd, request->fd, request);
    close(request->fd);
    free(request);
    return 0;
}

int init_request_t(http_request_t *request, int fd, int epfd, char *path)
{
    request->fd = fd;
    request->epoll_fd = epfd;
    request->pos = 0;
    request->last = 0;
    request->state = 0;
    request->root = path;
    pthread_mutex_init(&(request->lock), NULL);
    INIT_LIST_HEAD(&(request->list));
    return 0;
}

int init_out_t(http_out_t *out, int fd)
{
    out->fd = fd;
    out->keep_alive = 1;
    out->modified = 1;
    out->status = 200;
    return 0;
}

const char *stat_info(int stat_code)
{
    switch (stat_code)
    {
    case HTTP_OK:
        return "OK";
    case HTTP_NOT_MODIFIED:
        return "Not Modified";
    case HTTP_NOT_FOUND:
        return "Not Fount";
    default:
        return "Unknown";
    }
    return (char *)NULL;
}
