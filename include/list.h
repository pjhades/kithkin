#ifndef __LIST_H__
#define __LIST_H__

#ifndef offsetof
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, field)                     \
    ({                                                     \
         const typeof(((type *)0)->field) *__tptr = (ptr); \
         (type *)((char *)__tptr - offsetof(type, field)); \
     })
#endif

struct list_node {
    struct list_node *next;
    struct list_node *prev;
};

#define list_node_initializer(head) { &(head), &(head) }

#define list_init(head)        \
    do {                       \
        (head)->prev = (head); \
        (head)->next = (head); \
    } while (0)

#define list_isempty(head) ((head)->next == (head))

#define list_foreach(var, head, field)                                 \
    for ((var) = container_of((head)->next, typeof(*var), field);      \
         &(var)->field != (head);                                      \
         (var) = container_of((var)->field.next, typeof(*var), field))

#define list_foreach_safe(var, tvar, head, field)                                \
    for ((var) = container_of((head)->next, typeof(*var), field);                \
         &(var)->field != (head) &&                                              \
             ((tvar) = container_of((var)->field.next, typeof(*var), field), 1); \
         (var) = (tvar))

#define list_foreach_back(var, head, field)                            \
    for ((var) = container_of((head)->prev, typeof(*var), field);      \
         &(var)->field != (head);                                      \
         (var) = container_of((var)->field.prev, typeof(*var), field))

#define list_foreach_back_safe(var, tvar, head, field)                           \
    for ((var) = container_of((head)->prev, typeof(*var), field);                \
         &(var)->field != (head) &&                                              \
             ((tvar) = container_of((var)->field.prev, typeof(*var), field), 1); \
         (var) = (tvar))

#define list_insert_before(node, listnode) \
    do {                                   \
        (node)->next = (listnode);         \
        (node)->prev = (listnode)->prev;   \
        (listnode)->prev->next = (node);   \
        (listnode)->prev = (node);         \
    } while (0)

#define list_insert_after(node, listnode) \
    do {                                  \
        (node)->next = (listnode)->next;  \
        (node)->prev = (listnode);        \
        (listnode)->next->prev = (node);  \
        (listnode)->next = (node);        \
    } while (0)

#define list_insert_head(node, head) LIST_INSERT_AFTER((node), (head))
#define list_insert_tail(node, head) LIST_INSERT_BEFORE((node), (head)) 

#define list_remove(node)                  \
    do {                                   \
        (node)->prev->next = (node)->next; \
        (node)->next->prev = (node)->prev; \
    } while (0)

#define list_remove_head(head)             \
    do {                                   \
        (head)->next->next->prev = (head); \
        (head)->next = (head)->next->next; \
    } while (0)

#define list_remove_tail(head)             \
    do {                                   \
        (head)->prev->prev->next = (head); \
        (head)->prev = (head)->prev->prev; \
    } while (0)

#endif
