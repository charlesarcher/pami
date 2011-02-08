///
/// \file components/devices/bgq/mu2/msg/CollectiveDputMcomb2Device.h
/// \brief ???
///
#ifndef __components_devices_bgq_mu2_msg_CollectiveDputMcomb2Device_h__
#define __components_devices_bgq_mu2_msg_CollectiveDputMcomb2Device_h__

#include "spi/include/mu/DescriptorBaseXX.h"
#include "util/trace.h"
#include "components/devices/bgq/mu2/InjChannel.h"
#include "components/devices/bgq/mu2/msg/MessageQueue.h"
#include "common/bgq/Mapping.h"
#include "components/memory/MemoryAllocator.h"
#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#include "components/devices/shmemcoll/msgs/ShortMcombMessage.h"
#include "components/devices/shmemcoll/msgs/CNShmemMessage.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/indirect/IndirectCounter.h"
#include "math/a2qpx/Core_memcpy.h"


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
    typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Counter::BGQ::IndirectL2> ShmemCollDesc;
    typedef PAMI::Device::ShmemCollDevice <PAMI::Counter::BGQ::IndirectL2> ShmemCollDevice;
    //typedef PAMI::Device::Shmem::ShmemCollDesc <Counter::Indirect<Counter::Native> > ShmemCollDesc;
    //typedef PAMI::Device::ShmemCollDevice <Counter::Indirect<Counter::Native> > ShmemCollDevice;
      ///
      /// \brief Inject one or more collective descriptors into an
      /// injection fifo and wait for the collective to complete. This
      /// code currently only works with linear PipeWorkQueues.
      ///
      class CollectiveDputMcomb2Device 
      {
        public:

          ///Default dummy constructor
          CollectiveDputMcomb2Device (): _context(*(MU::Context*)NULL),_shmsg() {}

          ///
          /// \brief Inject descriptor(s) into a specific injection fifo
          ///
          /// \param[in] context the MU context for this message
          /// \param[in] fn  completion event fn
          /// \param[in] cookie callback cookie
          /// \param[in] spwq pipeworkqueue that has data to be consumed
          /// \param[in] dpwq pipeworkqueue where data has to be produced
          /// \param[in] length the totaly number of bytes to be transfered
          /// \param[in] op operation identifier
          /// \param[in] sizeoftype the size of the datatype
          /// \param[in] counterAddress address of the counter
          ///
          CollectiveDputMcomb2Device (MU::Context         & context,
                                      pami_event_function   fn,
                                      void                * cookie,
                                      PipeWorkQueue       * spwq,
                                      PipeWorkQueue       * dpwq,
                                      uint32_t              length,
                                      uint32_t              op,
                                      uint32_t              sizeoftype,
                                      volatile uint64_t   * counterAddress,
                                      ShmemCollDesc       *shmem_desc):
              _context (context),
              _injectedBytes (0),
              _length (length),
              _spwq (spwq),
              _dpwq (dpwq),
              _fn (fn),
              _cookie (cookie),
              _op (op),
              _sizeoftype(sizeoftype),
              _doneMULarge (false),
              _doneMUShort (false),
              _cc (length),
              _counterAddress (counterAddress),
              _shmem_desc(shmem_desc),
              _doneShmemMcomb(0),
              _doneShmemMcombLarge(0),
              _doneShmemMcastLarge(0),
              _shmsg(shmem_desc,  length),
              _combineDone(false)
//              _phase(0)
          {
            //TRACE_FN_ENTER();
            //TRACE_FN_EXIT();
          };

          inline ~CollectiveDputMcomb2Device () {};

          void init(void* counter_addr_gva)
          {
            const uint64_t 	alignment = 64;
            uint64_t	mask	= 0;
            mask = (alignment - 1);

            char * rcvbuf = NULL, * srcbuf = NULL;
            uint64_t rcvbuf_phy=0, srcbuf_phy=0;
            rcvbuf = (char*)_dpwq->bufferToProduce(); 
            PAMI_assert (rcvbuf != NULL);
            PAMI_assert(((uint64_t)rcvbuf & (uint64_t)mask) == 0);
            srcbuf = (char*)_spwq->bufferToConsume();
            PAMI_assert (srcbuf != NULL);
            PAMI_assert(((uint64_t)srcbuf & (uint64_t)mask) == 0);

            Kernel_MemoryRegion_t memRegion;
            uint32_t rc;

            rc = Kernel_CreateMemoryRegion (&memRegion, _shmem_desc->get_buffer(), ShmBufSize);
            PAMI_assert ( rc == 0 );
            _shmbuf_phy = (uint64_t)memRegion.BasePa +
              ((uint64_t)_shmem_desc->get_buffer() - (uint64_t)memRegion.BaseVa);

            if (_length < VERY_SHORT_MSG_CUTOFF)
            {
              if (_length <= 32)
              {
                char* buf = (char*) _shmem_desc->get_buffer();
                memcpy((void*)&buf[_length*__global.mapping.t()], srcbuf, _length);
              }
              else
              {
                void* buf = _shmem_desc->get_buffer(__global.mapping.t());
                memcpy(buf, srcbuf, _length);
              }
            }
            else
            {
              rc = Kernel_CreateMemoryRegion (&memRegion, rcvbuf, _length);
              PAMI_assert ( rc == 0 );
              rcvbuf_phy = (uint64_t)memRegion.BasePa +
                ((uint64_t)rcvbuf - (uint64_t)memRegion.BaseVa);
              void *rcvbuf_gva = NULL;
              rc = Kernel_Physical2GlobalVirtual ((void*)rcvbuf_phy, &rcvbuf_gva);


              rc = Kernel_CreateMemoryRegion (&memRegion, srcbuf, _length);
              PAMI_assert ( rc == 0 );
              srcbuf_phy = (uint64_t)memRegion.BasePa +
                ((uint64_t)srcbuf - (uint64_t)memRegion.BaseVa);
              void *srcbuf_gva = NULL;
              rc = Kernel_Physical2GlobalVirtual ((void*)srcbuf_phy, &srcbuf_gva);

              _shmsg.init((void*)srcbuf, (void*)rcvbuf, (void*)srcbuf_gva, (void*)rcvbuf_gva, (void*)rcvbuf_phy, (void*)_shmbuf_phy, __global.mapping.t());
            }

            if (__global.mapping.t() == 0)
            {
              _desc.setOpCode (_op);
              _desc.setWordLength (_sizeoftype);
              if (_length <= SHORT_MSG_CUTOFF) 
                _context.setThroughputCollectiveBufferBatEntry((uint64_t)_shmbuf_phy);
              else
                _context.setThroughputCollectiveBufferBatEntry((uint64_t)rcvbuf_phy);
            }
            mem_sync();
            _shmem_desc->signal_arrived(); //signal that I have copied all my addresses/data
          }

          bool advanceMUShort (void* next_inj_buf, uint64_t bytes_available)
          {
            if (_doneMUShort)
              return true;

            if (next_inj_buf == NULL)
              return false;

            _desc.Pa_Payload  = (uint64_t)next_inj_buf;
            _desc.Message_Length = bytes_available;

            size_t                fnum    = 0;
            InjChannel & channel = _context.injectionGroup.channel[fnum];
            size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();
            //printf("injecting short bytes:%lu counter:%lu\n", bytes_available, *_counterAddress);
            //fflush(stdout);
            if (ndesc > 0)
            {
              MUSPI_DescriptorBase * d = (MUSPI_DescriptorBase *) channel.getNextDescriptor ();
              _desc.clone (*d);
              channel.injFifoAdvanceDesc ();
              _doneMUShort =  true;
            }
            else
              return false;

            _injectedBytes  += bytes_available;
            _desc.setRecPutOffset(_injectedBytes);

            return true;
          }

          bool advanceMULarge (void* next_inj_buf, uint64_t bytes_available, uint64_t offset_b = 0)
          {
            //TRACE_FN_ENTER();

            if (_doneMULarge)
              return true;

            if (next_inj_buf == NULL)
              return false;

            _desc.Pa_Payload  = (uint64_t)next_inj_buf + offset_b;
            _desc.Message_Length = bytes_available;

            size_t                fnum    = 0;
            InjChannel & channel = _context.injectionGroup.channel[fnum];
            size_t ndesc = channel.getFreeDescriptorCountWithUpdate ();

            if (ndesc > 0)
            {
              MUSPI_DescriptorBase * d = (MUSPI_DescriptorBase *) channel.getNextDescriptor ();
              _desc.clone (*d);
              
              channel.injFifoAdvanceDesc ();
              _shmsg.injection_complete();
            }
            else
              return false;

            _injectedBytes  += bytes_available;
            _desc.setRecPutOffset(_injectedBytes);

            _doneMULarge = (_injectedBytes == _length);

            return _doneMULarge;
          }


          bool advanceShmemMcombShort(unsigned length, bool &combineDone)
          {
            if (_doneShmemMcomb) return true;

            pami_result_t res;

            if (length < VERY_SHORT_MSG_CUTOFF)
              res = PAMI::Device::Shmem::CNShmemMessage<ShmemCollDesc>::very_short_msg_combine(_shmem_desc, length, __global.mapping.tSize(), __global.mapping.t(), combineDone);
            else
              res = _shmsg.short_msg_combine(length, __global.mapping.tSize(), __global.mapping.t(), combineDone);

            if (res == PAMI_SUCCESS)
            {
              _doneShmemMcomb = 1;
              return true;
            }

            return false;
          }

          bool advanceShmemMcastShort()
          {

            pami_result_t res;
            if (_length < VERY_SHORT_MSG_CUTOFF)
              res = PAMI::Device::Shmem::CNShmemMessage<ShmemCollDesc>::very_short_msg_multicast(_shmem_desc, _dpwq, _length, 
                  __global.mapping.tSize(), __global.mapping.t(), (uint64_t*)_counterAddress, _cc);
            else
              res = _shmsg.short_msg_multicast(_length, __global.mapping.tSize(), __global.mapping.t(),
                  (uint64_t*)_counterAddress, _cc);

            if (res == PAMI_SUCCESS)
            {
              if (_fn)
                _fn (NULL, _cookie, PAMI_SUCCESS);

              _dpwq->produceBytes(_length);
              return true;
            }

            return false;
          }

          bool advance()
          {
            bool flag;

            flag = false;
            void* next_inj_buf = NULL;

            register  uint64_t  count = 0;
            while ((flag == false) && (++count < 1e5)){
              if (__global.mapping.t() == 0)
              {
                flag = advanceShmemMcombShort(_length, _combineDone);
                if (_combineDone)
                  next_inj_buf = (void*)_shmbuf_phy;
                flag = flag && advanceMUShort(next_inj_buf, _length);
                flag = flag && advanceShmemMcastShort();
              }
              else
              {
                flag =  advanceShmemMcombShort(_length, _combineDone);
                flag = flag && advanceShmemMcastShort();
              }

            }
            return flag;
          }

          bool advanceShmemMcombLarge(unsigned length, unsigned offset_dbl=0)
          {
            if (_doneShmemMcombLarge) return true;

            pami_result_t res;

            res = _shmsg.large_msg_combine_peers(length, __global.mapping.tSize(), __global.mapping.t(), offset_dbl);
            if (res == PAMI_SUCCESS)
            {
              _doneShmemMcombLarge = 1;
              return true;
            }

            return false;
          }

          bool advanceShmemMcastLarge()
          {

            if (_doneShmemMcastLarge) return true;

            pami_result_t res;

            res = _shmsg.large_msg_multicast(_length, __global.mapping.tSize(), __global.mapping.t(), _counterAddress, _cc);
            if (res == PAMI_SUCCESS)
            {
              if (_fn)
                _fn (NULL, _cookie, PAMI_SUCCESS);

              _dpwq->produceBytes(_length);
              _doneShmemMcastLarge = 1;
              return true;
            }

            return false;
          }

          bool advance_large()
          {
            bool flag;

            flag = false;
            uint64_t  bytes_available =0;
            register  uint64_t  count = 0;

            while ((flag == false) && (++count < 1e5)){
              if (__global.mapping.t() == 0)
              {
                void* next_inj_buf = _shmsg.next_injection_buffer(&bytes_available, _length, __global.mapping.tSize());
                flag = advanceMULarge(next_inj_buf, bytes_available);
                flag = flag && advanceShmemMcastLarge();
              }
              else
              {
                flag =  advanceShmemMcombLarge(_length);
                flag = flag && advanceShmemMcastLarge();
              }

            }
            return flag;
          }

          bool advance_large_hybrid()
          {
            bool flag;
            //static bool combineDone = false;
            uint64_t  bytes_available =0;
            void* next_inj_buf = NULL;
            register  uint64_t  count = 0;
            flag = false;

            while ((flag == false) && (++count < 1e5))
            {
              next_inj_buf = NULL;

              if (__global.mapping.t() == 0)
              {
                if (_phase == 0)
                {
                  flag = advanceShmemMcombShort(SHORT_MSG_CUTOFF, _combineDone);
                  if (_combineDone)
                    next_inj_buf = (void*)_shmbuf_phy;
                  flag = flag && advanceMUShort(next_inj_buf, SHORT_MSG_CUTOFF);
                  
                  if (flag)
                    _phase++;
                }
                else
                {
                  next_inj_buf = _shmsg.next_injection_buffer(&bytes_available, _length-SHORT_MSG_CUTOFF, __global.mapping.tSize());
                  flag = advanceMULarge(next_inj_buf, bytes_available, SHORT_MSG_CUTOFF);
                }
              }
              else
              {
                if (_phase == 0)
                {
                  flag =  advanceShmemMcombShort(SHORT_MSG_CUTOFF, _combineDone);
                  if (flag)
                    _phase++;
                }
                else
                {
                  flag =  advanceShmemMcombLarge(_length-SHORT_MSG_CUTOFF, SHORT_MSG_CUTOFF/sizeof(double));
                }
              }
                
              flag = flag && advanceShmemMcastLarge();
            }
            return flag;

          }

          MUSPI_DescriptorBase     _desc; //The descriptor is setup externally and contains batids, sndbuffer base and msg length

        protected:

          MU::Context            & _context;
          uint32_t                 _injectedBytes;
          uint32_t                 _length;        //Number of bytes to transfer
          PipeWorkQueue          * _spwq;
          PipeWorkQueue          * _dpwq;
          pami_event_function      _fn;
          void                   * _cookie;
          uint32_t                 _op;
          uint32_t                 _sizeoftype;
          bool                     _doneMULarge;
          bool                     _doneMUShort;
          //bool                     _isRoot;
          uint64_t                 _cc;
          volatile uint64_t      * _counterAddress;

          ShmemCollDesc          * _shmem_desc;
          bool                     _doneShmemMcomb;
          bool                     _doneShmemMcombLarge;
          bool                     _doneShmemMcastLarge;
          Shmem::CNShmemMessage<ShmemCollDesc>   _shmsg;
          bool                      _combineDone;
          uint64_t                _shmbuf_phy;
          uint8_t                 _phase;

      }; // class     PAMI::Device::MU::CollectiveDputMcomb2Device
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#endif // __components_devices_bgq_mu2_msg_CollectiveDputMcomb2Device_h__                     
