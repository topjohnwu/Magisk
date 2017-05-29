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
void list_pop(struct list_head *pos);
void list_pop_end(struct list_head *head);

#define list_entry(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_r(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#endif