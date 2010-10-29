/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/fifo/Fifo.h
 * \brief ???
 */

#ifndef __util_fifo_Fifo_h__
#define __util_fifo_Fifo_h__

#include "components/memory/MemoryManager.h"

#ifndef TRACE
#define TRACE(x)
#endif

namespace PAMI
{
  namespace Fifo
  {
    namespace Interface
    {
      template <class T_Producer>
      class PacketProducer
      {
        protected:

          inline PacketProducer () {};
          inline ~PacketProducer () {};

        public:

          ///
          /// \brief Intialize a packet with header and payload data
          ///
          template <class T_Packet>
          inline bool produce (T_Packet & packet)
          {
            return static_cast<T_Producer*>(this)->produce_impl (packet);
          };
      };

      template <class T_Consumer>
      class PacketConsumer
      {
        protected:

          inline PacketConsumer () {};
          inline ~PacketConsumer () {};

        public:

          ///
          /// \brief Read packet header and payload data
          ///
          template <class T_Packet>
          inline bool consume (T_Packet & packet)
          {
            return static_cast<T_Consumer*>(this)->consume_impl (packet);
          };
      };
    };

    ///
    /// \brief Fifo interface class
    ///
    /// \tparam T_Fifo Fifo implementation class
    ///
    template <class T_Fifo>
    class Fifo
    {
      protected:

        Fifo () {};

        ~Fifo () {};

      public:

        ///
        /// \brief Number of available bytes in each packet header for application data
        ///
        /// \note All fifo implementation classes must define the static constant
        ///       \c packet_header_size_impl.
        ///
        static const size_t packet_header_size = T_Fifo::packet_header_size_impl;

        ///
        /// \brief Number of available bytes in each packet payload for application data
        ///
        /// \note All fifo implementation classes must define the static constant
        ///       \c packet_payload_size_impl.
        ///
        static const size_t packet_payload_size = T_Fifo::packet_payload_size_impl;

        ///
        /// \brief Initialize a fifo using a memory manager and unique key.
        ///
        /// The memory manager may be used by a fifo implementation to
        /// construct the actual fifo packet array in a known location.
        ///
        /// After initialization the fifo is available for use.
        ///
        /// \param [in] mm  Memory manager to allocate internal fifo objects
        /// \param [in] key Unique key that identifies the fifo
        ///
        inline void initialize (PAMI::Memory::MemoryManager * mm,
                                char                        * key);

        ///
        /// \brief Initialize a fifo by cloning a previously initialized fifo.
        ///
        /// \param [in] A previously initialized fifo object
        ///
        inline void initialize (T_Fifo & fifo);

        ///
        /// \brief Retrieve the identifier of the last packet that was produced by this fifo object.
        ///
        /// The identifier is a monotonically increasing integer.
        ///
        /// \note The packet indentifier does not represent the last packet
        ///       produced globally, such as when there are multiple fifo
        ///       objects used by multiple producers.
        ///
        /// \return Identifier of the last packet produced by this fifo object
        ///
        inline size_t lastPacketProduced ();

        ///
        /// \brief Retrieve the identifier of the last packet that was consumed in this fifo.
        ///
        /// The identifier is a monotonically increasing integer.
        ///
        /// \note The packet indentifier represents the last packet that was
        ///       consumed globally, typically as when there are multiple fifo
        ///       objects but only one consumer.
        ///
        /// \return Identifier of the last packet consumed in this fifo
        ///
        inline size_t lastPacketConsumed ();

        ///
        /// \brief Produce a packet into the fifo.
        ///
        /// If the fifo is not full or otherwise unavailable the functor object
        /// produce method is invoked to write the packet into the fifo.
        ///
        /// \see PAMI::Fifo::Interface::PacketProducer::produce()
        /// \see PAMI::Fifo::Interface::Packet
        ///
        /// \tparam T_Functor Packet producer functor implementation class
        ///
        /// \param [in] produce Packet producer functor implementation object
        ///
        /// \retval true  A packet was produced
        /// \retval false A packet was not produced
        ///
        template <class T_Functor>
        inline bool producePacket (T_Functor & produce);

        ///
        /// \brief Consume a packet from the fifo.
        ///
        /// If the fifo is not empty the functor object consume method is
        /// invoked to read the next packet from the fifo.
        ///
        /// \see PAMI::Fifo::Interface::PacketConsume::consume()
        /// \see PAMI::Fifo::Interface::Packet
        ///
        /// \tparam T_Functor Packet consumer functor implementation class
        ///
        /// \param [in] consume Packet consumer functor implementation object
        ///
        /// \retval true  A packet was produced
        /// \retval false A packet was not produced
        ///
        template <class T_Functor>
        inline bool consumePacket (T_Functor & consume);
    };

    template <class T_Fifo>
    void Fifo<T_Fifo>::initialize (PAMI::Memory::MemoryManager * mm,
                                   char                        * key)
    {
      return static_cast<T_Fifo*>(this)->initialize_impl (mm, key);
    }

    template <class T_Fifo>
    void Fifo<T_Fifo>::initialize (T_Fifo & fifo)
    {
      return static_cast<T_Fifo*>(this)->initialize_impl (fifo);
    }

    template <class T_Fifo>
    size_t Fifo<T_Fifo>::lastPacketProduced ()
    {
      return static_cast<T_Fifo*>(this)->lastPacketProduced_impl ();
    }

    template <class T_Fifo>
    size_t Fifo<T_Fifo>::lastPacketConsumed ()
    {
      return static_cast<T_Fifo*>(this)->lastPacketConsumed_impl ();
    }

    template <class T_Fifo>
    template <class T_Functor>
    bool Fifo<T_Fifo>::producePacket (T_Functor & produce)
    {
      return static_cast<T_Fifo*>(this)->producePacket_impl (produce);
    }

    template <class T_Fifo>
    template <class T_Functor>
    bool Fifo<T_Fifo>::consumePacket (T_Functor & consume)
    {
      return static_cast<T_Fifo*>(this)->consumePacket_impl (consume);
    }
  };
};
#endif // __util_fifo_Fifo_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
