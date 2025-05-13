#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef enum // More intuitive representation of the different states.
{
	RUNNING,
	READY,
	BLOOCKED,
	EXITED
} state_type;

struct uthread_tcb
{
	/* TODO Phase 2 */
	state_type st;		   // state the thread is found.
	void *stack;		   // designated stack.
	uthread_ctx_t context; // save thread's context.
};

struct uthread_tcb main_thread;	 // this is the idle thread
struct uthread_tcb *curr_thread; // keeps track of the current thread.
queue_t r_queue;

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2/3 */
	return curr_thread;
}

void uthread_yield(void)
{
	/* TODO Phase 2 */

	if (queue_enqueue(r_queue, curr_thread)) // true, then means it failed -1 == true
	{
		return -1;
	}
	/* Setting up for context switch the next thread in line. */
	struct uthread_tcb *old_thread = curr_thread;
	struct uthread_tcb *new_thread; // get the next thread from the queue
	if (queue_dequeue(r_queue, (void **)&new_thread))
	{
		return -1;
	}
	if (old_thread->st != EXITED)
	{
		old_thread->st = READY;
	}
	curr_thread = new_thread;
	new_thread->st = RUNNING;
	uthread_ctx_switch(&old_thread->context, &new_thread->context);
}

void uthread_exit(void)
{
	/* TODO Phase 2 */
	curr_thread->st = EXITED;
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	struct uthread_tcb *new_task = (struct uthread_tcb *)malloc(sizeof(struct uthread_tcb));
	if (new_task == NULL)
	{
		return -1;
	}
	new_task->st = READY;
	new_task->stack = uthread_ctx_alloc_stack();
	if (new_task->stack == NULL)
	{
		free(new_task);
		return -1;
	}
	if (uthread_ctx_init(&new_task->context, new_task->stack, func, arg) || queue_enqueue(r_queue, new_task))
	{
		uthread_ctx_destroy_stack(new_task->stack);
		free(new_task);
		return -1;
	}
	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */

	if (!preempt)
	{
		r_queue = queue_create();

		uthread_create(func, arg); // Add the newly created TBC to the queue.

		/* Setting the idle thread to be the first running thread*/
		struct uthread_tcb *prev_thread = &main_thread;
		while (queue_length(r_queue) > 0)
		{
			queue_dequeue(r_queue, (void **)&curr_thread);
			if (curr_thread->st == EXITED) // This means the thread exited and needs to be deleted.
			{
				uthread_ctx_destroy_stack(curr_thread->stack);
				free(curr_thread);
				continue;
			}
			uthread_ctx_switch(&prev_thread->context, &curr_thread->context);
			prev_thread = curr_thread;
		}
		queue_destroy(r_queue);
		r_queue = NULL;
		return 0;
	}
}

void uthread_block(void)
{
	/* TODO Phase 3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 3 */
}
