#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "./timer.h"
#include "./util.h"
#include "./io.h"
#include "./epoll.h"
#include "./http_parse.h"
#include "./http_request.h"

#define MAXLINE 8192
#define SHORTLINE 512

// 用key-value表示mime_type_t
typedef struct mime_type
{
    const char *type;
    const char *value;
} mime_type_t;

void do_request(void *ptr);

#endif
