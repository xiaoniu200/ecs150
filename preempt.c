#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction sa, old_sa;
struct itimerval old_it_val;
sigset_t sig;

bool enabled;

void handler(int sig)
{
	(void)sig;
	uthread_yield();
}

void preempt_disable(void)
{
	if (!enabled)
		return;
	
	sigprocmask(SIG_BLOCK, &sig, NULL);
}

void preempt_enable(void)
{
	if (!enabled)
		return;

	sigprocmask(SIG_UNBLOCK, &sig, NULL);
}

void preempt_start(bool preempt)
{
	struct itimerval it_val;

	enabled = preempt;
	if (!enabled)
		return;
	
	// memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler;

	sigemptyset(&sig);
	sigaddset(&sig, SIGVTALRM);

	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = 1e6 / HZ;
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 1e6 / HZ;

	// set action
	if (sigaction(SIGVTALRM, &sa, &old_sa) == -1) {
		enabled = false;
		return;
	}
	// set timer
	if (setitimer(ITIMER_VIRTUAL, &it_val, &old_it_val) == -1) {
		enabled = false;
	}
	return;
}

void preempt_stop(void)
{
	if (!enabled)
		return;

	// restore timer
	setitimer(ITIMER_VIRTUAL, &old_it_val, NULL);
	// restore action
	sigaction(SIGVTALRM, &old_sa, NULL);

	enabled = false;
}

