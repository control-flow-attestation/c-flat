/*
 * Singly-Linked List API
 */
#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stdbool.h>

typedef struct elem elem_t;
typedef struct list list_t;
typedef struct iter iter_t;

typedef struct elem {
	elem_t *next;
	void   *data;
} elem_t;

typedef struct list {
	elem_t *head;
	elem_t *last;
} list_t;

typedef struct iter {
	elem_t *curr;
} iter_t;

void list_init(list_t *list);
void list_finalize(list_t *list);

void *list_head(const list_t *list);
void *list_last(const list_t *list);
bool list_is_empty(const list_t *list);

void list_append(list_t *list, void* data);
void *list_remove(list_t *list);

void list_push(list_t *list, void* data);
void *list_pop(list_t *list);

iter_t list_iter(list_t *list);
void *list_next(iter_t *iter);

#endif /* UTIL_LIST_H */

