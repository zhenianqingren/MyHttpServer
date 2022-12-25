#ifndef TIMER_H
#define TIMER_H

#include "./http_request.h"
#include "./priority_queue.h"
#include <pthread.h>

#define TIMEOUT_DEFAULT 500 /* ms */
#define PQ_DEFAULT_SIZE 16

#define TIMEOUT_INCREASE(request)         \
    do                                    \
    {                                     \
        timer_rt *_node = request->timer; \
        (_node)->key += TIMEOUT_DEFAULT;  \
    } while (0)

typedef int (*timer_handler)(http_request_t *request);

typedef struct timer_r
{
    size_t key;              // 标记超时时间
    int deleted;             // 标记是否被删�? lazy delete
    timer_handler handler;   // 超时处理函数
    http_request_t *request; // 指向对应的request请求
} timer_rt;

extern pq_t pq_timer;
extern size_t current_msec;

int timer_init();
int find_timer();
void handle_expire();
void add_timer(http_request_t *request, size_t timeout, timer_handler handler);
void del_timer(http_request_t *request);
int timer_comp(void *ti, void *tj);

#endif