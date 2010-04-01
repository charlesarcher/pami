/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/queue/gendev_perf.cc
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

// A cheat to override GenericDeviceWorkQueue, etc...
#include "GenericDevicePlatform.h"
#if 1
#define QUEUE_NAME	"MutexedQueue<GenericDeviceMutex>"
#define GenericDeviceWorkQueue	PAMI::MutexedQueue<GenericDeviceMutex>
#else
#define QUEUE_NAME	"GccThreadSafeQueue"
#define GenericDeviceWorkQueue	PAMI::GccThreadSafeQueue
#endif

#include "sys/pami.h"
#include "components/memory/MemoryManager.h"
#include "util/queue/GccThreadSafeMultiQueue.h"
#include "util/queue/GccThreadSafeQueue.h"
#include "util/queue/MutexedMultiQueue.h"
#include "util/queue/Queue.h"
#include "util/queue/MutexedQueue.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

#include "components/devices/generic/Device.h"
#include "components/devices/generic/AdvanceThread.h"

static inline pid_t gettid() {
	return syscall(SYS_gettid);
}

// template <class T_GenDev, int T_NumCtx>
class QueueTest {
	static const size_t num_ctx = 1;
public:

	PAMI::Memory::MemoryManager mm;

	PAMI::Device::Generic::Device *queue;
	typedef PAMI::Device::Generic::GenericThread element_t;

	double base_t;
	const char *name;
	int elements;
	int seed;
	int done;

	QueueTest(const char *n, int pth, int elem, int s) :
	name(n),
	elements(elem),
	seed(s)
	{
		size_t size = 32*1024;
		void *ptr = malloc(size);
		assert(ptr);
		mm.init(ptr, size);

		size_t x;
		unsigned long long t1, t0 = PAMI_Wtimebase();
		for (x = 0; x < 10000; ++x) {
			t1 = PAMI_Wtimebase() - t0;
		}
		t1 = PAMI_Wtimebase() - t0;
		double d = t1;
		base_t = d / x;

		queue = PAMI::Device::Generic::Device::Factory::generate(0, num_ctx, mm);
		for (x = 0; x < num_ctx; ++x) {
			PAMI::Device::Generic::Device::Factory::init(queue, 0, x,
							(pami_client_t)(x+1),
							(pami_context_t)(x*2+1),
							&mm, queue);
		}
	}

	static pami_result_t work(pami_context_t ctx, void *cookie) {
		QueueTest *thus = (QueueTest *)cookie;
		++thus->done;
		return PAMI_SUCCESS;
	}

	static void *enqueuers(void *v) {
		QueueTest *thus = (QueueTest *)v;
		int num = thus->elements;
		int x;
		element_t *e;
		unsigned long long t0, t = 0;

		fprintf(stderr, "%d: starting %d enqueues\n", gettid(), num);
		for (x = 0; x < num; ++x) {
			e = (element_t *)malloc(sizeof(*e));
			assert(e);
			e->setStatus(PAMI::Device::Ready);
			e->setFunc(work, thus);

			PAMI::Device::Generic::Device *gd =
				&PAMI::Device::Generic::Device::Factory::getDevice(thus->queue, 0, 0);
			t0 = PAMI_Wtimebase();
			gd->postThread(e);
			t += PAMI_Wtimebase() - t0;
		}
		double d = t;
		fprintf(stderr, "%d: finished %d enqueues (%g cycles each) %g\n",
				gettid(), num, d / num, thus->base_t);
		return NULL;
}

	static void *dequeuer(void *v) {
		QueueTest *thus = (QueueTest *)v;
		int num = thus->elements * thus->num_ctx;
		size_t x;
		unsigned long long t0, t = 0;

		fprintf(stderr, "%d: looking for %d dequeues\n", gettid(), num);
		t0 = PAMI_Wtimebase();
		while (thus->done < num) {
			for (x = 0; x < thus->num_ctx; ++x) {
				PAMI::Device::Generic::Device::Factory::advance(thus->queue, 0, x);
			}
		}
		t += PAMI_Wtimebase() - t0;

		double d = t;
		fprintf(stderr, "%d: finished %d dequeues (%g cycles ea)\n",
				gettid(), num, d / num);
		return NULL;
	}

	int run_test(void) {
		fprintf(stderr, "main: starting %s test with %d threads "
				"and %d elements per enqueuer (seed %d)\n",
				name, 1, elements, seed);
		done = 0;
		(void)enqueuers((void *)this);
		(void)dequeuer((void *)this);
		return 0;
	}

}; // class QueueTest

int main(int argc, char **argv) {
	int x;
	int elements = 1000;
	int seed = 1;

	//extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "e:s:")) != EOF) {
		switch(x) {
		case 'e':
			elements = strtol(optarg, NULL, 0);
			break;
		case 's':
			seed = strtol(optarg, NULL, 0);
			srand(seed);
			break;
		}
	}
	int ret = 0;
	QueueTest test1(QUEUE_NAME, 0, elements, seed);
	ret = test1.run_test();
	exit(ret);
}
