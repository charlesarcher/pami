/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemPacketMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemPacketMessage_h__
#define __components_devices_shmem_ShmemPacketMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include "sys/xmi.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Device>
    class ShmemPacketMessage : public ShmemMessage
    {
      public:
        inline ShmemPacketMessage (xmi_event_function   fn,
                                   void               * cookie,
                                   T_Device           * device,
                                   size_t               fifo) :
            ShmemMessage (fn, cookie),
            _device (device),
            _fifo (fifo)
        {};

        inline void setHeader (uint16_t   dispatch_id,
                               void     * metadata,
                               size_t     metasize)
        {
          _dispatch_id = dispatch_id;
          memcpy ((void *)&_metadata, metadata, metasize);
          _metasize = metasize;
        };

        inline void setPayload (void * src, size_t bytes)
        {
          __iov.iov_base = src;
          __iov.iov_len  = bytes;
          _iov  = & __iov;
          _niov = 1;
        };

        inline void setPayload (struct iovec * iov, size_t niov)
        {
          _iov = iov;
          _niov = niov;
        };

        template <unsigned T_Niov>
        inline void setPayload (struct iovec (&iov)[T_Niov])
        {
          _iov  = & iov[0];
          _niov = T_Niov;
        };

        virtual bool advance (xmi_context_t context)
        {
          size_t sequence = 0;

          if (_device->writeSinglePacket (_fifo, _dispatch_id, _metadata, _metasize,
                                          _iov, _niov, sequence) == XMI_SUCCESS)
            {
              invokeCompletionFunction (context);
              return true;
            }

          return false;
        };

      protected:

        T_Device      * _device;
        size_t          _fifo;

        uint16_t        _dispatch_id;
        size_t          _metasize;
        uint8_t         _metadata[T_Device::metadata_size];

        struct iovec  * _iov;
        size_t          _niov;
        struct iovec    __iov;

    };  // XMI::Device::ShmemPacketMessage class

    template <class T_Device>
    class ShmemMultiPacketMessage : public ShmemPacketMessage<T_Device>
    {
      public:
        inline ShmemMultiPacketMessage (xmi_event_function   fn,
                                        void               * cookie,
                                        T_Device           * device,
                                        size_t               fifo) :
            ShmemPacketMessage<T_Device> (fn, cookie, device, fifo)
        {};

        inline void setPayload (void * src, size_t bytes)
        {
          this->__iov.iov_base = src;
          this->__iov.iov_len  = bytes;
        };

        virtual bool advance (xmi_context_t context)
        {
          size_t sequence = 0;
          size_t bytes = MIN(this->__iov.iov_len,T_Device::payload_size);
          TRACE_ERR((stderr, ">> ShmemMultiPacketMessage::advance() .. __iov.iov_len = %zu, T_Device::payload_size = %zu, bytes = %zu\n", this->__iov.iov_len, T_Device::payload_size, bytes));

          while (this->_device->writeSinglePacket (this->_fifo, this->_dispatch_id,
                                                   this->_metadata, this->_metasize,
                                                   this->__iov.iov_base, bytes,
                                                   sequence) == XMI_SUCCESS)
            {
              if (this->__iov.iov_len <= bytes)
                {
                  TRACE_ERR((stderr, "   ShmemMultiPacketMessage::advance() .. before this->invokeCompletionFunction()\n"));
                  this->invokeCompletionFunction (context);
                  TRACE_ERR((stderr, "<< ShmemMultiPacketMessage::advance() .. return true (== \"done\")\n"));
                  return true;
                }
              uint8_t * tmp = (uint8_t *) this->__iov.iov_base;
              this->__iov.iov_base = (void *)(tmp + this->__iov.iov_len);
              this->__iov.iov_len -= bytes;
              TRACE_ERR((stderr, "   ShmemMultiPacketMessage::advance() .. update state, __iov.iov_base = %p, __iov.iov_len = %zu\n", this->__iov.iov_base, this->__iov.iov_len));
            }


          TRACE_ERR((stderr, "<< ShmemMultiPacketMessage::advance() .. return false (== \"not done\")\n"));
          return false;
        };
    };  // XMI::Device::ShmemMultiPacketMessage class
  };    // XMI::Device namespace
};      // XMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemPacketMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
