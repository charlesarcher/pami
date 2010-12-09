/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemMcombMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemMcombMessage_h__
#define __components_devices_shmemcoll_ShmemMcombMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "components/devices/shmemcoll/McombMessage.h"
//#include "quad_sum.h"
#include "16way_sum.h"
#include "assert.h"


#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

#define SHMEMBUF(x)	((double*) _master_desc->get_buffer(x))
#define SHMEMBUF_SHORT(x)	((double*) master_desc->get_buffer(x))

      template <class T_Device, class T_Desc>
      class McombMessageShmem : public McombMessage<T_Device, T_Desc>
      {
        public:
          //currently optimized to a many-to-one combine
          static inline pami_result_t short_msg_advance(T_Desc* master_desc, pami_multicombine_t* mcomb_params,
                                                        unsigned npeers, unsigned local_rank, unsigned task)
          {

            if (local_rank == 0)
              {

                size_t num_src_ranks = ((PAMI::Topology*)mcomb_params->data_participants)->size();

                while (master_desc->arrived_peers() != (unsigned)num_src_ranks) {};

                TRACE_ERR((stderr, "all peers:%zu arrived, starting the blocking Shmem Mcomb protocol\n", num_src_ranks));

                PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params->results;

                size_t bytes = mcomb_params->count << pami_dt_shift[mcomb_params->dtype];

                double* dst = (double*)(rcv->bufferToConsume());

                if (npeers == 4)
                  {

                    for (unsigned i = 0; i < mcomb_params->count; i++)
                      SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i];

                  }
                else if (npeers == 8)
                  {
                    for (unsigned i = 0; i < mcomb_params->count; i++)
                      SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i] +
                                             SHMEMBUF_SHORT(4)[i] + SHMEMBUF_SHORT(5)[i] + SHMEMBUF_SHORT(6)[i] + SHMEMBUF_SHORT(7)[i] ;


                  }
                else if (npeers == 16)
                  {
                    for (unsigned i = 0; i < mcomb_params->count; i++)
                      SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i] +
                                             SHMEMBUF_SHORT(4)[i] + SHMEMBUF_SHORT(5)[i] + SHMEMBUF_SHORT(6)[i] + SHMEMBUF_SHORT(7)[i] +
                                             SHMEMBUF_SHORT(8)[i] + SHMEMBUF_SHORT(9)[i] + SHMEMBUF_SHORT(10)[i] + SHMEMBUF_SHORT(11)[i] +
                                             SHMEMBUF_SHORT(12)[i] + SHMEMBUF_SHORT(13)[i] + SHMEMBUF_SHORT(14)[i] + SHMEMBUF_SHORT(15)[i];

                  }
                else
                  {
                    fprintf(stderr, "sum not yet supported\n");
                  }

                char* src = (char*) master_desc->get_buffer(0);
                char* my_dst = (char*)(dst);
                memcpy((void*)my_dst, (void*)src, bytes);
                rcv->produceBytes(bytes);
                TRACE_ERR((stderr, "Finished gathering results, signalling done\n"));
//				master_desc->signal_flag();
              }

            /* Reduction over...start gathering the results */

#if 0

            if (((PAMI::Topology*)mcomb_params->results_participants)->isRankMember(task))
              {
                while (master_desc->get_flag() == 0) {}; //wait till reduction is done

                char* src = (char*) master_desc->get_buffer(0);

                char* my_dst = (char*)(dst);

                memcpy((void*)my_dst, (void*)src, bytes);

                rcv->produceBytes(bytes);

                TRACE_ERR((stderr, "Finished gathering results, signalling done\n"));
              }

            master_desc->signal_done();

            while (master_desc->in_use()) {}; //wait for everyone to signal done

#endif
            return PAMI_SUCCESS;
          }

        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            McombMessageShmem * msg = (McombMessageShmem *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {

            TRACE_ERR((stderr, "in Shmem Mcomb advance\n"));
            T_Desc* _my_desc = this->_my_desc;
            T_Desc* _master_desc = this->_master_desc;
            unsigned _npeers = this->_npeers;
            unsigned _local_rank = this->_local_rank;
            unsigned _task = this->_task;

            pami_multicombine_t & mcomb_params = _my_desc->get_mcomb_params();
            size_t num_src_ranks = ((PAMI::Topology*)mcomb_params.data_participants)->size();

            /* Non blocking until all the peers arrive at the collective */

            //assert(_my_desc != NULL);
            //assert(_master_desc != NULL);
            /*if (_master_desc->arrived_peers() != (unsigned) num_src_ranks)
            {
            	 TRACE_ERR((stderr,"arrived_peers:%u waiting for:%u\n", _master_desc->arrived_peers(), (unsigned) num_src_ranks));
            	 return PAMI_EAGAIN;
            }*/
            while (_master_desc->arrived_peers() != (unsigned)num_src_ranks) {};

            TRACE_ERR((stderr, "all peers:%zu arrived, starting the blocking Shmem Mcomb protocol\n", num_src_ranks));

            /* Start the protocol here..blocking version since everyone arrived */

            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params.results;

            size_t bytes = mcomb_params.count << pami_dt_shift[mcomb_params.dtype];

            double* dst = (double*)(rcv->bufferToConsume());

            if (_local_rank == 0)
              {
                if (_npeers == 4)
                  {

                    for (unsigned i = 0; i < mcomb_params.count; i++)
                      SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i];

                  }
                else if (_npeers == 8)
                  {
                    for (unsigned i = 0; i < mcomb_params.count; i++)
                      SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i] +
                                       SHMEMBUF(4)[i] + SHMEMBUF(5)[i] + SHMEMBUF(6)[i] + SHMEMBUF(7)[i] ;


                  }
                else if (_npeers == 16)
                  {
                    for (unsigned i = 0; i < mcomb_params.count; i++)
                      SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i] +
                                       SHMEMBUF(4)[i] + SHMEMBUF(5)[i] + SHMEMBUF(6)[i] + SHMEMBUF(7)[i] +
                                       SHMEMBUF(8)[i] + SHMEMBUF(9)[i] + SHMEMBUF(10)[i] + SHMEMBUF(11)[i] +
                                       SHMEMBUF(12)[i] + SHMEMBUF(13)[i] + SHMEMBUF(14)[i] + SHMEMBUF(15)[i];

                  }
                else
                  {
                    fprintf(stderr, "sum not yet supported\n");
                  }

                _master_desc->signal_flag();
              }

            /* Reduction over...start gathering the results */

            if (((PAMI::Topology*)mcomb_params.results_participants)->isRankMember(_task))
              {
                while (_master_desc->get_flag() == 0) {}; //wait till reduction is done

                char* src = (char*) _master_desc->get_buffer(0);

                char* my_dst = (char*)(dst);

                memcpy((void*)my_dst, (void*)src, bytes);

                rcv->produceBytes(bytes);

                TRACE_ERR((stderr, "Finished gathering results, signalling done\n"));
              }

            _master_desc->signal_done();

            while (_master_desc->in_use()) {}; //wait for everyone to signal done

            this->setStatus (PAMI::Device::Done);

            return PAMI_SUCCESS;

          }


        public:
          inline McombMessageShmem (T_Device *device, T_Desc* desc, T_Desc* matched_desc) :
              McombMessage<T_Device, T_Desc> (device, desc, matched_desc, McombMessageShmem::__advance, this)
          {
            TRACE_ERR((stderr, "<> McombMessageShmem::McombMessageShmem()\n"));
          };



      };  // PAMI::Device::McombMessageShmem class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcombMessageShmem_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
