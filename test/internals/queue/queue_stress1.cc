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
#include "util/queue/GccCmpSwapQueue.h"
#include "util/queue/MutexedMultiQueue.h"
#include "util/queue/Queue.h"
#include "util/queue/MutexedQueue.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

static inline pid_t gettid() {
	return syscall(SYS_gettid);
}

#ifndef MAX_PTHREADS
#define MAX_PTHREADS	8
#endif // MAX_PTHREADS

#define QUEUE1_NAME	"GccThreadSafeQueue<GccProcCounter>"
#define QUEUE1_TYPE	(1 << 0)
typedef PAMI::MutexedQueue<PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter> > queue_1a;
typedef PAMI::GccThreadSafeQueue<queue_1a> queue_1;


#define QUEUE2_NAME	"MutexedQueue<GccProcCounter>"
#define QUEUE2_TYPE	(1 << 1)
typedef PAMI::MutexedQueue<PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter> > queue_2;

#define QUEUE3_NAME	"GccThreadSafeQueue<GccCmpSwap>"
#define QUEUE3_TYPE	(1 << 2)
typedef PAMI::GccCmpSwapQueue queue_3a;
typedef PAMI::GccThreadSafeQueue<queue_3a> queue_3;

#ifdef __bgp__
#include "components/atomic/bgp/LockBoxMutex.h"
#define QUEUE4_NAME	"MutexedQueue<LockBoxProcMutex>"
#define QUEUE4_TYPE	(1 << 3)
typedef PAMI::MutexedQueue<PAMI::Mutex::BGP::LockBoxProcMutex> queue_4;

#define QUEUE5_NAME	"GccThreadSafeQueue<LockBoxProcMutex>"
#define QUEUE5_TYPE	(1 << 4)
typedef PAMI::MutexedQueue<PAMI::Mutex::BGP::LockBoxProcMutex> queue_5a;
typedef PAMI::GccThreadSafeQueue<queue_5a> queue_5;

#define QUEUE_ALL	(QUEUE1_TYPE | QUEUE2_TYPE | QUEUE3_TYPE | QUEUE4_TYPE | QUEUE5_TYPE)
#endif

#ifdef __bgq__
#include "components/atomic/bgq/L2Mutex.h"
#define QUEUE4_NAME	"MutexedQueue<L2ProcMutex>"
#define QUEUE4_TYPE	(1 << 3)
typedef PAMI::MutexedQueue<PAMI::Mutex::BGQ::L2ProcMutex> queue_4;

#define QUEUE5_NAME	"GccThreadSafeQueue<L2ProcMutex>"
#define QUEUE5_TYPE	(1 << 4)
typedef PAMI::MutexedQueue<PAMI::Mutex::BGQ::L2ProcMutex> queue_5a;
typedef PAMI::GccThreadSafeQueue<queue_5a> queue_5;

#define QUEUE_ALL	(QUEUE1_TYPE | QUEUE2_TYPE | QUEUE3_TYPE | QUEUE4_TYPE | QUEUE5_TYPE)
#endif

#ifndef QUEUE_ALL
#define QUEUE_ALL	(QUEUE1_TYPE | QUEUE2_TYPE | QUEUE3_TYPE)
#endif // ! QUEUE_ALL

#define DEBUG	// define/undef

template <class T_Queue, int T_BackoffNS = 0, int T_RandMask = 0>
class QueueTest {
public:

	struct element_t {
		typename T_Queue::Element elem;
		unsigned int pid;
		unsigned int seq;
		unsigned int val;
	};

	PAMI::Memory::MemoryManager mm;
	T_Queue queue;

	double base_t;
	const char *name;
	const int testnum;
	int pthreads;
	int elements;
	int seed;

	QueueTest(const int tnum, const char *n, int pth, int elem, int s) :
	name(n),
	testnum(tnum),
	pthreads(pth),
	elements(elem),
	seed(s)
	{
		size_t size = 32*1024;
		void *ptr = malloc(size);
		assert(ptr);
		mm.init(ptr, size);
		queue.init(&mm);

		int x;
		unsigned long long t1, t0 = PAMI_Wtimebase();
		for (x = 0; x < 10000; ++x) {
			t1 = PAMI_Wtimebase() - t0;
		}
		t1 = PAMI_Wtimebase() - t0;
		double d = t1;
		base_t = d / x;
	}

	static void *enqueuers(void *v) {
		QueueTest *thus = (QueueTest *)v;
		T_Queue *q = &thus->queue;
		int num = thus->elements;
		int x;
		element_t *e;
		unsigned long long t0, t = 0;
		timespec tv = {0, T_BackoffNS};

		fprintf(stderr, "%d: starting %d enqueues\n", gettid(), num);
		for (x = 0; x < num; ++x) {
			if (T_BackoffNS) nanosleep(&tv, NULL);
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
		fprintf(stderr, "%d: test %d finished %d enqueues (%g cycles each)\n",
				gettid(), thus->testnum, num, (d / num) - thus->base_t);
		return NULL;
}

	static void *dequeuer(void *v) {
		QueueTest *thus = (QueueTest *)v;
		T_Queue *q = &thus->queue;
		int num = thus->elements * (thus->pthreads - 1);
		int x = 0, y = 0, z = 0;
		int yy, xx;
		element_t *e;
		unsigned long long t0, t1, t = 0, tr = 0, tb = 0;
#ifdef DEBUG
		int dbg = 0;
#endif // DEBUG

		fprintf(stderr, "%d: looking for %d dequeues\n", gettid(), num);
		typename T_Queue::Iterator qi;
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
			xx = yy = 0;
			t0 = PAMI_Wtimebase();
			for (; q->iter_check(&qi); q->iter_end(&qi)) {
				++yy;
				e = (element_t *)q->iter_current(&qi);
				if (e->val == (unsigned)-1 || (rand() & T_RandMask) == 0) {
					e->val = -1;
					t += PAMI_Wtimebase() - t0;
					t0 = PAMI_Wtimebase();
					if (q->iter_remove(&qi) == PAMI_SUCCESS) {
						tr += PAMI_Wtimebase() - t0;
#ifdef DEBUG
						dbg = 0;
#endif // DEBUG
						free(e);
						e = NULL; // just in case we try to access it
						++xx;
					} else {
						tr += PAMI_Wtimebase() - t0;
					}
					t0 = PAMI_Wtimebase();
				}
			}
			t += PAMI_Wtimebase() - t0;
			y += yy;
			x += xx;
#ifdef DEBUG
			if (!xx && ++dbg == 100000) {
				fprintf(stderr, "stuck? %d %d queue = { %p %p %zu }\n",
					y, x, q->head(), q->tail(), q->size());
				abort();
			}
#endif // DEBUG
		}
		double d = t + tr + tb;
		double dr = tr;
		double db = tb;
		if (z) {
			fprintf(stderr, "%d: test %d finished %d dequeues "
				"(%g cycles per iter-elem (%d), "
				"%g per remove, %g per merge (%d))\n",
				gettid(), thus->testnum, num, d / y, y, dr / num, db / z, z);
		} else {
			fprintf(stderr, "%d: test %d finished %d dequeues "
				"(%g cycles per iter-elem (%d), "
				"%g per remove)\n",
				gettid(), thus->testnum, num, d / y, y, dr / num);
		}
		return NULL;
	}

	pthread_attr_t attr[MAX_PTHREADS];
	pthread_t thread[MAX_PTHREADS];

	int run_test(void) {
		int x;
		unsigned long long t0 = PAMI_Wtimebase();
		fprintf(stderr, "main: test %d starting %s test with %d threads "
				"and %d elements per enqueuer (seed %d)\n",
				testnum, name, pthreads, elements, seed);

		int status;
		// thread "0" is the main thread - already running
		for (x = 1; x < pthreads; ++x) {
			pthread_attr_init(&attr[x]);
			pthread_attr_setscope(&attr[x], PTHREAD_SCOPE_SYSTEM);
			status = pthread_create(&thread[x], &attr[x],
						enqueuers, (void *)this);
			/* don't care about status? just reap threads below? */
		}
		// create args struct for main thread
		(void)dequeuer((void *)this);

		for (x = 1; x < pthreads; ++x) {
			pthread_join(thread[x], NULL);
		}
		t0 = PAMI_Wtimebase() - t0;
		fprintf(stderr, "test %d main done. queue = { %p %p %zu } (%lld cy)\n"
				"----------------------------------------------------\n",
				testnum, queue.head(), queue.tail(), queue.size(), t0);

		return (queue.head() || queue.tail() || queue.size());
	}

}; // class QueueTest

int main(int argc, char **argv) {
	int x;
	int pthreads = 4;
	int elements = 1000;
	int seed = 1;
	int qtype = 0;

	//extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "e:p:q:s:")) != EOF) {
		switch(x) {
		case 'e':
			elements = strtol(optarg, NULL, 0);
			break;
		case 'p':
			pthreads = strtol(optarg, NULL, 0);
			break;
		case 'q': // may specify multiple...
			qtype |= (1 << (strtol(optarg, NULL, 0) - 1));
			if (qtype & ~QUEUE_ALL) {
				fprintf(stderr, "Invalid queue type %s\n", optarg);
				exit(1);
			}
			break;
		case 's':
			seed = strtol(optarg, NULL, 0);
			break;
		}
	}
	if (!qtype) qtype = QUEUE_ALL; // default to all tests
	int ret = 0;
	if (qtype & QUEUE1_TYPE) {
		srand(seed);
		QueueTest<queue_1,0> test1(1, QUEUE1_NAME, pthreads, elements, seed);
		ret += test1.run_test();
	}
	if (qtype & QUEUE2_TYPE) {
		srand(seed);
		QueueTest<queue_2,0> test2(2, QUEUE2_NAME, pthreads, elements, seed);
		ret += test2.run_test();
	}
	if (qtype & QUEUE3_TYPE) {
		srand(seed);
		QueueTest<queue_3,0> test3(3, QUEUE3_NAME, pthreads, elements, seed);
		ret += test3.run_test();
	}
#ifdef QUEUE4_NAME
	if (qtype & QUEUE4_TYPE) {
		srand(seed);
		QueueTest<queue_4,0> test4(4, QUEUE4_NAME, pthreads, elements, seed);
		ret += test4.run_test();
	}
#endif // QUEUE4_NAME
#ifdef QUEUE5_NAME
	if (qtype & QUEUE5_TYPE) {
		srand(seed);
		QueueTest<queue_5,0> test5(5, QUEUE5_NAME, pthreads, elements, seed);
		ret += test5.run_test();
	}
#endif // QUEUE5_NAME
	exit(ret);
}
