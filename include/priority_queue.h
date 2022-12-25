#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdlib.h>

#define PQ_DEFAULT_SIZE 10
#define MOVE_ELEMENT(CONTAINER, PI, PJ) (CONTAINER)[PI] = (CONTAINER)[PJ] // PI <- PJ
#define PQ_ISEMPTY(PQ) (PQ)->nalloc == 0
#define PQ_SIZE(PQ) (PQ)->nalloc

typedef int (*comparator)(void *pi, void *pj);

typedef struct priority_queue
{
    void **container;
    size_t nalloc; // element count
    size_t size;   // container size
    comparator comp;
} pq_t;

int pq_init(pq_t *pq, comparator comp, size_t size);
void *pq_min(pq_t *pq);
int pq_delmin(pq_t *pq);
int pq_insert(pq_t *pq, void *item);
int pq_sink(pq_t *pq, size_t i);

#endif