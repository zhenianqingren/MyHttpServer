#ifndef LIST_H
#define LIST_H

typedef struct list_head
{
    struct list_head *prev;
    struct list_head *next;
} list_head;

// init list
#define INIT_LIST_HEAD(ptr)                               \
    do                                                    \
    {                                                     \
        struct list_head *_ptr = (struct list_head *)ptr; \
        (_ptr)->next = (_ptr);                            \
        (_ptr->prev) = (_ptr);                            \
    } while (0)

#define OFFSETOF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER) // 结构体成员的绝对偏移地址

#define CONTAINER_OF(ptr, type, member) ({             \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - OFFSETOF(type, member)); \
}) // 结构体地址

#define LIST_ENTRY(ptr, type, member) \
    CONTAINER_OF(ptr, type, member)

#define LIST_FOR_EACH(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define LIST_FOR_EACH_PREV(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

// insert new node
static inline void __list_add(list_head *_new, list_head *prev, list_head *next)
{
    _new->prev = prev;
    _new->next = next;
    prev->next = _new;
    next->prev = _new;
}

// insert head
static inline void list_add_head(list_head *_new, list_head *head)
{
    __list_add(_new, head, head->next);
}

// insert tail
static inline void list_add_tail(list_head *_new, list_head *head)
{
    __list_add(_new, head->prev, head);
}

// del node
static inline void __list_del(list_head *prev, list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

// del entry node
static inline void list_del(list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline int list_empty(list_head *head)
{
    return (head->next == head) && (head->prev == head);
}

#endif