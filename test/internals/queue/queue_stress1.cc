/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/queue/queue_stress1.cc
 * \brief ???
 */

#define _POSIX_C_SOURCE 199309
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "sys/pami.h"
#include "components/memory/MemoryManager.h"
#include "util/queue/GccThreadSafeMultiQueue.h"
#include "util/queue/GccThreadSafeQueue.h"
#include "util/queue/MutexedMultiQueue.h"
#include "util/queue/Queue.h" // includes MutexedQueue
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

static inline pid_t gettid() {
	return syscall(SYS_gettid);
}

#ifndef MAX_PTHREADS
#define MAX_PTHREADS	8
#endif // MAX_PTHREADS

//typedef PAMI::GccThreadSafeMultiQueue<2,0> queue_t;
typedef PAMI::GccThreadSafeQueue queue_t;
//typedef PAMI::MutexedQueue<PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter> > queue_t;

#ifdef __bgp__
#include "components/atomic/bgp/LockBoxMutex.h"
//typedef PAMI::MutexedMultiQueue<PAMI::Mutex::BGP::LockBoxProcMutex,2,0> queue_t;
//typedef PAMI::MutexedQueue<PAMI::Mutex::BGP::LockBoxProcMutex> queue_t;
#endif

typedef struct {
	queue_t::Element elem;
	unsigned int pid;
	unsigned int seq;
	unsigned int val;
} element_t;

PAMI::Memory::MemoryManager mm;
queue_t queue;

double base_t = 0.0;

struct thread_args {
	queue_t *queue;
	int num_iter;
	int nthreads;
};

void *enqueuers(void *v) {
	struct thread_args *args = (struct thread_args *)v;
	queue_t *q = args->queue;
	int num = args->num_iter;
	int x;
	element_t *e;
	unsigned long long t0, t = 0;
#ifdef BACKOFF_NS
	timespec tv = {0, BACKOFF_NS};
#endif // BACKOFF_NS

	fprintf(stderr, "%d: starting %d enqueues\n", gettid(), num);
	for (x = 0; x < num; ++x) {
#ifdef BACKOFF_NS
		nanosleep(&tv, NULL);
#endif // BACKOFF_NS
		e = (element_t *)malloc(sizeof(*e));
		assert(e);
		e->pid = gettid();
		e->seq = x;
		e->val = 1;	// debug
		t0 = PAMI_Wtimebase();
		q->enqueue(&e->elem);
		t += PAMI_Wtimebase() - t0;
	}
	double d = t;
	fprintf(stderr, "%d: finished %d enqueues (%g cycles each) %g\n", gettid(), num, d / num, base_t);
	return NULL;
}

void *dequeuer(void *v) {
	struct thread_args *args = (struct thread_args *)v;
	queue_t *q = args->queue;
	int num = args->num_iter * (args->nthreads - 1);
	int x = 0, y = 0, z = 0;
	element_t *e;
	unsigned long long t0, t1, t = 0, tr = 0, tb = 0;

	fprintf(stderr, "%d: looking for %d dequeues\n", gettid(), num);
	static queue_t::Iterator qi;
	q->iter_init(&qi);
	while (x < num) {
		if (!q->head()) sched_yield();
		t0 = PAMI_Wtimebase();
		bool b = q->iter_begin(&qi);
		t1 = PAMI_Wtimebase();
		if (b) {
			tb += t1 - t0;
			++z;
		}
		t0 = PAMI_Wtimebase();
		for (; q->iter_check(&qi); q->iter_end(&qi)) {
			++y;
			e = (element_t *)q->iter_current(&qi);
			if (e->val == (unsigned)-1 || (rand() & 0x03) == 0) {
				e->val = 0; // helps debug
				t += PAMI_Wtimebase() - t0;
				t0 = PAMI_Wtimebase();
				if (q->iter_remove(&qi) == PAMI_SUCCESS) {
					tr += PAMI_Wtimebase() - t0;
					free(e);
					e = NULL; // just in case we try to access it
					++x;
				} else {
					tr += PAMI_Wtimebase() - t0;
				}
				t0 = PAMI_Wtimebase();
			}
		}
		t += PAMI_Wtimebase() - t0;
	}
	double d = t;
	double dr = tr;
	double db = tb;
	fprintf(stderr, "%d: finished %d dequeues (%g cycles per iter-elem (%d), %g per remove, %g per merge (%d))\n", gettid(), num, d / y, y, dr / num, db / z, z);
	return NULL;
}

pthread_attr_t attr[MAX_PTHREADS];
pthread_t thread[MAX_PTHREADS];
struct thread_args args[MAX_PTHREADS];

int main(int argc, char **argv) {
	int x;
	int pthreads = 4;
	int elements = 1000;
	int seed = 1;

	//extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "e:mp:s:t")) != EOF) {
		switch(x) {
		case 'e':
			elements = strtol(optarg, NULL, 0);
			break;
		case 'm':
			break;
		case 'p':
			pthreads = strtol(optarg, NULL, 0);
			break;
		case 's':
			seed = strtol(optarg, NULL, 0);
			srand(seed);
			break;
		case 't':
			break;
		}
	}
	fprintf(stderr, "main: starting test with %d threads and %d elements per enqueuer (seed %d)\n", pthreads, elements, seed);

	size_t size = 32*1024;
	void *ptr = malloc(size);
	assert(ptr);
	mm.init(ptr, size);
	queue.init(&mm);

	unsigned long long t1, t0 = PAMI_Wtimebase();
	for (x = 0; x < 10000; ++x) {
		t1 = PAMI_Wtimebase() - t0;
	}
	t1 = PAMI_Wtimebase() - t0;
	double d = t1;
	base_t = d / x;

	int status;
	// thread "0" is the main thread - already running
	for (x = 1; x < pthreads; ++x) {
		pthread_attr_init(&attr[x]);
		pthread_attr_setscope(&attr[x], PTHREAD_SCOPE_SYSTEM);
		args[x].queue = &queue;
		args[x].nthreads = pthreads;
		args[x].num_iter = elements;
		if (x) {
			status = pthread_create(&thread[x], &attr[x], enqueuers, (void *)&args[x]);
		} else {
			status = pthread_create(&thread[x], &attr[x], dequeuer, (void *)&args[x]);
		}
		/* don't care about status? just reap threads below? */
	}
	// create args struct for main thread
	args[0].queue = &queue;
	args[0].nthreads = pthreads;
	args[0].num_iter = elements;
	(void)dequeuer((void *)&args[0]);

	for (x = 1; x < pthreads; ++x) {
		pthread_join(thread[x], NULL);
	}
	fprintf(stderr, "main done. queue = { %p %p %zu }\n", queue.head(), queue.tail(), queue.size());
	if (queue.head() || queue.tail() || queue.size()) {
		exit(1);
	}
	exit(0);
}
