/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MemoryFifoPacketHeader.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__
#define __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__

#include <hwi/include/bqc/MU_PacketHeader.h>


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      ///  network and mu headers, 32B
      ///
      /// +-------------------+--+----------------------------------+
      /// | used by hw        |##| metadata available for           |
      /// | 13B 1b            |##| single packet messages, 17B      |
      /// +-------------------+--+----------------------------------+
      /// unused by hw    ^    ^ dispatch id, 14b
      /// 1b, 'single pkt'
      ///
      ///
      /// +-------------------------+--+----------------------------+
      /// | used by hw              |##| metadata available for     |
      /// | 18B                     |##| multi-packet messages, 12B |
      /// +-------------------------+--+----------------------------+
      /// unused by hw    ^          ^ dispatch id, 16b
      /// 1b, 'multi pkt'
      ///
      class MemoryFifoPacketHeader : public MUHWI_PacketHeader_t
      {
        public:

          /// Number of unused software bytes available in a single packet
          /// transfer. The dispatch identifier and other control data are
          /// are set outside of this range.
	  static const size_t packet_singlepacket_metadata_size = 16;

          /// Number of unused software bytes available in a multi-packet
          /// transfer. The dispatch identifier and other control data are
          /// are set outside of this range.
          static const size_t packet_multipacket_metadata_size  = 12;

          ///
          /// \brief Set the 'single packet' attribute in a mu packet header
          ///
          /// The 'single packet' attribute is a single bit in the header which
          /// is unused/unmodified by hardware. This bit is a hint to the type
          /// of packet to be sent or received. A 'single packet' message has
          /// additional bytes in the packet header which are unused and
          /// unmodified by the mu hardware. This allows software to 'pack'
          /// additional information into a 'single packet' header.
          ///
          /// \warning The single packet attribute must be set before the
          ///          dispatch id is set.
          ///
          /// \see setDispatchId
          /// \see getDispatchId
          ///
          /// \param[in] value  The 'single packet' attribute
          ///
          inline void setSinglePacket (bool value)
          {
            messageUnitHeader.Packet_Types.Memory_FIFO.Unused1 = value;
          };

          ///
          /// \brief Retreive the 'single packet' attribute from a mu packet header
          ///
          /// \see setSinglePacket
          ///
          /// \return The 'single packet' attribute value
          ///
          inline uint32_t isSinglePacket ()
          {
            return messageUnitHeader.Packet_Types.Memory_FIFO.Unused1;
          };

	  inline void setHeaderInfo (uint16_t id, void *metadata, uint16_t metasize){
	    //Use bytes 14 and 15 for single packet and 18 and 19 for multipacket
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    *raw16 = (id | (metasize<<12));

	    uint8_t *raw8 = (uint8_t *) this + 32 - metasize;
	    uint8_t *src = (uint8_t *) metadata;
	    //If metasize is a constant compiler will inline the memcpy
	    memcpy(raw8, src, metasize);
	  }

	  inline void getHeaderInfo (uint16_t &id, void **metadata) {
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    id = (*raw16)&0x3FFF; //Use 12bits for dispatch id
	    *metadata = (char *)this + 32 - ((*raw16)>>12);
	  }


          ///
          /// \brief Set the dispatch id in a mu packet header
          ///
          /// The dispatch id is written in a different location depending on
          /// the value of the "single packet" attribute in the header.
          ///
          /// \warning The single packet attribute must be set before this
          ///          method is used.
          ///
          /// \see setSinglePacket
          /// \see isSinglePacket
          ///
          /// \param[in] id  The dispatch identifier to set
          ///
          inline void setDispatchId (uint16_t id)
          {
	    //Use bytes 14 and 15 for single packet and 18 and 19 for multipacket
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    *raw16 = (id & 0x3FFF);
            return;
          };

          ///
          /// \brief Get the dispatch id from a mu packet header
          ///
          /// The dispatch id is in a different location depending on
          /// the value of the "single packet" attribute of the header.
          ///
          /// \warning The single packet attribute must be set before this
          ///          method is used.
          ///
          /// \see setSinglePacket
          /// \see isSinglePacket
          ///
          /// \return The dispatch identifier
          ///
          inline uint16_t getDispatchId ()
          {
	    //Use bytes 14 and 15 for single packet and 18 and 19 for multipacket
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    return (*raw16)&0x3FFF; //Use 14bits for dispatch id
          };


	  ///
	  /// \brief Set the Metadata correspoding to this
	  /// packet. Metadata will be aligned to the input parameter
	  /// bytes
	  /// \param[in] metadata of the packet protocol
	  /// \param[in] metadata size in bytes
	  ///
	  inline void setMetaData (void *metadata, int bytes) 
	  { 
	    //Use bytes 14 and 15 for single packet and 18 and 19 for multipacket
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    
	    *raw16 = ((*raw16) | (bytes<<12));

	    uint8_t *raw8 = (uint8_t *) this + 32 - bytes;
	    uint8_t *src = (uint8_t *) metadata;
	    memcpy(raw8, src, bytes);
	  }

	  ///
	  /// \brief Get the Metadata in the packet
	  /// \retval pointer to the metadata
	  ///
	  inline void* getMetaData () { 
	    uint16_t *raw16 = (uint16_t*)((char *)this + (18 - isSinglePacket()*4));
	    uint16_t bytes = (*raw16)>>12;
	    //return the bottom maxmetasize bytes
	    return (char *)this + (32 - bytes);
	  }

      };
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif   // __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
