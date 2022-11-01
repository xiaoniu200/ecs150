#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
	int d;
	queue_t q;
};

sem_t sem_create(size_t count)
{
	sem_t sem = malloc(sizeof(struct semaphore));
	if (sem == NULL)
		return NULL;

	sem->d = count;
	sem->q = queue_create();
	if (sem->q == NULL) {
		free(sem);
		return NULL;
	}

	return sem;
}

int sem_destroy(sem_t sem)
{
	if (sem == NULL || queue_length(sem->q) > 0) {
		return -1;
	}

	queue_destroy(sem->q);
	free(sem);

	return 0;
}

int sem_down(sem_t sem)
{
	if (sem == NULL)
		return -1;
	
	preempt_disable();
	while (1) {
		if (sem->d > 0) {
			sem->d--;
			break;
		} else {
			queue_enqueue(sem->q, uthread_current());
			uthread_block();
		}
		preempt_disable();
	}
	preempt_disable();

	return 0;
}

int sem_up(sem_t sem)
{
	if (sem == NULL)
		return -1;

	preempt_disable();
	sem->d++;
	if (queue_length(sem->q) > 0) {
		struct uthread_tcb *t;
		queue_dequeue(sem->q, (void **)&t);
		uthread_unblock(t);
	}
	preempt_enable();
	return 0;
}

