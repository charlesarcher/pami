/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDmaMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDmaMessage_h__
#define __components_devices_shmem_ShmemDmaMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include "sys/pami.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "Memregion.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device>
    class ShmemDmaMessage : public ShmemMessage
    {
      public:
        inline ShmemDmaMessage (GenericDeviceMessageQueue *QS,
                                pami_event_function   fn,
                                void               * cookie,
                                T_Device           * device,
                                size_t               fifo,
                                Memregion          * local_memregion,
                                size_t               local_offset,
                                Memregion          * remote_memregion,
                                size_t               remote_offset,
                                size_t               bytes) :
            ShmemMessage (QS, fn, cookie, device->getContextId()),
            _device (device),
            _fifo (fifo),
            _local_memregion (local_memregion),
            _local_offset (local_offset),
            _remote_memregion (remote_memregion),
            _remote_offset (remote_offset),
            _bytes (bytes)
        {};

      protected:

        T_Device           * _device;
        size_t               _fifo;
        Memregion          * _local_memregion;
        size_t               _local_offset;
        Memregion          * _remote_memregion;
        size_t               _remote_offset;
        size_t               _bytes;

    };  // PAMI::Device::ShmemDmaMessage class

    template <class T_Device>
    class ShmemDmaPutMessage : public ShmemDmaMessage<T_Device>
    {
      public:
        inline ShmemDmaPutMessage (pami_event_function   fn,
                                   void               * cookie,
                                   T_Device           * device,
                                   size_t               fifo,
                                   Memregion          * local_memregion,
                                   size_t               local_offset,
                                   Memregion          * remote_memregion,
                                   size_t               remote_offset,
                                   size_t               bytes) :
            ShmemDmaMessage<T_Device> (fn, cookie, device, fifo,
                                       local_memregion, local_offset,
                                       remote_memregion, remote_offset, bytes)
        {};

        DECL_ADVANCE_ROUTINE2(advancePut,ShmemDmaPutMessage,ShmemThread)
        inline pami_result_t __advancePut (pami_context_t context, ShmemThread *thr)
        {
          // These constant-expression branch instructions will be optimized
          // out by the compiler
          if (Memregion::shared_address_read_supported &&
              Memregion::shared_address_write_supported)
            {
              size_t sequence = this->_device->nextInjSequenceId (this->_fifo);
              size_t last_rec_seq_id = this->_device->lastRecSequenceId (this->_fifo);

              if (sequence - 1 <= last_rec_seq_id) //sequence id is carried by a pt-to-pt message before me
                {
                  this->_local_memregion->write (this->_local_offset, this->_remote_memregion, this->_remote_offset, this->_bytes);
                  this->invokeCompletionFunction (context);
                  return PAMI_SUCCESS;
                }
            }
          else
            {
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            }

          return PAMI_EAGAIN;
        };

        inline int setThreads(ShmemThread **th)
        {
                ShmemThread *t;
                int n;
                _device->__getThreads(&t, &n);
                int nt = 0;
                // only one thread... for now...
                t[nt].setMsg(this);
                t[nt].setAdv(advancePut);
                t[nt].setStatus(PAMI::Device::Ready);
                __advancePut(_device->getContext(), t); // was this done by model?
                ++nt;
                *th = t;
                return nt;
        }

        pami_context_t postNext(bool devQueued)
        {
                return _device->__postNext<ShmemDmaPutMessage>__postNext(this, devQueued);
        }

    };  // PAMI::Device::ShmemDmaPutMessage class

    template <class T_Device>
    class ShmemDmaGetMessage : public ShmemDmaMessage<T_Device>
    {
      public:
        inline ShmemDmaGetMessage (pami_event_function   fn,
                                   void               * cookie,
                                   T_Device           * device,
                                   size_t               fifo,
                                   Memregion          * local_memregion,
                                   size_t               local_offset,
                                   Memregion          * remote_memregion,
                                   size_t               remote_offset,
                                   size_t               bytes) :
            ShmemDmaMessage<T_Device> (fn, cookie, device, fifo,
                                       local_memregion, local_offset,
                                       remote_memregion, remote_offset, bytes)
        {};

        DECL_ADVANCE_ROUTINE2(advanceGet,ShmemDmaPutMessage,ShmemThread)
        inline pami_result_t __advanceGet (pami_context_t context, ShmemThread *thr)
        {
          // These constant-expression branch instructions will be optimized
          // out by the compiler
          if (Memregion::shared_address_read_supported &&
              Memregion::shared_address_write_supported)
            {
#warning FIX THESE NEXT TWO LINES
#if 0
              size_t sequence = this->_device->nextInjSequenceId (this->_fifo);
              size_t last_rec_seq_id = this->_device->lastRecSequenceId (this->_fifo);

              if (sequence - 1 <= last_rec_seq_id) //sequence id is carried by a pt-to-pt message before me
#endif
                {
                  this->_local_memregion->read (this->_local_offset, this->_remote_memregion, this->_remote_offset, this->_bytes);
                  this->invokeCompletionFunction (context);
                  return PAMI_SUCCESS;
                }
            }
          else
            {
              PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            }

          return PAMI_EAGAIN;
        };

        inline int setThreads(ShmemThread **th)
        {
                ShmemThread *t;
                int n;
                _device->__getThreads(&t, &n);
                int nt = 0;
                // only one thread... for now...
                t[nt].setMsg(this);
                t[nt].setAdv(advanceGet);
                t[nt].setStatus(PAMI::Device::Ready);
                __advancePut(_device->getContext(), t); // was this done by model?
                ++nt;
                *th = t;
                return nt;
        }

        pami_context_t postNext(bool devQueued)
        {
                return _device->__postNext<ShmemDmaGetMessage>__postNext(this, devQueued);
        }
    };  // PAMI::Device::ShmemDmaGetMessage class
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_shmempacketmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
