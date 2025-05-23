#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
	int count;
	queue_t blocked_queue;
};

sem_t sem_create(size_t count)
{
	struct semaphore *sem = malloc(sizeof(struct semaphore));
	if (!sem)
		return NULL;

	sem->count = count;
	sem->blocked_queue = queue_create();
	if (!sem->blocked_queue) {
		free(sem);
		return NULL;
	}

	return sem;
}

int sem_destroy(sem_t sem)
{
	if (!sem)
		return -1;

	/* Cannot destroy if threads are waiting */
	if (queue_length(sem->blocked_queue) > 0)
		return -1;

	queue_destroy(sem->blocked_queue);
	free(sem);
	return 0;
}

int sem_down(sem_t sem)
{
	if (!sem)
		return -1;

	/* Disable preemption during critical section */
	preempt_disable();

	/* If resources available, take one */
	if (sem->count > 0) {
		sem->count--;
		preempt_enable();
		return 0;
	}

	/* No resources available, block current thread */
	struct uthread_tcb *current = uthread_current();
	queue_enqueue(sem->blocked_queue, current);
	
	/* Re-enable preemption before blocking */
	preempt_enable();
	
	/* Block the thread */
	uthread_block();

	return 0;
}

int sem_up(sem_t sem)
{
	if (!sem)
		return -1;

	/* Disable preemption during critical section */
	preempt_disable();

	/* If threads are waiting, wake up the first one */
	if (queue_length(sem->blocked_queue) > 0) {
		struct uthread_tcb *blocked_thread;
		queue_dequeue(sem->blocked_queue, (void **)&blocked_thread);
		uthread_unblock(blocked_thread);
	} else {
		/* No threads waiting, just increment count */
		sem->count++;
	}

	preempt_enable();
	return 0;
}



