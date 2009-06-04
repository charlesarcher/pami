#ifndef  __ccmi_adaptor_executor_pool__
#define  __ccmi_adaptor_executor_pool__

#include "collectives/util/queue/Queue.h"
#include "collectives/algorithms/protocols/broadcast/BcastQueueElem.h"

#define MAX_PREALLOCATED_BYTES      256
#define MAX_NUM_PREALLOCATED        64


namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      ///
      /// \brief This class implements utilites like a buffer pool and
      /// constructs async broadcast queue elements
      ///
      class ExecutorPool
      {

      protected:

        ///
        /// \brief A buffer pool to optimize memory allocation
        /// overheads
        ///
        CCMI::Queue                      _bufferPool;

      public:

        ///
        /// \brief Constructor
        ///
        ExecutorPool ()
        {
        }

        ///
        /// \brief Allocate an async buffer for the incoming
        ///  message and the executor associated with that
        ///  broadcast. For short messages we keep a pool of 64 buffers
        ///  to optimize malloc/free overheads
        ///
        void allocateAsync (CCMI_Executor_t  ** exec,
                            char             ** rcvbuf,
                            unsigned            bytes) 
        {
          if(bytes < MAX_PREALLOCATED_BYTES &&
             ! _bufferPool.isEmpty())
          {
            BcastQueueElem *bqe = (BcastQueueElem *)_bufferPool.popHead();
            * exec = (CCMI_Executor_t *)bqe->composite();
            * rcvbuf = (char *) (*exec) + sizeof(CCMI_Executor_t);

            return;
          }

          int max_bytes = ((bytes < MAX_PREALLOCATED_BYTES)?
                           (MAX_PREALLOCATED_BYTES):(bytes));

          void *buf = CCMI_Alloc (sizeof(CCMI_Executor_t) + max_bytes);
          * exec   = (CCMI_Executor_t *) buf;
          * rcvbuf = (char *)buf + sizeof(CCMI_Executor_t);
        }

        ///
        /// \brief Free the buffer allocated from
        /// allocateAsync. Enqueue to pool if size is less than
        /// MAX_PREALLOCATED_BYTES
        ///
        void freeAsync (BcastQueueElem *elem, unsigned bytes) 
        {
          if(bytes < MAX_PREALLOCATED_BYTES &&
             _bufferPool.size() < MAX_NUM_PREALLOCATED)
            _bufferPool.pushHead (elem);
          else
            CCMI_Free (elem->composite());
        }

      };  //- Executor Pool
    };  //- Broadcast
  };  //- Adaptor
};  //- CCMI


#endif
