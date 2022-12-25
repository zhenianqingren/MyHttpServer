#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H


#define CR '\r'
#define LF '\n'

#include "./http_request.h"

// http�����н���
int http_parse_request_line(http_request_t *request);
// http���������
int http_parse_request_body(http_request_t *request);

#endif