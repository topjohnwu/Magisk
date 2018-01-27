/* list.h - Double link list implementation
 */

#include "list.h"

void init_list_head(struct list_head *head) {
	head->next = head;
	head->prev = head;
}

void list_insert(struct list_head *pos, struct list_head *node) {
	// First construct our new node
	node->next = pos->next;
	node->prev = pos;
	// Maintain the list
	pos->next->prev = node;
	pos->next = node;
}

void list_insert_end(struct list_head *head, struct list_head *node) {
	list_insert(head->prev, node);
}

struct list_head *list_pop(struct list_head *pos) {
	struct list_head *ret;
	ret = pos->prev;
	// Maintain the list
	pos->prev->next = pos->next;
	pos->next->prev = pos->prev;
	// Remove references
	pos->next = pos;
	pos->prev = pos;
	// Return the previous node in the list
	return ret;
}

struct list_head *list_pop_end(struct list_head *head) {
	return list_pop(head->prev);
}
