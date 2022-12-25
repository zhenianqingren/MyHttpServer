#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/priority_queue.h"

int comp(void *pi, void *pj)
{
    int i = *((int *)pi);
    int j = *((int *)pj);
    return i < j;
}

int main(int argc, char *argv[])
{
    pq_t *pq = (pq_t *)malloc(sizeof(pq_t));
    pq_init(pq, comp, 8);

    int *tmp = NULL;

    for (int i = 128; i >= 1; --i)
    {
        tmp = (int *)malloc(sizeof(int));
        *tmp = i;
        pq_insert(pq, (void *)tmp);
    }

    for (int i = 0; i < 128; ++i)
    {
        tmp = (int *)pq_min(pq);
        printf("%d ", *tmp);
        pq_delmin(pq);
        free(tmp);
    }
    printf("\n");
    return 0;
}