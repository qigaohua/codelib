#ifndef __LIST_H__
#define __LISR_H__



#define _USE_FUNCTION_  0

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

typedef struct list_head  list_head_t;

#if _USE_FUNCTION
static inline void LIST_HEAD_INIT(struct list_head *head)
{
	head->prev = head;
	head->next = head;
}

static inline int list_empty(struct list_head *head)
{
	return head == head->prev;
}

static inline void list_insert_head(struct list_head *head, struct list_head *i)
{
	i->next = head->next;
	i->next->prev = i;
	i->prev = head;
	head->next = i;
}

static inline void list_insert_tail(struct list_head *head, struct list_head *i)
{
	i->prev = head->prev;
	i->prev->next = i;
	i->next = head;
	head->prev = i;
}
#else
#define LIST_HEAD_INIT(h)   \
	(h)->prev = (h);        \
	(h)->next = (h)

#define list_empty(h)       \
	((h) == (h)->prev)

#define list_insert_head(h, x) \
	(x)->next = (h)->next;     \
	(x)->next->prev = (x);     \
	(h)->next = (x);           \
	(x)->prev = (h)

#define list_insert_after  list_insert_head

#define list_insert_tail(h, x)  \
	(x)->prev = (h)->prev;      \
	(x)->prev->next = (x);       \
	(x)->next = (h);               \
	(h)->prev = (x)


#define list_head(h)       \
	(h)->next

#define list_last(h)      \
	(h)->prev

#define list_next(q)      \
	(q)->next

#define list_prev(q)        \
	(q)->prev

#define list_remove(r)     \
	r->prev->next = r->next; \
	r->next->prev = r->prev 

#define list_split(h, q, n)                                              \
   (n)->prev = (h)->prev;                                                    \
   (n)->prev->next = n;                                                      \
   (n)->next = q;                                                            \
   (h)->prev = (q)->prev;                                                    \
   (h)->prev->next = h;                                                      \
   (q)->prev = n;


 #define list_add(h, n)                                                   \
	(h)->prev->next = (n)->next;                                              \
	(n)->next->prev = (h)->prev;                                              \
	(h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;



#define list_data(q, type, link)                                         \
	(type *) ((char *) q - offsetof(type, link))


#define list_entry(ptr, type, member) \
	list_data(ptr, type, member)

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#endif

static inline void list_sort(list_head_t *queue,
    int (*cmp)(const list_head_t *, const list_head_t *))
{
    list_head_t  *q, *prev, *next;

    q = list_head(queue);

    if (q == list_last(queue)) {
        return;
    }

    for (q = list_next(q); q != queue; q = next) {

        prev = list_prev(q);
        next = list_next(q);

        list_remove(q);

        do {
            if (cmp(prev, q) <= 0) {
                break;
            }

            prev = list_prev(prev);

        } while (prev != queue);

        list_insert_after(prev, q);
    }
}


#endif
