/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpipacketmodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpipacketmodel_h__
#define __components_devices_mpi_mpipacketmodel_h__

#define USE_GCC_ICE_WORKAROUND

#include "sys/xmi.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/mpi/mpimessage.h"
#include "errno.h"
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
#ifdef USE_GCC_ICE_WORKAROUND
    class MPIPacketModel : public Interface::PacketModel<MPIPacketModel<T_Device, T_Message>, T_Device, 512>
#else // USE_GCC_ICE_WORKAROUND
    class MPIPacketModel : public Interface::PacketModel<MPIPacketModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
#endif // USE_GCC_ICE_WORKAROUND
    {
    public:
      MPIPacketModel (T_Device & device) :
#ifdef USE_GCC_ICE_WORKAROUND
        Interface::PacketModel < MPIPacketModel<T_Device, T_Message>, T_Device, 512 > (device),
#else // USE_GCC_ICE_WORKAROUND
        Interface::PacketModel < MPIPacketModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device),
#endif // USE_GCC_ICE_WORKAROUND
        _device (device),
//        _client(client),
        _context (device.getContext()),
        _contextid (device.getContextOffset())
        {};

#ifdef EMULATE_NONDETERMINISTIC_DEVICE
      static const bool   deterministic_packet_model   = false;
      static const bool   deterministic_message_model  = false;
#else
      static const bool   deterministic_packet_model   = true;
      static const bool   deterministic_message_model  = true;
#endif
#ifdef EMULATE_UNRELIABLE_DEVICE
      static const bool   reliable_packet_model        = false;
      static const bool   reliable_message_model       = false;
#else
      static const bool   reliable_packet_model        = true;
      static const bool   reliable_message_model       = true;
#endif

      static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
      static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
      static const size_t packet_model_payload_bytes        = T_Device::payload_size;
#ifdef USE_GCC_ICE_WORKAROUND
      static const size_t packet_model_state_bytes          = 512;
#else // USE_GCC_ICE_WORKAROUND
      static const size_t packet_model_state_bytes          = sizeof(T_Message);
#endif // USE_GCC_ICE_WORKAROUND

      xmi_result_t init_impl (size_t                      dispatch,
                              Interface::RecvFunction_t   direct_recv_func,
                              void                      * direct_recv_func_parm,
                              Interface::RecvFunction_t   read_recv_func,
                              void                      * read_recv_func_parm)
        {
#ifdef USE_GCC_ICE_WORKAROUND
          COMPILE_TIME_ASSERT(sizeof(T_Message) <= 512);
#endif // USE_GCC_ICE_WORKAROUND
          _dispatch_id = _device.registerRecvFunction (dispatch, direct_recv_func, direct_recv_func_parm);
         TRACE_DEVICE((stderr,"<%p>MPIModel::init_impl %d \n",this, _dispatch_id));
         return XMI_SUCCESS;
        };

      inline bool postPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   xmi_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec_t     * iov,
                                   size_t               niov)
        {
          XMI_abort();
          return false;
        };

      template <unsigned T_Niov>
      inline bool postPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   xmi_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[T_Niov])
        {
          XMI_abort();
          return false;
        };


      inline bool postPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   xmi_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[1])
        {
          int rc;
          MPIMessage * msg = (MPIMessage *)state;
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl %d \n",this, this->_dispatch_id));
#ifdef EMULATE_UNRELIABLE_DEVICE
          unsigned long long t = __global.time.timebase ();
          if (t % EMULATE_UNRELIABLE_DEVICE_FREQUENCY == 0) return true;
#endif
          new(msg)MPIMessage(this->_client, this->_context,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=iov[0].iov_len;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], iov[0].iov_base, iov[0].iov_len);
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl MPI_Isend %zd to %zd\n",this,
                         sizeof(msg->_p2p_msg),target_task));
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
          msg->_target_task = (xmi_task_t) target_task;
          _device.addToNonDeterministicQueue (msg,__global.time.timebase());
#else
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg),
                          MPI_CHAR,
                          target_task,
                          0,
                          _device._communicator,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);
#endif
          return true;
        };

      inline bool postPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   xmi_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   struct iovec         (&iov)[2])
        {
          int rc;
//          void       * obj = malloc(sizeof(MPIMessage));
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket %d \n",this, this->_dispatch_id));
#ifdef EMULATE_UNRELIABLE_DEVICE
          unsigned long long t = __global.time.timebase ();
          if (t % EMULATE_UNRELIABLE_DEVICE_FREQUENCY == 0) return true;
#endif
          MPIMessage * msg = (MPIMessage *)state;
          new(msg)MPIMessage(this->_client, this->_contextid,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=iov[0].iov_len;
          msg->_p2p_msg._payloadsize1=iov[1].iov_len;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], iov[0].iov_base, iov[0].iov_len);
          memcpy(&msg->_p2p_msg._payload[iov[0].iov_len], iov[1].iov_base, iov[1].iov_len);
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket MPI_Isend %zd to %zd\n",this,
                         sizeof(msg->_p2p_msg),target_task));
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
          msg->_target_task = (xmi_task_t) target_task;
          _device.addToNonDeterministicQueue (msg,__global.time.timebase());
#else
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg),
                          MPI_CHAR,
                          target_task,
                          0,
                          _device._communicator,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);
#endif
          return true;
        };

      inline bool postPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                   xmi_event_function   fn,
                                   void               * cookie,
                                   xmi_task_t           target_task,
                                   size_t               target_offset,
                                   void               * metadata,
                                   size_t               metasize,
                                   void               * payload,
                                   size_t               length)
        {
          int rc;
          MPIMessage * msg = (MPIMessage *)state;
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl %d \n",this, this->_dispatch_id));
#ifdef EMULATE_UNRELIABLE_DEVICE
          unsigned long long t = __global.time.timebase ();
          if (t % EMULATE_UNRELIABLE_DEVICE_FREQUENCY == 0) return true;
#endif
          new(msg)MPIMessage(this->_client, this->_contextid,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=0;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=length;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], payload, length);
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl MPI_Isend %zd to %zd\n",this,
                         sizeof(msg->_p2p_msg),target_task));
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
          msg->_target_task = (xmi_task_t) target_task;
          _device.addToNonDeterministicQueue (msg,__global.time.timebase());
#else
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg),
                          MPI_CHAR,
                          target_task,
                          0,
                          _device._communicator,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);
#endif
          return true;
        };



      template <unsigned T_Niov>
      inline bool postPacket_impl (xmi_task_t     target_task,
                                   size_t         target_offset,
                                   void         * metadata,
                                   size_t         metasize,
                                   struct iovec   (&iov)[T_Niov])
        {
          XMI_assert(T_Niov<=2);

          int rc;
          void       * obj = malloc(sizeof(MPIMessage));
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl %d \n",this, this->_dispatch_id));
#ifdef EMULATE_UNRELIABLE_DEVICE
          unsigned long long t = __global.time.timebase ();
          if (t % EMULATE_UNRELIABLE_DEVICE_FREQUENCY == 0) return true;
#endif
          MPIMessage * msg = (MPIMessage *)obj;
          new(msg)MPIMessage(this->_client,this->_contextid,
                             this->_dispatch_id,
                             NULL,
                             0);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=iov[0].iov_len;
          if (T_Niov == 2)
            msg->_p2p_msg._payloadsize1=iov[1].iov_len;
          else
            msg->_p2p_msg._payloadsize1=0;

          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy(&msg->_p2p_msg._payload[0], iov[0].iov_base, iov[0].iov_len);
          if (T_Niov)
            memcpy(&msg->_p2p_msg._payload[iov[0].iov_len], iov[1].iov_base, iov[1].iov_len);
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postPacket_impl MPI_Isend %zd to %zd\n",this,
                         sizeof(msg->_p2p_msg),target_task));
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
          msg->_target_task = (xmi_task_t) target_task;
          _device.addToNonDeterministicQueue (msg,__global.time.timebase());
#else
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg),
                          MPI_CHAR,
                          target_task,
                          0,
                          _device._communicator,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);
#endif
          return true;
        }


      inline bool postMultiPacket_impl (uint8_t              (&state)[MPIPacketModel::packet_model_state_bytes],
                                        xmi_event_function   fn,
                                        void               * cookie,
                                        xmi_task_t           target_task,
                                        size_t               target_offset,
                                        void               * metadata,
                                        size_t               metasize,
                                        void               * payload,
                                        size_t               length)
        {
          int rc;
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postMultiPacket_impl %d \n",this, this->_dispatch_id));
#ifdef EMULATE_UNRELIABLE_DEVICE
          unsigned long long t = __global.time.timebase ();
          if (t % EMULATE_UNRELIABLE_DEVICE_FREQUENCY == 0) return true;
#endif
          MPIMessage * msg = (MPIMessage *)malloc(sizeof(MPIMessage)+metasize+length-128-224);
          new(msg)MPIMessage(this->_client,this->_contextid,
                             this->_dispatch_id,
                             fn,
                             cookie);
          msg->_freeme=1;
          msg->_p2p_msg._metadatasize=metasize;
          msg->_p2p_msg._payloadsize0=length;
          msg->_p2p_msg._payloadsize1=0;
          memcpy(&msg->_p2p_msg._metadata[0], metadata, metasize);
          memcpy((char*)(&msg->_p2p_msg._metadata[0])+metasize, payload, length);
          TRACE_DEVICE((stderr,"<%p>MPIPacketModel::postMultiPacket_impl MPI_Isend %zd+%zd+%zd-128-244 to %zd\n",this,
                         sizeof(msg->_p2p_msg),metasize,(sizeof(msg->_p2p_msg)+metasize+length-128-224),target_task));
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
          msg->_target_task = (xmi_task_t) target_task;
          _device.addToNonDeterministicQueue (msg,__global.time.timebase());
#else
          rc = MPI_Isend (&msg->_p2p_msg,
                          sizeof(msg->_p2p_msg)+metasize+length-128-224,
                          MPI_CHAR,
                          target_task,
                          1,
                          _device._communicator,
                          &msg->_request);
          _device.enqueue(msg);
          assert(rc == MPI_SUCCESS);
#endif
          return true;
        };

    protected:
      T_Device                    & _device;
      xmi_client_t                  _client;
      xmi_context_t                 _context;
      size_t                        _contextid;
      size_t                        _dispatch_id;
      Interface::RecvFunction_t     _direct_recv_func;
      void                        * _direct_recv_func_parm;
      Interface::RecvFunction_t     _read_recv_func;
      void                        * _read_recv_func_parm;
    };
  };
};
#endif // __components_devices_mpi_mpipacketmodel_h__
