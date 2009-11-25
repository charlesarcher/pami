/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDevice.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu_MUDevice_h__
#define __components_devices_bgq_mu_MUDevice_h__

#include <stdint.h>

#include <spi/include/mu/RecFifo.h>

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/bgq/mu/InjFifoSubGroup.h"
#include "components/devices/bgq/mu/MUChannel.h"
#include "components/devices/bgq/mu/Dispatch.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include "SysDep.h"
#include "Global.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

#define ENABLE_MAMBO_WORKAROUNDS

// \brief Number of dispatch sets
//
// There are DISPATCH_SET_COUNT possible dispatch IDs.
//
#define DISPATCH_SET_COUNT 256

// \brief Number of dispatch functions in a dispatch set
#define DISPATCH_SET_SIZE   16

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      typedef   MUSPI_RecvFunction_t  MUDevice_DispatchFn_t;

      class MUDevice : public Interface::BaseDevice<MUDevice,SysDep>, public Interface::PacketDevice<MUDevice>//, public CDI::Dma::Device<MUDevice>
          //class MUDevice : public BaseDevice, public CDI::Base::Device<MUDevice>, public CDI::Message::Device<MUDevice>//, public CDI::Dma::Device<MUDevice>
      {

          static const unsigned MAX_NUM_P2P_CHANNELS = 64;

          static const unsigned NULL_P2P_CHANNEL = MAX_NUM_P2P_CHANNELS;

        public:
          MUDevice ();
          ~MUDevice();

          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------
          //
          //   Device interface implementations -- XMI::Device::Interface::BaseDevice
          //
          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------

          /// \copydoc XMI::Device::Interface::BaseDevice::init
          int init_impl (SysDep * sysdep);

          /// \copydoc XMI::Device::Interface::BaseDevice::isInit
          bool isInit_impl ();

          /// \copydoc XMI::Device::Interface::BaseDevice::peers
          inline size_t peers_impl ();

          /// \copydoc XMI::Device::Interface::BaseDevice::task2peer
          inline size_t task2peer_impl (size_t task);

          /// \copydoc XMI::Device::Interface::BaseDevice::advance
          inline int advance_impl ();

          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------
          //
          //   Device interface implementations -- XMI::Device::Interface::PacketDevice
          //
          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------

          /// \copydoc XMI::Device::Interface::PacketDevice::readData
          inline int read_impl (void * dst, size_t length, void * cookie);

          static const size_t packet_metadata_size  = 17;  // <-- replace with a constant from SPIs somewhere
          static const size_t message_metadata_size = 13;  // <-- replace with a constant from SPIs somewhere
          static const size_t payload_size          = 512; // <-- replace with a constant from SPIs somewhere

          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------
          //
          //   Device interface implementations -- XMI::Device::Interface::MessageDevice
          //
          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------

          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------
          //
          //   Device interface implementations -- XMI::Device::Interface::DmaDevice
          //
          // ----------------------------------------------------------------------
          // ----------------------------------------------------------------------



          bool registerPacketHandler (size_t                      dispatch,
                                      Interface::RecvFunction_t   function,
                                      void                      * arg,
                                      uint16_t                  & id);

          bool push (size_t target_rank, MUDescriptorWrapper & wrapper)
          {
            InjFifoSubGroup *injFifoSubGroup = NULL;
            uint32_t         relativeFnum = 0;

            uint32_t fnum = _p2pChannel[_p2pSendChannelIndex]->pinFifo (target_rank);

            getSubGroupAndRelativeFifoNum ( fnum,
                                            &injFifoSubGroup,
                                            &relativeFnum );
            wrapper.setFIFONum ( relativeFnum );

            if (_p2pChannel[_p2pSendChannelIndex]->isEmptyMsgQ(fnum))
              {
                return (injFifoSubGroup->push ( wrapper ) != (uint64_t) - 1);
              }

            return false;
          }

          ///
          ///
          /// \see advanceInjectionFifoDescriptorTail
          ///
          inline bool nextInjectionDescriptor (size_t                target_rank,
                                               MUSPI_InjFifo_t    ** injfifo,
                                               MUHWI_Descriptor_t ** desc,
                                               void               ** payloadVa,
                                               void               ** payloadPa)
          {
            uint32_t fnum = _p2pChannel[_p2pSendChannelIndex]->pinFifo (target_rank);
            TRACE((stderr, ">> MUDevice::nextInjectionDescriptor() .. _p2pChannel[%d]->pinFifo (%zd) = %d\n", _p2pSendChannelIndex, target_rank, _p2pChannel[_p2pSendChannelIndex]->pinFifo (target_rank)));

            if (!_p2pChannel[_p2pSendChannelIndex]->isEmptyMsgQ(fnum)) return false;

            TRACE((stderr, "   MUDevice::nextInjectionDescriptor() .. before getSubGroupAndRelativeFifoNum()\n"));
            InjFifoSubGroup *injFifoSubGroup = NULL;
            uint32_t         relativeFnum = 0;
            getSubGroupAndRelativeFifoNum ( fnum,
                                            &injFifoSubGroup,
                                            &relativeFnum );
            TRACE((stderr, "   MUDevice::nextInjectionDescriptor() ..  after getSubGroupAndRelativeFifoNum(), injFifoSubGroup = %p, relativeFnum = %d\n", injFifoSubGroup, relativeFnum));

            bool success = injFifoSubGroup->nextDescriptor (relativeFnum, injfifo, desc, payloadVa, payloadPa);

            TRACE((stderr, "<< MUDevice::nextInjectionDescriptor() ..  success = %d\n", success));
            return success;
          }

          inline void addToDoneQ (size_t                target_rank,
                                  MUDescriptorWrapper * wrapper)
          {
            InjFifoSubGroup * injFifoSubGroup = NULL;
            uint32_t          relativeFnum = 0;

            uint32_t fnum = _p2pChannel[_p2pSendChannelIndex]->pinFifo (target_rank);

            getSubGroupAndRelativeFifoNum (fnum,
                                           &injFifoSubGroup,
                                           &relativeFnum);

            injFifoSubGroup->addToDoneQ(relativeFnum, wrapper);
          }

          inline void addToSendQ (size_t      target_rank,
                                  QueueElem * msg)
          {
            InjFifoSubGroup * injFifoSubGroup = NULL;
            uint32_t          relativeFnum = 0;

            uint32_t fnum = _p2pChannel[_p2pSendChannelIndex]->pinFifo (target_rank);

            getSubGroupAndRelativeFifoNum (fnum,
                                           &injFifoSubGroup,
                                           &relativeFnum);

            injFifoSubGroup->addToSendQ(relativeFnum, msg);
          }



#if 0
          ///
          /// \brief post a message to the MU Fifos of the current P2P channel.
          /// The message is short circuited if the software injection queue is empty. Hence a
          /// send/post will immediately be posted on the MU.
          ///  \param msg : the message to post
          ///  \param dstrank : the destination to send the message
          ///  \param lifo : Post message at the front of queue or back of
          ///  queue. Its posted at the back by default. This is required
          ///  in the RZV protocol to avoid deadlocks. The Remote Get ACK
          ///  can be stuck behind a Remote Get message to the same
          ///  destination, allowing the two processors to deadlock each
          ///  other.
          ///
          void     post( MUBaseMessage &msg, int dstrank, bool lifo = false )
          {
            //printf("MUDeevice.h post - p2p chindex:%d\n",_p2pSendChannelIndex);
            _p2pChannel[_p2pSendChannelIndex]->post( msg, dstrank, lifo );
          }


          ///
          /// \brief post a collective  message to the MU Fifos of the collective channel.
          /// The message is short circuited if the software injection queue is empty. Hence a
          /// send/post will immediately be posted on the MU.
          ///  \param msg : the message to post
          void    post( MUBaseMessage &msg )
          {
            XMI_assert( _colSendChannelFlag );
            _colChannel->post( msg );
          }
#endif
          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Acquire and release P2P channel
          ///
          //////////////////////////////////////////////////////////////////////////
          void p2pSendChannelAcquire(unsigned ch)
          {
            XMI_assert( ch < MAX_NUM_P2P_CHANNELS && _p2pChannel[ch] );

            _p2pSendChannelIndex = ch;
          }

          void p2pSendChannelRelease()
          {
            _p2pSendChannelIndex = NULL_P2P_CHANNEL;
          }

          void p2pRecvChannelAcquire(unsigned ch)
          {
            XMI_assert( ch < MAX_NUM_P2P_CHANNELS && _p2pChannel[ch] );

            _p2pRecvChannelIndex = ch;
          }

          void p2pRecvChannelRelease()
          {
            _p2pRecvChannelIndex = NULL_P2P_CHANNEL;
          }

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Acquire and release the collective channel
          ///
          //////////////////////////////////////////////////////////////////////////


          void colSendChannelAcquire()
          {
            _colSendChannelFlag = true;
          }

          void colSendChannelRelease()
          {
            _colSendChannelFlag = false;
          }


          void colRecvChannelAcquire()
          {
            _colRecvChannelFlag = true;
          }

          void colRecvChannelRelease()
          {
            _colRecvChannelFlag = false;
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Get info about mapped channels
          ///
          //////////////////////////////////////////////////////////////////////////
          void p2pChannelInfo( unsigned * startIndex, unsigned * num  )
          {
            *startIndex =  _firstP2PChannelIndex;
            *num        =  _numP2PChannels;
          }

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Number of mapped channels
          ///
          //////////////////////////////////////////////////////////////////////////
          unsigned p2pChannelsNum()
          {
            return _numP2PChannels;
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Map a Fifo Number to an InjFifoSubGroup and a Relative Fifo
          ///        Number
          ///
          /// \param[in]  fifoNum  The input fifo number.  This spans the subgroups.
          /// \param[in,out] injFifoSubGroup  Input is a pointer to the output
          ///                                 injection fifo subgroup pointer.
          ///                                 The specified fifo belongs to this
          ///                                 injection fifo subgroup.
          /// \param[in,out] relativeFifoNum  Input is a pointer to the output
          ///                                 fifo number that is relative to the
          ///                                 injection fifo subgroup to which the
          ///                                 specified fifo belongs
          ///
          //////////////////////////////////////////////////////////////////////////

          void getSubGroupAndRelativeFifoNum ( uint32_t          fifoNum,
                                               InjFifoSubGroup **injFifoSubGroup,
                                               uint32_t         *relativeFifoNum )
          {
            _p2pChannel[_p2pSendChannelIndex]->
            getSubGroupAndRelativeFifoNum ( fifoNum,
                                            injFifoSubGroup,
                                            relativeFifoNum );
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Get Reception Fifo ID
          ///
          /// \param[in]  t  T coordinate
          ///
          /// \retval  recFifoID  Reception Fifo ID (global number across all
          ///                     subgroups).
          ///
          //////////////////////////////////////////////////////////////////////////

          uint32_t getRecFifoIdForDescriptor ( int t )
          {
            //return _resMgr.getRecFifoIdForDescriptor ( t );
            return _p2pChannel[_p2pSendChannelIndex]->getRecFifoIdForDescriptor ( t );
          }

          uint16_t getRgetInjFifoId (size_t target_rank)
          {
            return _p2pChannel[_p2pSendChannelIndex]->getRgetInjFifoId (target_rank);
          }

          SysDep * sysdep;                         /**< sysdep pointer     */

        protected:

          static __thread unsigned   _p2pSendChannelIndex;    /**< current P2P send channel */
          static __thread unsigned   _p2pRecvChannelIndex;    /**< current P2P recv channel */
          static __thread bool       _colSendChannelFlag;     /**< coll send channel flag   */
          static __thread bool       _colRecvChannelFlag;     /**< coll recv channel flag   */


          P2PChannel    * _p2pChannel[ MAX_NUM_P2P_CHANNELS ]; /**< P2P channels       */

          unsigned        _firstP2PChannelIndex;
          unsigned        _numP2PChannels;                     /**< # of p2p channels  */

          ColChannel        * _colChannel;                     /**< Collective channel */
          bool                _initialized;                    /**< true when init() invoked successfully */

          dispatch_t      _dispatch[DISPATCH_SET_COUNT * DISPATCH_SET_SIZE];
          //uint8_t         _dispatch_num;

        private:
          ///
          /// \brief Noop/abort recv function used to initialize
          ///        unregistered dispatch ids.
          ///
          /// \see XMI::Device::RecvFunction_t
          ///
          static int noop (void   * metadata,
                           void   * payload,
                           size_t   bytes,
                           void   * recv_func_parm,
                           void   * cookie);

      }; // XMI::Device::MU::MUDevicee class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// XMI::Device::MU::MUDevice inline method interface implementations
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

size_t XMI::Device::MU::MUDevice::peers_impl ()
{
  return __global.mapping.size();
}

size_t XMI::Device::MU::MUDevice::task2peer_impl (size_t task)
{
  return task;
}

int XMI::Device::MU::MUDevice::advance_impl()
{
  int events = 0;

#if 1
  static size_t loopcount = 0;
  if (loopcount++ > 1000) XMI_abort();
#endif

  TRACE((stderr, ">> MUDevice::advance_impl() .. _p2pSendChannelIndex = %d (%d)\n", _p2pSendChannelIndex, NULL_P2P_CHANNEL));
  if ( _p2pSendChannelIndex != NULL_P2P_CHANNEL )
    events += _p2pChannel[_p2pSendChannelIndex]->advanceSend();

  TRACE((stderr, "   MUDevice::advance_impl() .. _p2pRecvChannelIndex = %d (%d)\n", _p2pRecvChannelIndex, NULL_P2P_CHANNEL));
  if ( _p2pRecvChannelIndex != NULL_P2P_CHANNEL )
    events += _p2pChannel[_p2pRecvChannelIndex]->advanceRecv();

  TRACE((stderr, "   MUDevice::advance_impl() .. _colSendChannelFlag = %d\n", _colSendChannelFlag));
  if ( _colSendChannelFlag )
    events += _colChannel->advanceSend();

  TRACE((stderr, "   MUDevice::advance_impl() .. _colRecvChannelFlag = %d\n", _colRecvChannelFlag));
  if ( _colRecvChannelFlag )
    events += _colChannel->advanceRecv();

  TRACE((stderr, "<< MUDevice::advance_impl() .. events = %d\n", events));
  return events;
}

int XMI::Device::MU::MUDevice::read_impl (void * dst, size_t length, void * cookie)
{
  // TODO - replace this with an optimized packet copy function.
  memcpy (dst, cookie, length);
  return 0;
};

#undef TRACE

#endif // __components_devices_bgq_mu_mudevice_h__
