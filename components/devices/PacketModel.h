/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketModel.h
 * \brief ???
 */

#ifndef __components_devices_packetmodel_h__
#define __components_devices_packetmodel_h__

#include <sys/uio.h>

#include "sys/xmi.h"

#include "components/devices/PacketDevice.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Packet model template class
      /// \param T_Device  Packet device template class
      /// \param T_Object  Packet object template class
      ///
      /// \see Packet::Model
      /// \see Packet::Device
      ///
      template <class T_Model, class T_Device, class T_Object>
      class PacketModel
      {
        public:

          /// \brief Packet model interface constructor
          ///
          /// \param[in] device  Packet device reference
          /// \param[in] context Communication context
          ///
          PacketModel (T_Device & device, xmi_context_t context) {};

          ~PacketModel () {};

          ///
          /// \brief Returns the deterministic network attribute of this model
          ///
          /// A deterministic network "routes" all communication in a fixed,
          /// deterministic, way such that messages are received by the remote
          /// task in the same order as the messages were sent by the local
          /// task.
          ///
          /// \attention All device derived classes \b must contain a static
          ///            const data member named 'bool deterministic'.
          ///
          static const bool isDeterministic ();

          ///
          /// \brief Base packet model initializer
          ///
          /// The packet device implementation will use the appropriate receive
          /// function depending on the packet device "requires read" attribute.
          ///
          /// \see XMI::Device::Interface::PacketDevice::requiresRead()
          /// \see XMI::Device::Interface::PacketDevice::readData()
          ///
          /// \param[in] dispatch              Dispatch set identifier
          /// \param[in] direct_recv_func      Receive function for direct-access packet devices
          /// \param[in] direct_recv_func_parm Receive function clientdata for direct-access packet devices
          /// \param[in] read_recv_func        Receive function for read-access packet devices
          /// \param[in] read_recv_func_parm   Receive function clientdata for read-access packet devices
          ///
          xmi_result_t init (size_t           dispatch,
                             RecvFunction_t   direct_recv_func,
                             void           * direct_recv_func_parm,
                             RecvFunction_t   read_recv_func,
                             void           * read_recv_func_parm);

          ///
          /// \brief Post a single packet contigous transfer operation
          ///
          /// The post packet immediate interface allows the packet model
          /// and packet device implementations to optimize for performance by
          /// avoiding the overhead to construct a packet message object. If
          /// the packet device has resources immediately available then the
          /// packet may be directly posted, otherwise a packet message object
          /// may be constructed at the location specified to maintain the
          /// operation state until the completion callback is invoked.
          ///
          /// \note The packet device will only transfer a single packet. The
          ///       payload data will be truncated if more bytes than a single
          ///       packet can transfer is specified as the payload.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketPayloadSize()
          ///
          /// \note The size of the metadata to be copied into the packet
          ///       is an attribute of the specific packet device associated
          ///       with this packet model.
          ///
          /// \see XMI::Device::Interface::PacketDevice::getPacketMetadataSize()
          ///
          /// \param[in] obj          Location to store the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload      Virtual address of source buffer
          /// \param[in] bytes        Number of bytes to transfer
          ///
          /// \retval true  Packet operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Packet operation did not complete and the packet
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postPacket (T_Object           * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  void               * payload,
                                  size_t               bytes);

          ///
          /// \brief Post a single packet non-contiguous transfer operation
          ///
          /// \param[in] obj          Location to store the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload0     Virtual address of the first source buffer
          /// \param[in] bytes0       Number of bytes to transfer from the first buffer
          /// \param[in] payload1     Virtual address of the second source buffer
          /// \param[in] bytes1       Number of bytes to transfer from the second buffer
          ///
          inline bool postPacket (T_Object           * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  void               * payload0,
                                  size_t               bytes0,
                                  void               * payload1,
                                  size_t               bytes1);

          ///
          /// \brief Post a single packet non-contiguous transfer operation
          ///
          /// \param[in] obj          Location to store the packet transfer state
          /// \param[in] cb           Callback to invoke when the operation completes
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload      Virtual address of source buffer
          /// \param[in] bytes        Number of bytes to transfer
          ///
          inline bool postPacket (T_Object           * obj,
                                  xmi_event_function   fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  void               * metadata,
                                  size_t               metasize,
                                  struct iovec       * iov,
                                  size_t               niov);

          ///
          /// \brief Immediate post of a single packet non-contiguous transfer operation
          ///
          /// Returns false if resources are not immediately available to post the operation
          ///
          /// \param[in] target_rank  Global rank of the packet destination process
          /// \param[in] metadata     Virtual address of metadata buffer
          /// \param[in] metasize     Number of metadata bytes
          /// \param[in] payload0     Virtual address of the first source buffer
          /// \param[in] bytes0       Number of bytes to transfer from the first buffer
          /// \param[in] payload1     Virtual address of the second source buffer
          /// \param[in] bytes1       Number of bytes to transfer from the second buffer
          ///
          /// \retval true  All data has been immediately sent
          /// \retval false Unable to send data with post immediate
          ///
          inline bool postPacketImmediate (size_t   target_rank,
                                           void   * metadata,
                                           size_t   metasize,
                                           void   * payload0,
                                           size_t   bytes0,
                                           void   * payload1,
                                           size_t   bytes1);
      };

      template <class T_Model, class T_Device, class T_Object>
      const bool PacketModel<T_Model, T_Device, T_Object>::isDeterministic ()
      {
        return T_Model::deterministic;
      }

      template <class T_Model, class T_Device, class T_Object>
      xmi_result_t PacketModel<T_Model, T_Device, T_Object>::init (size_t           dispatch,
                                                                   RecvFunction_t   direct_recv_func,
                                                                   void           * direct_recv_func_parm,
                                                                   RecvFunction_t   read_recv_func,
                                                                   void           * read_recv_func_parm)
      {
        return static_cast<T_Model*>(this)->init_impl (dispatch,
                                                       direct_recv_func,
                                                       direct_recv_func_parm,
                                                       read_recv_func,
                                                       read_recv_func_parm);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                        xmi_event_function   fn,
                                                                        void               * cookie,
                                                                        size_t            target_rank,
                                                                        void            * metadata,
                                                                        size_t            metasize,
                                                                        void            * payload,
                                                                        size_t            bytes)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize, payload, bytes);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                        xmi_event_function   fn,
                                                                        void               * cookie,
                                                                        size_t            target_rank,
                                                                        void            * metadata,
                                                                        size_t            metasize,
                                                                        void            * payload0,
                                                                        size_t            bytes0,
                                                                        void            * payload1,
                                                                        size_t            bytes1)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize,
                                                             payload0, bytes0,
                                                             payload1, bytes1);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacket (T_Object        * obj,
                                                                        xmi_event_function   fn,
                                                                        void               * cookie,
                                                                        size_t            target_rank,
                                                                        void            * metadata,
                                                                        size_t            metasize,
                                                                        struct iovec    * iov,
                                                                        size_t            niov)
      {
        return static_cast<T_Model*>(this)->postPacket_impl (obj, fn, cookie, target_rank,
                                                             metadata, metasize, iov, niov);
      }

      template <class T_Model, class T_Device, class T_Object>
      inline bool PacketModel<T_Model, T_Device, T_Object>::postPacketImmediate (size_t   target_rank,
                                                                                 void   * metadata,
                                                                                 size_t   metasize,
                                                                                 void   * payload0,
                                                                                 size_t   bytes0,
                                                                                 void   * payload1,
                                                                                 size_t   bytes1)
      {
        return static_cast<T_Model*>(this)->postPacketImmediate_impl (target_rank,
                                                                      metadata, metasize,
                                                                      payload0, bytes0,
                                                                      payload1, bytes1);
      }
    };
  };
};
#endif // __components_devices_packetmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
