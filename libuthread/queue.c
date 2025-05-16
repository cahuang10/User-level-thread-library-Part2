#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct Node
{
	void *data;
	struct Node *_next;
} Node;

struct queue
{
	/* TODO Phase 1 */
	Node *head;
	Node *tail; // needs to be pointers because head and tail changes as linked list enlarges or shrinks.
	int size;
};

queue_t queue_create(void)
{
	/* TODO Phase 1 */
	struct queue *q = (struct queue *)malloc(sizeof(struct queue));
	if (q == NULL)
	{
		return NULL;
	}
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

int queue_destroy(queue_t queue)
{
	/* TODO Phase 1 */
	if (queue == NULL || queue->size > 0)
	{
		return -1;
	}
	free(queue); // deallocating the queue.
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
		return -1;

	Node *new = (Node *)malloc(sizeof(Node));
	if (new == NULL)
		return -1;

	new->data = data;
	new->_next = NULL;

	if (queue->size == 0)
	{ // queue is empty
		queue->head = queue->tail = new;
	}
	else
	{ // queue has items
		queue->tail->_next = new;
		queue->tail = new;
	}

	queue->size++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data) // double pointer data works as a return value. return by reference
{
	/* TODO Phase 1 */
	if (queue == NULL || data == NULL || queue->size == 0)
		return -1;

	Node *temp = queue->head;
	*data = temp->data;

	queue->head = temp->_next;
	if (queue->head == NULL) // queue is now empty
		queue->tail = NULL;

	free(temp);
	queue->size--;

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	/* TODO Phase 1 */
	if (queue == NULL || queue->size == 0 || data == NULL)
	{
		return -1;
	}

	Node *prev = NULL;
	Node *current = queue->head;
	while (current != NULL)
	{
		if (current->data == data)
		{
			if (prev == NULL)
			{ // it is the head
				if (queue->size == 1)
				{
					queue->head = queue->tail = NULL;
				}
				else
				{
					queue->head = queue->head->_next;
				}
			}
			else if (current == queue->tail)
			{
				queue->tail = prev;
				prev->_next = NULL;
			}
			else
			{ // item is in the middle
				prev->_next = current->_next;
			}
			free(current);
			current = NULL;
			queue->size--;
			return 0;
		}
		prev = current;
		current = current->_next;
	}
	return -1; // data was not found.
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	/* TODO Phase 1 */

	if (queue == NULL || func == NULL)
	{
		return -1;
	}

	Node *current = queue->head;
	while (current != NULL)
	{
		Node *next = current->_next; // saving variable in cases it gets deleted.
		func(queue, current->data);
		current = next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL)
	{
		return -1;
	}
	return queue->size;
}
