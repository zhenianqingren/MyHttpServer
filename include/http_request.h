#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "./util.h"
#include "./list.h"
#include <pthread.h>

#define AGAIN EAGAIN

#define HTTP_PARSE_INVALID_METHOD 10
#define HTTP_PARSE_INVALID_REQUEST 11
#define HTTP_PARSE_INVALID_HEADER 12

#define HTTP_UNKNOWN 0x0001
#define HTTP_GET 0x0002
#define HTTP_HEAD 0x0004
#define HTTP_POST 0x0008

#define HTTP_OK 200
#define HTTP_NOT_MODIFIED 304
#define HTTP_NOT_FOUND 404
#define MAX_BUF 8124

typedef struct http_request
{
    char *root;
    int fd;
    int epoll_fd;
    char buff[MAX_BUF];
    size_t pos;
    size_t last;
    int state;

    pthread_mutex_t lock;

    void *request_start;
    void *method_end;
    int method;
    void *uri_start;
    void *uri_end;
    void *path_start;
    void *path_end;
    void *query_start;
    void *query_end;
    int http_major;
    int http_minor;
    void *request_end;

    struct list_head list; // 存储请求�?

    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;
    void *timer;
} http_request_t;

typedef struct http_out
{
    int fd;
    int keep_alive; // 长连接告�?
    time_t mtime;   // 服务器时�?
    int modified;   // 告知变化 减少重复发�?
    int status;     // 状态码
} http_out_t;

// 请求头键值对 只需要对部分进行处理
typedef struct http_header
{
    void *key_start;
    void *key_end;
    void *value_start;
    void *value_end;
    struct list_head list;
} http_header_t;

typedef int (*header_handler_pt)(http_request_t *request, http_out_t *out, char *data, int len);

typedef struct http_header_handle // 处理请求�?
{
    char *name;
    header_handler_pt handler; // 函数指针
} http_header_handle_t;

extern http_header_handle_t headers[];

void http_handle_header(http_request_t *request, http_out_t *out);
int http_close_conn(http_request_t *request);
int init_request_t(http_request_t *request, int fd, int epfd, char *path);
int init_out_t(http_out_t *out, int fd);
const char *stat_info(int stat_code);
#endif