#include "../include/http_parse.h"

#define SHORT_STR_CMP(s, c0, c1, c2, c3) \
    *(u_int32_t *)s == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

int http_parse_request_line(http_request_t *request)
{
    // GET /index.html HTTP/1.1\r\n
    enum
    {
        start = 0,
        method,
        spaces_uri,
        after_slash_in_uri,
        http,
        http_HTTP,
        first_major_digit,
        major_digit,
        first_minor_digit,
        minor_digit,
        almost_done
    } state;

    state = request->state;
    u_char ch, *p, *m;
    size_t pi;
    for (pi = request->pos; pi < request->last; ++pi)
    {
        p = (u_char *)&request->buff[pi % MAX_BUF];
        ch = *p;
        switch (state)
        {
        case start:
            request->request_start = p;
            if (ch == CR || ch == LF)
                break;
            if ((ch < 'A' || ch > 'Z') && ch != '_')
                return HTTP_PARSE_INVALID_METHOD;
            state = method;
            break;

        case method:
            if (ch == ' ')
            {
                request->method_end = p;
                m = request->request_start;
                request->method = HTTP_UNKNOWN;
                if (SHORT_STR_CMP(m, 'G', 'E', 'T', ' '))
                    request->method = HTTP_GET;
                if (SHORT_STR_CMP(m, 'P', 'O', 'S', 'T'))
                    request->method = HTTP_POST;
                if (SHORT_STR_CMP(m, 'H', 'E', 'A', 'D'))
                    request->method = HTTP_HEAD;
                state = spaces_uri;
                break;
            }
            if ((ch < 'A' || ch > 'Z') && ch != '_')
                return HTTP_PARSE_INVALID_METHOD;
            break;

        case spaces_uri:
            if (ch == '/')
            {
                request->uri_start = p + 1;
                state = after_slash_in_uri;
                break;
            }
            else
            {
                if (ch == ' ')
                    break;
                else
                    return HTTP_PARSE_INVALID_REQUEST;
            }

        case after_slash_in_uri:
            switch (ch)
            {
            case ' ':
                request->uri_end = p;
                state = http;
                break;
            default:
                break;
            }
            break;

        case http:
            if (SHORT_STR_CMP("HTTP", *p, *(p + 1), *(p + 2), *(p + 3)))
                state = http_HTTP;
            else
                return HTTP_PARSE_INVALID_REQUEST;
            pi = pi + 3;
            break;

        case http_HTTP:
            switch (ch)
            {
            case '/':
                state = first_major_digit;
                break;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case first_major_digit:
            if (ch < '1' || ch > '9')
                return HTTP_PARSE_INVALID_REQUEST;
            request->http_major = ch - '0';
            state = major_digit;
            break;

        case major_digit:
            if (ch == '.')
            {
                state = first_minor_digit;
                break;
            }
            if (ch < '0' || ch > '9')
                return HTTP_PARSE_INVALID_REQUEST;
            request->http_major = request->http_major * 10 + ch - '0';
            break;

        case first_minor_digit:
            if (ch < '0' || ch > '9')
                return HTTP_PARSE_INVALID_REQUEST;
            request->http_minor = ch - '0';
            state = minor_digit;
            break;

        case minor_digit:
            if (ch == CR)
            {
                state = almost_done;
                break;
            }
            if (ch == LF)
                goto done;
            if (ch > '0' && ch < '9')
                request->http_minor = request->http_minor * 10 + ch - '0';
            return HTTP_PARSE_INVALID_REQUEST;

        case almost_done:
            request->request_end = p - 1;
            switch (ch)
            {
            case LF:
                goto done;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;
        }
    }
    request->pos = pi;
    request->state = state;
    return AGAIN;
done:
    request->pos = pi + 1;
    if (request->request_end == NULL)
        request->request_end = p;
    request->state = start;
    return 0;
}

int http_parse_request_body(http_request_t *request)
{
    enum
    {
        start = 0,
        key,
        spaces_before_colon,
        spaces_after_colon,
        value,
        cr,
        crlf,
        crlfcr
    } state;

    state = request->state;
    size_t pi;
    unsigned char ch;
    unsigned char *p;
    http_header_t *hd;
    for (pi = request->pos; pi < request->last; ++pi)
    {
        p = (u_char *)&request->buff[pi % MAX_BUF];
        ch = *p;

        switch (state)
        {
        case start:
            if (ch == CR || ch == LF)
                break;
            request->cur_header_key_start = p;
            state = key;
            break;

        case key:
            if (ch == ' ')
            {
                request->cur_header_key_end = p;
                state = spaces_before_colon;
            }
            if (ch == ':')
            {
                request->cur_header_key_end = p;
                state = spaces_after_colon;
            }
            break;

        case spaces_before_colon:
            if (ch == ' ')
                break;
            if (ch == ':')
            {
                state = spaces_after_colon;
                break;
            }
            return HTTP_PARSE_INVALID_HEADER;

        case spaces_after_colon:
            if (ch == ' ')
                break;
            state = value;
            request->cur_header_value_start = p;
            break;

        case value:
            if (ch == CR)
            {
                request->cur_header_value_end = p;
                state = cr;
            }
            if (ch == LF)
            {
                request->cur_header_value_end = p;
                state = crlf;
            }
            break;

        case cr:
            if (ch == LF)
            {
                state = crlf;
                hd = (http_header_t *)malloc(sizeof(http_header_t));
                hd->key_start = request->cur_header_key_start;
                hd->key_end = request->cur_header_key_end;
                hd->value_start = request->cur_header_value_start;
                hd->value_end = request->cur_header_value_end;
                list_add_head(&(hd->list), &(request->list));
                break;
            }
            return HTTP_PARSE_INVALID_REQUEST;

        case crlf:
            if (ch == CR)
                state = crlfcr;
            else
            {
                request->cur_header_key_start = p;
                state = key;
            }
            break;

        case crlfcr:
            switch (ch)
            {
            case LF:
                goto done;
            default:
                return HTTP_PARSE_INVALID_REQUEST;
            }
            break;
        }
    }

    request->pos = pi;
    request->state = state;
    return AGAIN;

done:
    request->pos = pi + 1;
    request->state = start;
    return 0;
}