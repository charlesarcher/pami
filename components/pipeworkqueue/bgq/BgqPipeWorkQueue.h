///
/// \file components/pipeworkqueue/bgq/BgqPipeWorkQueue.h
/// \brief XMI BG/P specific PipeWorkQueue implementation.
///
#ifndef   __components_pipeworkqueue_bgq_bgqpipeworkqueue_h__
#define   __components_pipeworkqueue_bgq_bgqpipeworkqueue_h__

#include <stdlib.h>
#include <string.h>

#include "components/pipeworkqueue/PipeWorkQueue.h"

#define XMI_PIPEWORKQUEUE_CLASS XMI::PipeWorkQueue::BgqPipeWorkQueue

namespace XMI
{
    namespace PipeWorkQueue
    {
        class BgqPipeWorkQueue : public PipeWorkQueue<XMI::PipeWorkQueue::BgqPipeWorkQueue>
        {
        public:
            inline BgqPipeWorkQueue():
                PipeWorkQueue<XMI::PipeWorkQueue::BgqPipeWorkQueue>()
                {
                }
            inline BgqPipeWorkQueue(XMI::PipeWorkQueue::BgqPipeWorkQueue &obj):
                PipeWorkQueue<XMI::PipeWorkQueue::BgqPipeWorkQueue>(obj)
                {
                }
            inline ~BgqPipeWorkQueue()
                {
                }

            inline void configure_impl(void *sysdep, size_t bufsize)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, size_t bufsize)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, size_t bufsize, size_t bufinit)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, xmi_type_t *type, size_t typecount, size_t typeinit)
                {
                }

            inline void reset_impl()
                {
                }

            inline void dump_impl(const char *prefix = NULL)
                {
                }

            inline xmi_result_t exp_impl(xmi_pipeworkqueue_ext_t *exp)
                {
                  return XMI_UNIMPL;
                }

            inline xmi_result_t import_impl(xmi_pipeworkqueue_ext_t *import)
                {
                  return XMI_UNIMPL;
                }

            inline void setConsumerWakeup_impl(void *vec)
                {
                }

            inline void setProducerWakeup_impl(void *vec)
                {
                }

            inline size_t bytesAvailableToProduce_impl()
                {
                  return 0;
                }

            inline size_t bytesAvailableToConsume_impl()
                {
                  return 0;
                }

            inline size_t getBytesProduced_impl()
                {
                  return 0;
                }

            inline size_t getBytesConsumed_impl()
                {
                  return 0;
                }

            inline char *bufferToProduce_impl()
                {
                  return NULL;
                }

            inline void produceBytes_impl(size_t bytes)
                {
                }

            inline char *bufferToConsume_impl()
                {
                  return NULL;
                }

            inline void consumeBytes_impl(size_t bytes)
                {
                }

            inline bool available_impl()
                {
                  return false;
                }

        }; // end XMI::PipeWorkQueue::BgqPipeWorkQueue
    }; // end namespace PipeWorkQueue
}; // end namespace XMI

#endif // __components_pipeworkqueue_bgq_bgqpipeworkqueue_h__
