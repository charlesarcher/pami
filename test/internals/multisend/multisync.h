/**
 * \file test/internals/multisend/multisync.h
 * \brief ???
 */

#ifndef __test_internals_multisend_multisync_h__
#define __test_internals_multisend_multisync_h__

#include <stdio.h>

#include "Topology.h"
#include "Global.h"
#undef USE_COMMTHREADS

#include "sys/pami.h"
#include "memorymanager.h"
#include "components/devices/generic/Device.h"

namespace PAMI {
namespace Test {
namespace Multisend {

template <class T_MultisyncModel, class T_MultisyncDevice>
class Multisync {
private:
        uint8_t _mdlbuf[sizeof(T_MultisyncModel)];
        T_MultisyncModel *_model;
        uint8_t _msgbuf[T_MultisyncModel::sizeof_msg];
        PAMI::Device::Generic::Device *_generics;
        T_MultisyncDevice *_dev;
        pami_result_t _status;
        int _done;
        const char *_name;

        static void _done_cb(pami_context_t context, void *cookie, pami_result_t result) {
                PAMI::Test::Multisend::Multisync<T_MultisyncModel,T_MultisyncDevice> *thus = (PAMI::Test::Multisend::Multisync<T_MultisyncModel,T_MultisyncDevice> *)cookie;
                // printf skews timing too much...
                //fprintf(stderr, "... completion callback for %s, done %d++\n", thus->_name, thus->_done);
                ++thus->_done;
        }

public:
        unsigned long long delay;
        unsigned long long raw_time;
        unsigned long long total_time;
        unsigned long long barrier_time;

        Multisync(const char *test, PAMI::Memory::MemoryManager &mm) :
        _name(test)
        {
                _generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, mm, NULL);
                _dev = T_MultisyncDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                T_MultisyncDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MultisyncModel(T_MultisyncDevice::Factory::getDevice(_dev, 0, 0), _status);
        }

        ~Multisync() {}

        /// msync is partially filled-in.
        ///
        inline pami_result_t perform_test(size_t task_id, size_t num_tasks,
                                        pami_context_t ctx, pami_multisync_t *msync) {
                unsigned long long t0, t1, t2;
                pami_result_t rc;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                msync->cb_done = (pami_callback_t){_done_cb, (void *)this};

                // Do three barriers: first is to synchronize the ranks so that
                // the timing of the follwing barriers are cleaner. Also, no printfs
                // in the barriers because that messes up timing.  So, if the barrier
                // needs debug then the printfs will have to be re-enabled.

                // Basic timing profile: Each rank will delay prior to the start
                // by the rank multiplied by 1000 (cycles). The idea being, if the
                // barrier actually does it's job then the total time spent by each
                // rank should be (relatively) close to the others. NOTE: these times
                // are not to be used as performance numbers! They intentionally introduce
                // extra time in order to show that the barrier is functional.

                // first barrier: get everyone together
                _done = 0;
                //fprintf(stderr, "... before %s.postMultisync\n", _name);
                rc = _model->postMultisync(_msgbuf,msync);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post first multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }
                // printf skews timing too much...
                //fprintf(stderr, "... before advance loop for %s.postMultisync\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MultisyncDevice::Factory::advance(_dev, 0, 0);
                }

                // second barrier: get an accurate time
                ++msync->connection_id;
                _done = 0;
                t0 = __global.time.timebase();
                rc = _model->postMultisync(_msgbuf,msync);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post second multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MultisyncDevice::Factory::advance(_dev, 0, 0);
                }
                raw_time = __global.time.timebase() - t0;
                delay = raw_time * task_id;

                // In order to get meaningful results below, we need to vary each rank's
                // arrival time at the barrier by a "significant" amount. We use the
                // time it took for the initial barrier, multiplied by our task ID, to
                // create noticable differences in arrival time (we hope).
                //
                // The resulting timings should show that barrier times (in parens)
                // vary a lot while the overall times should be more closely aligned.
                //
                // +--------------------------------------------------------------+
                // |                                        |    barrier          |
                // +--------------------------------------------------------------+
                // |                         |    barrier                         |
                // +--------------------------------------------------------------+
                // |               |    barrier                                   |
                // +--------------------------------------------------------------+
                // |                                             |    barrier     |
                // +--------------------------------------------------------------+
                // t0            (t1[x] ...         ...      ... )                t2
                //

                // third barrier: check that it really synchronizes participants
                ++msync->connection_id;
                _done = 0;
                t0 = __global.time.timebase();
                while ((t1 = __global.time.timebase()) - t0 < delay);
                rc = _model->postMultisync(_msgbuf,msync);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post third multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                // printf skews timing too much...
                //fprintf(stderr, "... before advance loop for %s.postMultisync\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MultisyncDevice::Factory::advance(_dev, 0, 0);
                }
                t2 = __global.time.timebase();
                total_time = t2 - t0;
                barrier_time = t2 - t1;

                return PAMI_SUCCESS;
        }

private:

}; // class Multisync

template <class T_MultisyncModel, class T_MultisyncDevice>
class Multisync_mutex {
private:
        uint8_t _mdlbuf[sizeof(T_MultisyncModel)];
        T_MultisyncModel *_model;
        uint8_t _msgbuf[T_MultisyncModel::sizeof_msg];
        PAMI::Device::Generic::Device *_generics;
        T_MultisyncDevice *_dev;
        pami_result_t _status;
        int _done;
        const char *_name;
	int *_sema; // used to check for mutex/serialization faults

        static void _done_cb(pami_context_t context, void *cookie, pami_result_t result) {
                PAMI::Test::Multisend::Multisync_mutex<T_MultisyncModel,T_MultisyncDevice> *thus = (PAMI::Test::Multisend::Multisync_mutex<T_MultisyncModel,T_MultisyncDevice> *)cookie;
                fprintf(stderr, "... completion callback for %s, done %d++ (seq %d)\n", thus->_name, thus->_done, *thus->_sema);
		++(*thus->_sema);
                ++thus->_done;

        }

public:
        Multisync_mutex(const char *test, PAMI::Memory::MemoryManager &mm) :
        _name(test)
        {
                _generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, mm, NULL);
                _dev = T_MultisyncDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                T_MultisyncDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MultisyncModel(T_MultisyncDevice::Factory::getDevice(_dev, 0, 0), _status);
		mm.memalign((void **)&_sema, sizeof(*_sema), sizeof(*_sema));
        }

        ~Multisync_mutex() {}

        /// msync is partially filled-in.
        ///
        inline pami_result_t perform_test(size_t task_id, size_t num_tasks,
                                        pami_context_t ctx, pami_multisync_t *msync) {
                pami_result_t rc;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                msync->cb_done = (pami_callback_t){_done_cb, (void *)this};
                msync->roles = _model->LOCK_ROLE;

                // first barrier: get everyone together
                _done = 0;
                fprintf(stderr, "... before %s.postMultisync\n", _name);
                rc = _model->postMultisync(_msgbuf,msync);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post first multisync \"%s\"\n", _name);
                        return PAMI_ERROR;
                }
                fprintf(stderr, "... before advance loop for %s.postMultisync\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MultisyncDevice::Factory::advance(_dev, 0, 0);
                }

                msync->cb_done = (pami_callback_t){NULL, NULL};
                msync->roles = _model->UNLOCK_ROLE;
		// we know this won't require completion... we're done now...
                rc = _model->postMultisync(_msgbuf,msync);

                return PAMI_SUCCESS;
        }

private:

}; // class Multisync_mutex
}; // namespace Multisend
}; // namespace Test
}; // namespace PAMI

#define DO_BARRIER_TEST(name,model,device,islocal,mm,task_id,num_tasks,context)	\
{										\
	const char *test = name;						\
	pami_result_t rc;							\
	pami_multisync_t msync;							\
	if (islocal && __global.topology_local.size() < 2) {			\
		if (task_id == 0) fprintf(stderr, "SKIPPING: %s requires more than 1 local task\n", test);\
	} else if (!islocal && __global.topology_local.size() > 1) {		\
		if (task_id == 0) fprintf(stderr, "SKIPPING: %s requires only 1 task per node\n", test);\
	} else {								\
		initializeMemoryManager("multisync test", 0, mm);		\
		msync.client = 0;						\
		msync.context = 0;						\
		msync.roles = (unsigned)-1;					\
		msync.participants = (pami_topology_t *)(islocal ?		\
			 &__global.topology_local : &__global.topology_global);	\
		if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);	\
		PAMI::Test::Multisend::Multisync<model,device> testclass(test, mm);\
		rc = testclass.perform_test(task_id, num_tasks, context, &msync);\
		if (rc != PAMI_SUCCESS) {					\
			fprintf(stderr, "Failed %s test result = %d\n", test, rc);\
			exit(1);						\
		}								\
		fprintf(stderr, "PASS2? %5lld (%5lld) [delay: %lld, time: %lld]\n",\
			testclass.total_time, testclass.barrier_time,		\
			testclass.delay, testclass.raw_time);			\
	}									\
}

#define DO_MUTEX_TEST(name,model,device,islocal,mm,task_id,num_tasks,context)	\
{										\
	const char *test = name;						\
	pami_result_t rc;							\
	pami_multisync_t msync;							\
	if (islocal && __global.topology_local.size() < 2) {			\
		if (task_id == 0) fprintf(stderr, "SKIPPING: %s requires more than 1 local task\n", test);\
	} else if (!islocal && __global.topology_local.size() > 1) {		\
		if (task_id == 0) fprintf(stderr, "SKIPPING: %s requires only 1 task per node\n", test);\
	} else {								\
		initializeMemoryManager("multisync test", 0, mm,		\
			__global.topology_local.size(),				\
			__global.topology_local.index2Rank(0) == task_id);	\
		msync.client = 0;						\
		msync.context = 0;						\
		msync.roles = (unsigned)-1;					\
		msync.participants = (pami_topology_t *)(islocal ?		\
			 &__global.topology_local : &__global.topology_global);	\
		if (task_id == 0) fprintf(stderr, "=== Testing %s...\n", test);	\
		PAMI::Test::Multisend::Multisync_mutex<model,device> testclass(test, mm);\
		rc = testclass.perform_test(task_id, num_tasks, context, &msync);\
		if (rc != PAMI_SUCCESS) {					\
			fprintf(stderr, "Failed %s test result = %d\n", test, rc);\
			exit(1);						\
		}								\
	}									\
}

#endif // __pami_test_internals_multisend_multisync_h__
