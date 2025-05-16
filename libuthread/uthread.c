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
	BLOCKED,
	EXITED
} state_type;

struct uthread_tcb
{
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

	if (curr_thread != &main_thread) // We do not want to enqueue the idle thread as the name implies.
	{
		if (queue_enqueue(r_queue, curr_thread) < 0)
			return;
	}
	/* Setting up for context switch the next thread in line. */
	struct uthread_tcb *old_thread = curr_thread;
	struct uthread_tcb *new_thread; // get the next thread from the queue
	if (queue_dequeue(r_queue, (void **)&new_thread) < 0)
	{
		return;
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
	if (uthread_ctx_init(&new_task->context, new_task->stack, func, arg) < 0 || queue_enqueue(r_queue, new_task) < 0)
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

	/* Initialize ready queue */
	r_queue = queue_create();
	if (!r_queue)
	{
		return -1;
	}

	/* Initialize main thread context */
	main_thread.st = RUNNING;
	main_thread.stack = NULL; 
	curr_thread = &main_thread;

	preempt_start(preempt);

	/* Create the initial thread */
	if (uthread_create(func, arg) < 0)
	{
		preempt_stop();
		queue_destroy(r_queue);
		return -1;
	}

	/* Main scheduling loop */
	while (queue_length(r_queue) > 0)
	{
		/* Get next thread to run */
		struct uthread_tcb *next_thread;
		if (queue_dequeue(r_queue, (void **)&next_thread) < 0)
			break;

		/* Skip exited threads and clean them up */
		if (next_thread->st == EXITED)
		{
			uthread_ctx_destroy_stack(next_thread->stack);
			free(next_thread);
			continue;
		}

		/* Switch to the next thread */
		struct uthread_tcb *prev_thread = curr_thread;
		curr_thread = next_thread;
		curr_thread->st = RUNNING;
		uthread_ctx_switch(&prev_thread->context, &next_thread->context);
	}

	preempt_stop();
	queue_destroy(r_queue);
	r_queue = NULL;

	return 0;
}

void uthread_block(void)
{
	/* Disable preemption while manipulating thread state */
	preempt_disable();

	curr_thread->st = BLOCKED;

	/* Re-enable preemption */
	preempt_enable();

	/* Get next thread to run */
	struct uthread_tcb *next_thread;
	if (queue_dequeue(r_queue, (void **)&next_thread) == 0)
	{
		struct uthread_tcb *old_thread = curr_thread;
		curr_thread = next_thread;
		curr_thread->st = RUNNING;
		uthread_ctx_switch(&old_thread->context, &next_thread->context);
	}
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* Disable preemption while manipulating thread state */
	preempt_disable();

	/* Put thread back into ready queue */
	if (uthread && uthread->st == BLOCKED)
	{
		uthread->st = READY;
		queue_enqueue(r_queue, uthread);
	}

	/* Re-enable preemption */
	preempt_enable();
}
