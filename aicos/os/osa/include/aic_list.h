/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aic list-related structure and api.
 * It can support on armcc and gcc complier.
 */
#ifndef __AIC_LIST_H__
#define __AIC_LIST_H__

#include <aic_type.h>

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(object) { &(object), &(object) }

#define LIST_HEAD(object) struct list_head object = LIST_HEAD_INIT(object)

static inline void list_head_init(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * list_add - add a new node after a list
 * @new: new node to be added
 * @head: list head to add it after
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    head->next->prev = new;
    new->next = head->next;
    new->prev = head;
    head->next = new;
}

/**
 * list_add_tail - add a new node before a list
 * @new: new node to be added
 * @head: list head to add it before
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    head->prev->next = new;
    new->next = head;
    new->prev = head->prev;
    head->prev = new;
}

/**
 * list_del - deletes node from list
 * @node: the node to delete from the list
 */
static inline void list_del(struct list_head *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

/**
 * list_del_init - deletes node from list and init it
 * @node: the node to delete from the list
 */
static inline void list_del_init(struct list_head *node)
{
    list_del(node);
    list_head_init(node);
}

/**
 * list_replace - replace old node by new one
 * @o: the node to be replaced
 * @n: the new node to insert
 */
static inline void list_replace(struct list_head *o, struct list_head *n)
{
    n->next = o->next;
    n->next->prev = n;
    n->prev = o->prev;
    n->prev->next = n;
}

/**
 * list_move - delete from one node and add as another's head
 * @node: the node to move
 * @head: list head to add it after
 */
static inline void list_move(struct list_head *node, struct list_head *head)
{
    list_del(node);
    list_add(node, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @node: the node to move
 * @head: list head to add it before
 */
static inline void list_move_tail(struct list_head *node,
                                  struct list_head *head)
{
    list_del(node);
    list_add_tail(node, head);
}

/**
 * list_is_last - tests whether @node is the last node in list @head
 * @node: the list to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct list_head *node,
                               const struct list_head *head)
{
    return node->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @node: the list to test
 */
static inline int list_empty(const struct list_head *node)
{
    return node->next == node;
}

/**
 * list_entry - get the struct for this node
 * @node:   the list pointer
 * @type:   the type of the structure
 * @member: the name of list in structure
 */
#define list_entry(node, type, member) \
    container_of(node, type, member)

/**
 * list_first_entry - get the first element from a list
 * @node:   the list pointer
 * @type:   the type of the structure
 * @member: the name of list in structure
 */
#define list_first_entry(node, type, member) \
    list_entry((node)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @node:   the list pointer
 * @type:   the type of the structure
 * @member: the name of list in structure
 */
#define list_last_entry(node, type, member) \
    list_entry((node)->prev, type, member)

/**
 * list_next_entry - get the next element in list
 * @node:   the type * to cursor
 * @member: the name of list in structure
 */
#define list_next_entry(node, member, type) \
    list_entry((node)->member.next, type, member)

/**
 * list_prev_entry - get the prev element in list
 * @node:   the type * to cursor
 * @member: the name of list in structure
 */
#define list_prev_entry(node, member, type) \
    list_entry((node)->member.prev, type, member)

/**
 * list_for_each - iterate over a list
 * @node: the list_head * to use as a loop cursor
 * @head: the head of the list
 */
#define list_for_each(node, head) \
    for (node = (head)->next; node != (head); node = node->next)

/**
 * list_for_each_prev - iterate over a list backwards
 * @node: the list_head * to use as a loop cursor
 * @head: the head of the list
 */
#define list_for_each_prev(node, head) \
    for (node = (head)->prev; node != (head); node = node->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @node: the list_head * to use as a loop cursor
 * @n:    another list_head * to use as temporary storage
 * @head: the head of the list
 */
#define list_for_each_safe(node, n, head) \
    for (node = (head)->next, n = node->next; node != (head); \
        node = n, n = node->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against
 * removal of list entry
 * @node: the list_head * to use as a loop cursor
 * @n:    another list_head * to use as temporary storage
 * @head: the head of the list
 */
#define list_for_each_prev_safe(node, n, head) \
    for (node = (head)->prev, n = node->prev; \
         node != (head); \
         node = n, n = node->prev)

/**
 * list_for_each_entry - iterate over list of given type
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry(node, head, member, type) \
    for (node = list_first_entry(head, type, member); \
         &node->member != (head); \
         node = list_next_entry(node, member, type))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_reverse(node, head, member, type) \
    for (node = list_last_entry(head, type, member); \
         &node->member != (head); \
         node = list_prev_entry(node, member, type))

/**
 * list_prepare_entry - prepare a node entry for use in
 * list_for_each_entry_continue()
 * @node:   the type * to use as a start point
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_prepare_entry(node, head, member, type) \
    ((node) ? : list_entry(head, type, member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_continue(node, head, member, type) \
    for (node = list_next_entry(node, member, type); \
         &node->member != (head); \
         node = list_next_entry(node, member, type))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_continue_reverse(node, head, member, type) \
    for (node = list_prev_entry(node, member, type); \
         &node->member != (head); \
         node = list_prev_entry(node, member, type))

/**
 * list_for_each_entry_from - iterate over list of given type from the
 * current point
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_from(node, head, member, type) \
    for (; &node->member != (head); \
         node = list_next_entry(node, member, type))

/**
 * list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @node:   the type * to use as a loop cursor
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_from_reverse(node, head, member, type) \
    for (; &node->member != (head); \
         node = list_prev_entry(node, member, type))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @node:   the type * to use as a loop cursor
 * @n:      another type * to use as temporary storage
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_safe(node, n, head, member, type) \
    for (node = list_first_entry(head, type, member), \
        n = list_next_entry(node, member, type); \
         &node->member != (head); \
         node = n, n = list_next_entry(n, member, type))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against
 * removal
 * @node:   the type * to use as a loop cursor
 * @n:      another type * to use as temporary storage
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_safe_continue(node, n, head, member, type) \
    for (node = list_next_entry(node, member, type), \
        n = list_next_entry(node, member, type); \
         &node->member != (head); \
         node = n, n = list_next_entry(n, member, type))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe
 * against removal
 * @node:   the type * to use as a loop cursor
 * @n:      another type * to use as temporary storage
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_safe_from(node, n, head, member, type) \
    for (n = list_next_entry(node, member, type); \
         &node->member != (head); \
         node = n, n = list_next_entry(n, member, type))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against
 * removal
 * @node:   the type * to use as a loop cursor
 * @n:      another type * to use as temporary storage
 * @head:   the head of the list
 * @member: the name of list in structure
 */
#define list_for_each_entry_safe_reverse(node, n, head, member, type) \
    for (node = list_last_entry(head, type, member), \
        n = list_prev_entry(node, member, type); \
         &node->member != (head); \
         node = n, n = list_prev_entry(n, member, type))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @node:   the loop cursor used in the list_for_each_entry_safe loop
 * @n:      temporary storage used in list_for_each_entry_safe
 * @member: the name of list in structure
 */
#define list_safe_reset_next(node, n, member, type) \
    n = list_next_entry(node, member, type)

#endif /* __AIC_LIST_H__ */

