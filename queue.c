#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

struct node {
	void *data;
	struct node *next;
};
typedef struct node* node_t;

node_t create_node(void *data) {
	node_t n = malloc(sizeof(struct node));
	n->data = data;
	n->next = NULL;
	return n;
}

struct queue {
	node_t head;
	node_t tail;
	unsigned count;
};

queue_t queue_create(void)
{
	// create queue
	queue_t q = malloc(sizeof(struct queue));
	if (q == NULL)
		return NULL;

	// add dummy node
	q->head = create_node(NULL);
	q->tail = q->head;
	q->count = 0;
	if (q->head == NULL) {
		free(q);
		return NULL;
	}

	return q;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->count > 0) {
		return -1;
	}
	free(queue->head);
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
		return -1;

	node_t n = create_node(data);
	if (n == NULL)
		return -1;
	queue->tail->next = n;
	queue->tail = queue->tail->next;
	queue->count++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || data == NULL || queue->count == 0)
		return -1;
	
	node_t n = queue->head->next;
	queue->head->next = n->next;
	*data = n->data;
	if (queue->tail == n)
		queue->tail = queue->head;
	queue->count--;

	free(n);
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
		return -1;
	
	node_t h = queue->head;
	while (h->next != NULL) {
		if (h->next->data == data) {
			break;
		}
		h = h->next;
	}
	if (h->next == NULL) {
		// not found
		return -1;
	}

	node_t tmp = h->next;
	h->next = tmp->next;
	if (queue->tail == tmp) {
		queue->tail = h;
	}
	queue->count--;

	free(tmp);
	return 0;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if (queue == NULL || func == NULL)
		return -1;

	node_t h = queue->head->next;
	while (h != NULL) {
		node_t tmp = h->next;
		func(queue, h->data);
		h = tmp;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL)
		return -1;
	return queue->count;
}

