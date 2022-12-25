#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

typedef struct task
{
    void (*func)(void *);
    void *arg;
    struct task *next; // 任务链表下一节点指针
} task_t;

typedef struct threadpool
{
    pthread_mutex_t lock; // 互斥
    pthread_cond_t cond;  // 条件变量
    pthread_t *threads;   // 线程
    task_t *head;         // 任务链表
    int thread_count;     // 线程
    int queue_size;       // 任务链表数量
    int shutdown;         // 关机模式
    int started;
} threadpool_t;

typedef enum
{
    invalid = -1,
    lock_fail = -2,
    already_shutdown = -3,
    cond_broadcast = -4,
    thread_fail = -5,
} threadpool_error_t;

typedef enum
{
    immediate_shutdown = 1,
    graceful_shutdown = 2
} threadpool_sd_t;

threadpool_t *threadpool_init(int thread_num);
int threadpool_add(threadpool_t *pool, void (*func)(void *), void *arg);
int threadpool_destroy(threadpool_t *pool, int gracegul);

#endif