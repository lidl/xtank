/*
** Xtank
**
** Copyright 2000 by Kurt J. Lidl
**
** $Id$
*/

/*
** A sample program to test pthread creation and scheduling
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

static int ret_a, ret_b, ret_c;

void thread_a(void);
void thread_b(void);
void thread_c(void);
pthread_t self, thd_a, thd_b, thd_c;

int
main(int argc, char *argv[])
{
	int status;
	pthread_attr_t suspended;

	self = pthread_self();

	status = pthread_attr_init(&suspended);
	if (status) {
		perror("pthread_attr_init");
		exit(1);
	}

	status = pthread_attr_setsuspendstate_np(&suspended,PTHREAD_CREATE_SUSPENDED);
	if (status) {
		perror("pthread_attr_setsuspendstate_np");
		exit(1);
	}

	status = pthread_create(&thd_a, &suspended, (void *)thread_a, (void *)0);
	if (status) {
		perror("pthread_create thread_a");
		exit(1);
	}
	status = pthread_create(&thd_b, &suspended, (void *)thread_b, (void *)0);
	if (status) {
		perror("pthread_create thread_b");
		exit(1);
	}
	status = pthread_create(&thd_c, &suspended, (void *)thread_c, (void *)0);
	if (status) {
		perror("pthread_create thread_c");
		exit(1);
	}

	(void) sched_yield();

	printf("after first call to sched_yield\n");

#if defined(__bsdi__) && 1
	pthread_resume_np(thd_a);
	printf("after call to pthread_resume_np(thd_a)\n");
	pthread_resume_np(thd_b);
	printf("after call to pthread_resume_np(thd_b)\n");
	pthread_resume_np(thd_c);
	printf("after call to pthread_resume_np(thd_c)\n");
#endif

#if defined(__bsdi__) && 0
	pthread_resume_all_np();
	printf("after call to pthread_resume_all_np\n");
#endif

	(void) sched_yield();

	printf("after second call to sched_yield\n");

	sleep(4);

	exit (0);
}

void
thread_a(void)
{
	printf("thread_a started\n");

	pthread_suspend_np(thd_a);
	sched_yield();
	sleep(2);

	printf("thread_a finished\n");
	ret_a = 1;
	pthread_exit (&ret_a);
}

void
thread_b(void)
{
	printf("thread_b started\n");

/*	pthread_suspend_np(thd_a); */
	sleep(2);

	printf("thread_b finished\n");
	ret_b = 2;
	pthread_exit (&ret_b);
}

void
thread_c(void)
{
	printf("thread_c started\n");

	sleep(2);

	printf("thread_c finished\n");
	ret_c = 3;
	pthread_exit (&ret_c);
}

