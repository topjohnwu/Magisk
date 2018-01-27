/* list.h - Double link list implementation
 */

#ifndef _LIST_H_
#define _LIST_H_

#include <stddef.h>

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

void init_list_head(struct list_head *head);
void list_insert(struct list_head *pos, struct list_head *node);
void list_insert_end(struct list_head *head, struct list_head *node);
struct list_head *list_pop(struct list_head *pos);
struct list_head *list_pop_end(struct list_head *head);

#define list_entry(pos, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (pos); \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define list_for_each(ptr, head, type, member) \
	ptr = list_entry((head)->next, type, member); \
	for (struct list_head *__ = (head)->next; __ != (head); __ = __->next, ptr = list_entry(__, type, member))

#define list_for_each_r(ptr, head, type, member) \
	ptr = list_entry((head)->prev, type, member); \
	for (struct list_head *__ = (head)->prev; __ != (head); __ = __->prev, ptr = list_entry(__, type, member))

#define list_destory(head, type, member, func) ({ \
	struct list_head *node = head->next; \
	while(node != head) { \
		node = node->next; \
		if (func) func(list_entry(node->prev, line_list, pos)); \
		free(list_entry(node->prev, line_list, pos)); \
	} \
	head->next = head; \
	head->prev = head; \
})

#endif
