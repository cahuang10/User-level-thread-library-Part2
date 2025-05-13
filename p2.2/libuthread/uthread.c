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

struct uthread_tcb *main_thread; // thread that maneges all the thread.
struct uthread_tcb *curr_thread; // keeps track of the current thread.
queue_t r_queue;

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2/3 */
}

void uthread_yield(void)
{
	/* TODO Phase 2 */
	if (queue_enqueue(r_queue, curr_thread) > -1)
	{
		return -1;
	}
	uthread_ctx_switch(&curr_thread->context, &main_thread->context);
}

void uthread_exit(void)
{
	/* TODO Phase 2 */
}

int uthread_create(uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	struct uthread_tcb *new_task = malloc(sizeof(struct uthread_tcb));
	new_task->st = READY;
	new_task->stack = uthread_ctx_alloc_stack();
	uthread_ctx_init(&new_task->context, new_task->stack, func, arg);
	if (queue_enqueue(r_queue, new_task) > -1)
	{
		return -1;
	}
	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */

	if (!preempt)
	{
		uthread_create(func, arg);
		while (queue_length > 0)
		{
			queue_enqueue(r_queue, &curr_thread);
			uthread_ctx_switch(&main_thread->context, &curr_thread->context);
		}
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
