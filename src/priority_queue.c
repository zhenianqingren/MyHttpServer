#include <string.h>
#include "../include/priority_queue.h"

int pq_init(pq_t *pq, comparator comp, size_t size)
{
    pq->container = (void **)malloc(sizeof(void *) * size);
    if (!pq->container)
        return -1;
    pq->nalloc = 0;
    pq->size = size;
    pq->comp = comp;
    return 0;
}

void *pq_min(pq_t *pq)
{
    if (PQ_ISEMPTY(pq))
        return (void *)(-1);

    return pq->container[0];
}

void resize(pq_t *pq, size_t _new_size)
{
    if (_new_size <= pq->nalloc)
        return;
    void **tmp = (void **)malloc(sizeof(void *) * _new_size);
    if (!tmp)
        return;
    memcpy(tmp, pq->container, (pq->nalloc) * sizeof(void *));
    free(pq->container);
    pq->container = tmp;
    pq->size = _new_size;
}

int pq_delmin(pq_t *pq)
{
    if (PQ_ISEMPTY(pq))
        return 0;

    MOVE_ELEMENT(pq->container, 0, pq->nalloc - 1);
    --pq->nalloc;
    pq_sink(pq, 0);
    if ((pq->nalloc > 0) && (pq->nalloc <= (pq->size - 1) / 4))
        resize(pq, pq->size / 2);
    return 0;
}

void up(pq_t *pq, size_t i)
{
    void *tmp = pq->container[i];
    while (i > 0)
    {
        size_t p = (i - 1) >> 1;
        if (!(pq->comp(tmp, pq->container[p])))
            break;
        MOVE_ELEMENT(pq->container, i, p);
        i = p;
    }
    pq->container[i] = tmp;
}

int pq_insert(pq_t *pq, void *item)
{
    if (pq->nalloc == pq->size)
        resize(pq, pq->size * 2);

    pq->container[pq->nalloc++] = item;
    up(pq, pq->nalloc - 1);
    return 0;
}

int pq_sink(pq_t *pq, size_t i)
{
    size_t j = i * 2 + 1;
    void *tmp = pq->container[i];
    while (j < pq->nalloc)
    {
        if ((j + 1 < pq->nalloc) && pq->comp(pq->container[j + 1], pq->container[j]))
            ++j;
        if (pq->comp(tmp, pq->container[j]))
            break;
        MOVE_ELEMENT(pq->container, i, j);
        i = j;
        j = i * 2 + 1;
    }
    pq->container[i] = tmp;
    return 0;
}