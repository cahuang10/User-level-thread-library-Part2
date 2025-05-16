#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#define HZ 100

static struct sigaction old_action;
static struct itimerval old_timer;
static bool preemption_enabled = false;

static void preempt_handler(int sig)
{
	(void)sig;
	uthread_yield();
}

void preempt_disable(void)
{
	sigset_t mask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);
}

void preempt_enable(void)
{
	sigset_t mask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void preempt_start(bool preempt)
{
	if (!preempt)
		return;

	/* Instll signal handler for SIGVTALRM */
	struct sigaction action;
	action.sa_handler = preempt_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	
	if (sigaction(SIGVTALRM, &action, &old_action) < 0) {
		perror("sigaction");
		exit(1);
	}

	/* Configure timer to fire 100 times per second */
	struct itimerval timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000000 / HZ; /* 10000 microseconds = 1/100 second */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000000 / HZ;

	if (setitimer(ITIMER_VIRTUAL, &timer, &old_timer) < 0) {
		perror("setitimer");
		exit(1);
	}

	preemption_enabled = true;
}

void preempt_stop(void)
{
	if (!preemption_enabled)
		return;

	/* Restoring old timer */
	setitimer(ITIMER_VIRTUAL, &old_timer, NULL);

	/* Restoring old signal action */
	sigaction(SIGVTALRM, &old_action, NULL);

	preemption_enabled = false;
}
