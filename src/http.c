#include <errno.h>
#include "../include/http.h"

static const char *get_file_type(const char *type);
static void parse_uri(char *uri, int len, char *fn, char query);
static void process_err(int fd, char *cause, char *err_num, char *short_msg, char *long_msg);

static char *ROOT = NULL;

mime_type_t mime[] =
    {
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/msword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css"},
        {NULL, "text/plain"}};

//  uri
//  [protocol]://[usrname]:[passwd]@[server_address]:[port]/[source_path]?[seach_str]#[fragment_ID]
static void parse_uri(char *uri, int len, char *fn, char query)
{
    uri[len] = '\0';

    // 找到?界定非参部分
    char *delimp = strchr(uri, '?');
    int fn_len = delimp != NULL ? ((int)(delimp - uri)) : len;
    // 根目录拷贝
    strcpy(fn, ROOT);
    // 追加uri 此处已经被解析为绝对路径文件名
    strncat(fn, uri, fn_len);
    // 界定请求文件
    char *last_comp = strrchr(fn, '/');
    char *last_dot = strrchr(fn, '.');
    if ((last_dot == NULL) && (fn[strlen(fn) - 1] != '/'))
        strcat(fn, '/');
    // 默认
    if (fn[strlen(fn) - 1] == '/')
        strcat(fn, "index.html");
}

const char *get_file_type(const char *type)
{
    // compare the _type with table
    for (int i = 0; mime[i].type != NULL; ++i)
    {
        if (strcmp(type, mime[i].type) == 0)
            return mime[i].value;
    }
    // not recognized
    return "text/plain";
}

static void process_err(int fd, char *cause, char *err_num, char *short_msg, char *long_msg)
{
    // response header buffer and dat buffer
    char header[MAXLINE];
    char body[MAXLINE];

    // with log_msg and cause to fill response body
    sprintf(body, "<html>\
                    <body>\
                    <h1>Error</h1>\
                    <p>%s : %s</p>\
                    <p>%s : %s</p>\
                    </body>\
                    </html>",
            err_num, short_msg, long_msg, cause);

    // return error code, manage error response header
    sprintf(header, "HTTP/1.1 %s %s\r\n", err_num, short_msg);
    sprintf(header, "%sServer: Ubuntu\r\n", header);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));

    // send
    writen(fd, header, strlen(header));
    write(fd, body, strlen(body));
}

void serve_static(int fd, char *fn, size_t len, http_out_t *out)
{
    // buf
    char header[MAXLINE];
    char buff[SHORTLINE];

    struct tm tm_stmp;

    // response line
    sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, stat_info(out->status));

    // response header
    // Connection Keep-Alive Content-type Content-length Last-Modified
    if (out->keep_alive)
    {
        // default long connect , timeout: 500ms
        sprintf(header, "%sConnection: keep-alive\r\n", header);
        sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT); // connection time
    }
    // other
    if (out->modified)
    {
        const char *type = get_file_type(strrchr(fn, '.'));
        sprintf(header, "%sContent-type: %s\r\n", header, type);
        sprintf(header, "%sContent-length: %zu\r\n", header, len);
        localtime_r(&(out->mtime), &tm_stmp);
        strftime(buff, SHORTLINE, "%a, %d %b %Y %H:%M:%S GMT", &tm_stmp);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buff);
    }
    sprintf(header, "%sServer: Ubuntu\r\n\r\n", header);

    size_t sendl = (size_t)writen(fd, header, strlen(header));
    if (sendl != strlen(header))
    {
        perror("Send header failed\n");
        return;
    }

    // send
    int srcfd = open(fn, O_RDONLY, 0);
    char *addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);

    sendl = (size_t)writen(fd, addr, len);
    if (sendl != len)
        perror("Send file failed\n");

    munmap(addr, len);
}

int has_err(struct stat *sbufptr, char *fn, int fd)
{
    // File not Found
    if (stat(fn, sbufptr) < 0)
    {
        process_err(fd, fn, "404", "Not Found", "Server can't find the file");
        return 1;
    }

    // beyond access
    if (!(S_ISREG((*sbufptr).st_mode)) || !(S_IRUSR & (*sbufptr).st_mode))
    {
        process_err(fd, fn, "403", "Forbidden", "Permission Denied!");
        return 1;
    }

    return 0;
}

void do_request(void *ptr)
{
    http_request_t *request = (http_request_t *)ptr;
    int fd = request->fd;
    ROOT = request->root;
    char fn[SHORTLINE];
    struct stat sbuf;
    int rc, n_read;
    char *plast = NULL;
    size_t rs;

    del_timer(request);

    while (1)
    {
        plast = &request->buff[request->last % MAX_BUF];
        // rs: left bytes available can be written to buff
        rs = MIN(MAX_BUF - (request->last - request->pos) - 1, MAX_BUF - request->last % MAX_BUF);
        n_read = read(fd, plast, rs);
        if (n_read == 0)
            goto err;
        if (n_read < 0 && errno != AGAIN)
            goto err;

        // reset timer
        if (n_read < 0 && errno == AGAIN)
            break;

        request->last += n_read;
        rc = http_parse_request_line(request);
        if (rc == AGAIN)
            continue;
        else if (rc != 0)
            goto err;

        rc = http_parse_request_body(request);
        if (rc == AGAIN)
            continue;
        else if (rc != 0)
            goto err;

        http_out_t *out = (http_out_t *)malloc(sizeof(http_out_t));
        init_out_t(out, fd);

        parse_uri(request->uri_start, request->uri_end - request->uri_start, fn, NULL);

        if (has_err(&sbuf, fn, fd))
            continue;

        http_handle_header(request, out);
        out->mtime = sbuf.st_mtime;

        serve_static(fd, fn, sbuf.st_size, out);

        int kconn = out->keep_alive;
        free(out);

        if (kconn)
            break;
        else
            goto close;
    }
    // reset the epoll about the fd
    _epoll_mod(request->epoll_fd, request->fd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));
    TIMEOUT_INCREASE(request);
    return;
err:
close:
    http_close_conn(request);
}
