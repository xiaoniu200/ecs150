#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "queue.h"
#include "private.h"
#include "uthread.h"

typedef enum state {
	Ready,
	Running,
	Blocked,
	Zombie
} state_t;

struct uthread_tcb {
	uthread_ctx_t ctx;
	// state_t state;
};

typedef struct uthread_tcb* uthread_tcb_t;

uthread_tcb_t current_thread = NULL;

queue_t ready_q = NULL;
queue_t blocked_q = NULL;
queue_t zombie_q = NULL;

struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

void uthread_yield(void)
{
	uthread_tcb_t cur_t, next_t;

	preempt_disable();

	// get next thread to run
	if (queue_dequeue(ready_q, (void **)&next_t) == -1) {
		if (queue_dequeue(blocked_q, (void **)&next_t) == -1) {
			return;
		}
	}

	// switch
	cur_t = current_thread;
	current_thread = next_t;
	queue_enqueue(ready_q, cur_t);
	uthread_ctx_switch(&cur_t->ctx, &next_t->ctx);
	preempt_enable();
}

void uthread_exit(void)
{
	uthread_tcb_t cur_t, next_t;

	preempt_disable();
	cur_t = current_thread;

	// stop current thread
	current_thread = NULL;
	queue_enqueue(zombie_q, cur_t);

	// get next thread to run
	if (queue_dequeue(ready_q, (void **)&next_t) == -1) {
		return;
	}

	// switch
	current_thread = next_t;
	uthread_ctx_switch(&cur_t->ctx, &next_t->ctx);
	preempt_enable();
}

uthread_tcb_t create_tcb(uthread_func_t func, void *arg)
{
	uthread_tcb_t new_tcb;

	new_tcb = malloc(sizeof(struct uthread_tcb));
	if (new_tcb == NULL)
		return NULL;
	
	// init thread
	if (uthread_ctx_init(&new_tcb->ctx,
			uthread_ctx_alloc_stack(),
			func,
			arg) == -1)
		return NULL;

	return new_tcb;
}

int uthread_create(uthread_func_t func, void *arg)
{
	queue_enqueue(ready_q, create_tcb(func, arg));

	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	ready_q = queue_create();
	blocked_q = queue_create();
	zombie_q = queue_create();

	// allocate space for `idle` thread
	current_thread = malloc(sizeof(struct uthread_tcb));

	// create `initial` thread
	uthread_create(func, arg);

	preempt_start(preempt);

	while (queue_length(ready_q) > 0 || queue_length(blocked_q) > 0) {
		uthread_yield();
	}

	preempt_stop();
	
	return 0;
}

void uthread_block(void)
{
	uthread_tcb_t cur_t, next_t;

	preempt_disable();

	// get next thread to run
	if (queue_dequeue(ready_q, (void **)&next_t) == -1)
		return;

	// switch
	cur_t = current_thread;
	current_thread = next_t;
	queue_enqueue(blocked_q, cur_t);
	uthread_ctx_switch(&cur_t->ctx, &next_t->ctx);
	preempt_enable();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	if (queue_delete(blocked_q, uthread) == -1)
		return;
	
	queue_enqueue(ready_q, uthread);
}

