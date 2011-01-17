/**
 * \file test/internals/multisend/bgq/local_multicombine.h
 * \brief ???
 */

#ifndef __test_internals_multisend_bgq_local_multicombine_h__
#define __test_internals_multisend_bgq_local_multicombine_h__

#include <stdio.h>
#include <pami.h>

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"
#undef USE_COMMTHREADS
#include "components/devices/generic/Device.h"

#include "components/devices/MulticombineModel.h"
//#define DATA_CHECK

namespace PAMI {
namespace Test {
namespace Multisend {

template <class T_MulticombineModel, class T_MulticombineDevice, int T_BufSize>
class Multicombine {
private:
        uint8_t _mdlbuf[sizeof(T_MulticombineModel)];
        T_MulticombineModel *_model;
        PAMI::Device::Generic::Device *_generics;
        T_MulticombineDevice *_dev;
        uint8_t _msgbuf[T_MulticombineModel::sizeof_msg];

	char *_source, *_result;

        PAMI::PipeWorkQueue _ipwq;
        PAMI::PipeWorkQueue _opwq;

        pami_result_t _status;
        int _done;
        const char *_name;

        static void _done_cb(pami_context_t context, void *cookie, pami_result_t result) {
                PAMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *thus =
                        (PAMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *)cookie;
                //fprintf(stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_done);
                ++thus->_done;
        }

public:

        Multicombine(const char *test, PAMI::Memory::MemoryManager &mm) :
        _name(test)
        {
		pami_result_t rc;
		uint64_t my_alignment;
		my_alignment = 128;
		rc = __global.heap_mm->memalign((void **)&_source, my_alignment, T_BufSize);
		if (rc != PAMI_SUCCESS) printf("malloc failed\n");
		rc = __global.heap_mm->memalign((void **)&_result, my_alignment, T_BufSize);
		if (rc != PAMI_SUCCESS) printf("malloc failed\n");

		_generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, __global.mm, NULL);
		_dev = T_MulticombineDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &__global.mm, _generics);
                T_MulticombineDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MulticombineModel(T_MulticombineDevice::Factory::getDevice(_dev, 0, 0), _status);
        }

        ~Multicombine() { }

        inline unsigned long long perform_test(size_t task_id, size_t num_tasks,
                                        pami_context_t ctx, pami_multicombine_t *mcomb) {
                pami_result_t rc;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multicombine \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

				//assert(root != 0); //everybody is the root
                _ipwq.configure(_source, sizeof(_source), sizeof(_source));
                _ipwq.reset();
                _opwq.configure(_result, sizeof(_result), 0);
                _opwq.reset();

                mcomb->cb_done = (pami_callback_t){_done_cb, (void *)this};
                mcomb->data = (pami_pipeworkqueue_t *)&_ipwq;
                mcomb->results = (pami_pipeworkqueue_t *)&_opwq;

                if (mcomb->dtype != PAMI_DOUBLE || mcomb->optor != PAMI_SUM) {
                        fprintf(stderr, "unsupported test case operator/datatype\n");
                        return PAMI_ERROR;
                }
#ifdef DATA_CHECK
                size_t x;
                bool root = (mcomb->results_participants == NULL ||
                        ((PAMI::Topology *)mcomb->results_participants)->isRankMember(task_id));
                for (x = 0; x < mcomb->count; ++x) {
                        ((double *)_source)[x] = 1.0;
                        ((double *)_result)[x] = 0;
                }
#endif
                _done = 0;
                //fprintf(stderr, "... before %s.postMulticombine\n", _name);
				 unsigned long long t1 = PAMI_Wtimebase();

                rc = _model->postMulticombine(_msgbuf, mcomb);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post multicombine \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                //fprintf(stderr, "... before advance loop for %s.postMulticombine\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MulticombineDevice::Factory::advance(_dev, 0, 0);
                }

				 unsigned long long t2 = PAMI_Wtimebase();
#ifdef DATA_CHECK
                for (x = 0; x < mcomb->count; ++x) {
                        if (((double *)_source)[x] != 1.0) {
                                fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
                                break;
                        }
                        if (root) {
                                if (((double *)_result)[x] != num_tasks*1.0) {
                                        fprintf(stderr, "Incorrect result %f at index %zu. stop.\n", ((double *)_result)[x], x);
                                        break;
                                }
                        } else {
                               /* if (((double *)_result)[x] != (double)0) {
                                        fprintf(stderr, "Corrupted result buffer at index %zu. stop.\n", x);
                                        break;
                                }*/
                        }
                }
                if (x < mcomb->count) {
                        return PAMI_ERROR;
                }
#endif
				return (t2-t1);
        }

private:

}; // class Multicombine
}; // namespace Multisend
}; // namespace Test
}; // namespace PAMI

#endif // __pami_test_internals_multisend_multicombine_h__
