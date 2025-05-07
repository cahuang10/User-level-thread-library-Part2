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
	queue_t q = (struct queue *)malloc(sizeof(struct queue));
	if (q == NULL)
	{
		return NULL;
	}
	q->head = q->tail = q->head->_next = q->tail->_next = NULL;
	q->size = 0;
	return q;
}

int queue_destroy(queue_t queue)
{
	/* TODO Phase 1 */
	if (queue == NULL || queue->size != 0)
	{
		return -1;
	}
	free(queue); // deallocating the queue.
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	/* TODO Phase 1 */
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	Node *new = (Node *)malloc(sizeof(Node));
	if (new == NULL)
	{
		return -1;
	}
	new->data = data;
	new->_next = NULL;

	if (queue->size == 0)
	{ // 1st edge case

		queue->head = new;
		queue->tail = new;
		queue->head->_next = queue->tail;
	} // needs if statments.
	// bigger than 0.
	queue->tail->_next = new;
	queue->tail = new;
	queue->size++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data) // double pointer data works as a return value. return by reference
{
	/* TODO Phase 1 */
	if (queue == NULL || queue->size == 0 || data == NULL)
	{
		return -1;
	}

	*data = queue->head->data; // return by reference the value of data.

	if (queue->size == 1)
	{
		free(queue->head);
		queue->head = queue->tail = queue->head->_next = queue->tail->_next = NULL;
	}

	// point to the new head and delete the oldest value.
	Node *tem = queue->head;
	queue->head = queue->head->_next;
	tem->_next = NULL;
	free(tem);

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
					queue->head = queue->tail = queue->head->_next = queue->tail->_next = NULL; // extra for safety.
				}
				else
				{
					queue->head = queue->head->_next;
				}
			}
			else if (current == queue->tail)
			{
				queue->tail == prev;
			}
			else
			{ // item is in the middle
				prev->_next = current->_next;
			}
			free(current->data);
			free(current);
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
		func(queue, current->data);
		current = current->_next;
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
