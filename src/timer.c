#include <sys/time.h>
#include "../include/timer.h"

pq_t pq_timer;
size_t current_msec;

int timer_comp(void *ti, void *tj)
{
    timer_rt *tii = (timer_rt *)ti;
    timer_rt *tjj = (timer_rt *)tj;
    return tii->key < tjj->key;
}

void time_update()
{
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    current_msec = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

int timer_init()
{
    int rc = pq_init(&pq_timer, timer_comp, PQ_DEFAULT_SIZE);
    time_update();
    return 0;
}

int find_timer()
{
    int tmp;
    while (!(PQ_ISEMPTY(&pq_timer)))
    {
        time_update();
        timer_rt *node = (timer_rt *)pq_min(&pq_timer);
        if (node->deleted)
        {
            pq_delmin(&pq_timer);
            continue;
        }
        tmp = (int)(node->key - current_msec);
        tmp = time > 0; // greater than zero which means not timeout
        break;
    }
    return tmp;
}

void handle_expire()
{
    while (!(PQ_ISEMPTY(&pq_timer)))
    {
        time_update();

        timer_rt *node = (timer_rt *)pq_min(&pq_timer);

        if (node->deleted)
        {
            pq_delmin(&pq_timer);
            free(node);
            continue;
        }

        if (node->key > current_msec) // not timeout
            return;
        if (node->handler) // timeout
            node->handler(node->request);
        pq_delmin(&pq_timer);
        free(node);
    }
}

void add_timer(http_request_t *request, size_t timeout, timer_handler handler)
{
    time_update();
    timer_rt *node = (timer_rt *)malloc(sizeof(timer_rt));
    request->timer = node;
    node->key = current_msec + timeout;
    node->deleted = 0;
    node->handler = handler;
    node->request = request;
    pq_insert(&pq_timer, (void *)node);
}

void del_timer(http_request_t *request)
{
    time_update();
    timer_rt *node = request->timer;

    node->deleted = 1; // lazy delete
}
