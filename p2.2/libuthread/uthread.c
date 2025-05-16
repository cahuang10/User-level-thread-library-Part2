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

typedef enum
{
	RUNNING,
	READY,
	BLOCKED,
	EXITED
} state_type;

struct uthread_tcb
{
	state_type st;
	void *stack;
	uthread_ctx_t context;
};

static struct uthread_tcb *curr_thread = NULL;
static queue_t ready_queue = NULL;

struct uthread_tcb *uthread_current(void)
{
	return curr_thread;
}

void uthread_yield(void)
{
	struct uthread_tcb *old_thread = curr_thread;
	struct uthread_tcb *new_thread;

	/* Put current thread back into ready queue if not exited */
	if (curr_thread->st == RUNNING) {
		curr_thread->st = READY;
		queue_enqueue(ready_queue, curr_thread);
	}

	/* Get next thread from ready queue */
	if (queue_dequeue(ready_queue, (void **)&new_thread) == 0) {
		curr_thread = new_thread;
		curr_thread->st = RUNNING;
		uthread_ctx_switch(&old_thread->context, &new_thread->context);
	}
	/* If no threads available, the current thread continues running */
}

void uthread_exit(void)
{
	curr_thread->st = EXITED;
	
	/* Get next thread to run */
	struct uthread_tcb *next_thread;
	if (queue_dequeue(ready_queue, (void **)&next_thread) == 0) {
		curr_thread = next_thread;
		curr_thread->st = RUNNING;
		/* This context switch will never return to the exited thread */
		uthread_ctx_switch(&curr_thread->context, &next_thread->context);
	}
	/* If no more threads, this will return to main thread's scheduling loop */
	curr_thread = NULL;
}

int uthread_create(uthread_func_t func, void *arg)
{
	struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));
	if (!new_thread)
		return -1;

	new_thread->st = READY;
	new_thread->stack = uthread_ctx_alloc_stack();
	if (!new_thread->stack) {
		free(new_thread);
		return -1;
	}

	if (uthread_ctx_init(&new_thread->context, new_thread->stack, func, arg) < 0) {
		uthread_ctx_destroy_stack(new_thread->stack);
		free(new_thread);
		return -1;
	}

	queue_enqueue(ready_queue, new_thread);
	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* Create ready queue */
	ready_queue = queue_create();
	if (!ready_queue) {
		return -1;
	}

	/* Create initial thread */
	if (uthread_create(func, arg) < 0) {
		queue_destroy(ready_queue);
		return -1;
	}

	/* Start preemption if requested */
	if (preempt) {
		preempt_start(true);
	}

	/* Main scheduling loop - run until no more threads */
	while (queue_length(ready_queue) > 0) {
		struct uthread_tcb *thread;
		
		/* Get next thread */
		if (queue_dequeue(ready_queue, (void **)&thread) == 0) {
			/* Clean up any exited threads first */
			while (thread->st == EXITED) {
				uthread_ctx_destroy_stack(thread->stack);
				free(thread);
				
				/* Get next thread */
				if (queue_dequeue(ready_queue, (void **)&thread) != 0) {
					/* No more threads */
					goto cleanup;
				}
			}
			
			/* Run the thread */
			curr_thread = thread;
			curr_thread->st = RUNNING;
			
			/* Create a temporary context for the main thread */
			uthread_ctx_t main_ctx;
			uthread_ctx_switch(&main_ctx, &thread->context);
			
			/* When we return here, the thread has yielded or exited */
			/* Clean up if thread exited */
			if (curr_thread && curr_thread->st == EXITED) {
				uthread_ctx_destroy_stack(curr_thread->stack);
				free(curr_thread);
				curr_thread = NULL;
			}
		}
	}

cleanup:
	/* Stop preemption if it was started */
	if (preempt) {
		preempt_stop();
	}

	/* Clean up remaining threads */
	struct uthread_tcb *thread;
	while (queue_dequeue(ready_queue, (void **)&thread) == 0) {
		uthread_ctx_destroy_stack(thread->stack);
		free(thread);
	}

	queue_destroy(ready_queue);
	ready_queue = NULL;
	curr_thread = NULL;

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
	if (queue_dequeue(ready_queue, (void **)&next_thread) == 0) {
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
	if (uthread && uthread->st == BLOCKED) {
		uthread->st = READY;
		queue_enqueue(ready_queue, uthread);
	}
	
	/* Re-enable preemption */
	preempt_enable();
}