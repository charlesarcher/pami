/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/Context.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_Context_h__
#define __components_devices_bgq_mu2_Context_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/RecFifo.h>
#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/MU.h>

#include "Mapping.h"

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/bgq/mu2/global/Global.h"
#include "components/devices/bgq/mu2/InjChannel.h"
#include "components/devices/bgq/mu2/InjGroup.h"
#include "components/devices/bgq/mu2/RecChannel.h"
#include "components/devices/bgq/mu2/CounterPool.h"

#include "util/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

extern PAMI::Device::MU::Global __MUGlobal;
extern void *__mu_context_cache;

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      /// \todo Eliminate the need for this class to implement
      ///       Interface::BaseDevice and Interface::PacketDevice
      ///
      class Context : public Interface::BaseDevice<Context>, public Interface::PacketDevice<Context>
      {
        public:

          typedef enum
          {
            PINFIFO_ALGORITHM_CACHED = 0,
            PINFIFO_ALGORITHM_RUNTIME
          } pinfifo_algorithm_t;

          ///
          /// \brief Number of bytes available in a packet payload.
          /// \todo Replace with a constant from SPIs somewhere
          ///
          static const size_t packet_payload_size    = 512;

          ///
          /// \brief Number of bytes in each lookaside payload buffer element
          ///
          static const size_t immediate_payload_size = sizeof(MU::InjGroup::immediate_payload_t);

	  ///
	  /// \brief Initial Value for Combining Inj Fifo Number
	  ///        This indicates this value has not been set.
	  ///
	  static const size_t combiningInjFifoIdNotSet = 0xFFFF;

          ///
          /// \brief Reverse Fifo Pin array
          ///
          uint32_t injFifoPinReverse[16];

          ///
          /// \brief foo
          ///
          /// \param[in] mapping   The mapping component is used to translate
          ///                      coordinates to task identifers, etc
          /// \param[in] id_base   The base identifier of the MU::Context with
          ///                      offset zero
          /// \param[in] id_offset Offset from the base identifier for this
          ///                      MU::Context.
          /// \param[in] id_count  Number of MU::Context objects in the context
          ///                      set - all context objects in a set share a
          ///                      common base identifier
          ///
          inline Context (PAMI::Mapping   & mapping,
                          size_t            id_base,
                          size_t            id_offset,
                          size_t            id_count) :
              Interface::BaseDevice<Context> (),
              Interface::PacketDevice<Context> (),
	      _pamiRM ( __MUGlobal.getPamiRM() ),
              _rm ( __MUGlobal.getMuRM() ),
              _mapping (mapping),
              _id_base (id_base),
              _id_offset (id_offset),
 	      _id_count (id_count),
	      _combiningInjFifo (combiningInjFifoIdNotSet),
              _counterPool(NULL),
              _numCounterPools(1)
          {
            TRACE_FN_ENTER();

	    // Reverse the fifo pin.
	    // For example, if pinned to fifo 0 (AM), output is pinned to fifo 1 (AP).
	    injFifoPinReverse[0] = 1;
	    injFifoPinReverse[1] = 0;
            injFifoPinReverse[2] = 3;
            injFifoPinReverse[3] = 2;
            injFifoPinReverse[4] = 5;
            injFifoPinReverse[5] = 4;
            injFifoPinReverse[6] = 7;
            injFifoPinReverse[7] = 6;
            injFifoPinReverse[8] = 9;
            injFifoPinReverse[9] = 8;
            injFifoPinReverse[10] = 11;
            injFifoPinReverse[11] = 10;
            injFifoPinReverse[12] = 11;
            injFifoPinReverse[13] = 10;
            injFifoPinReverse[14] = 11;
            injFifoPinReverse[15] = 10;

            TRACE_FN_EXIT();
          };

          ///
          /// \brief Initialize the mu context
          ///
          /// Functionally replaces the implementation of the
          /// Interface::BaseDevice::init interface and only specifies
          /// parameters that are necessary for MU operations.
          ///
          /// \param[in] id_client           The client identifier
          /// \param[in] mu_context_cookie   MU context cookie delivered in callbacks
          ///
          inline pami_result_t init (size_t   id_client,
                                     size_t   id_context,
                                     void   * mu_context_cookie,
                                     Generic::Device       * progress)
          {
            TRACE_FN_ENTER();

            // Select the generic device for this context.
	    _progressDevice = &(Generic::Device::Factory::getDevice (progress, id_client, id_context));

	    _mu_context_cookie = mu_context_cookie;

            _id_client = id_client;

            // Map the PAMI client ID to the resource manager's client ID.
            // The PAMI client ID is assigned on a first-come-first-served
            // basis PAMI_Client_create() is called.  We need an ID that
            // corresponds to the clients named on PAMI_CLIENTNAMES so the
            // resource manager knows which client we are talking about.
            _rm_id_client = id_client;

            // Need to find a way to break this dependency...
            //_client = client;
            //_context = context;

            // Resource Manager allocates the resources.

            // Construct arrays of Inj and Rec fifo pointers.
            _rm.getNumResourcesPerContext(_rm_id_client,
                                        &_numInjFifos,
                                        &_numRecFifos,
				        &_numBatIds );
	    pami_result_t prc;
	    prc = __global.heap_mm->memalign((void **)&_injFifos, 0,
					_numInjFifos * sizeof(*_injFifos));
	    PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _injFifos failed");

	    prc = __global.heap_mm->memalign((void **)&_recFifos, 0,
					_numRecFifos * sizeof(*_recFifos));
	    PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _recFifos failed");

	    uint32_t *globalInjFifoIds;
	    prc = __global.heap_mm->memalign((void **)&globalInjFifoIds, 0,
					_numInjFifos * sizeof(*globalInjFifoIds));
	    PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _globalInjFifoIds failed");

	    prc = __global.heap_mm->memalign((void **)&_globalRecFifoIds, 0,
					_numRecFifos * sizeof(*_globalRecFifoIds));
	    PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _globalRecFifoIds failed");

            _rm.getInjFifosForContext( _rm_id_client,
                                       _id_offset,
                                       _numInjFifos,
                                       _injFifos,
				       globalInjFifoIds );

            _rm.getRecFifosForContext( _rm_id_client,
                                       _id_offset,
                                       _numRecFifos,
                                       _recFifos,
                                       _globalRecFifoIds );

            // Get arrays of the following:
            // 1. Lookaside payload buffers virtual addresses
            // 2. Lookaside payload buffers physical addresses
            // 3. Lookaside completion function pointers
            // 4. Lookaside completion cookies
            // 5. Pin Injection Fifo Map that maps from the optimal 10 inj fifos
            //    to the actual number of fifos.
            _lookAsidePayloadVAs = _rm.getLookAsidePayloadBufferVAs( _rm_id_client,
                                                                     _id_offset );
            _lookAsidePayloadPAs = _rm.getLookAsidePayloadBufferPAs( _rm_id_client,
                                                                     _id_offset );
            _lookAsideCompletionFnPtrs = _rm.getLookAsideCompletionFnPtrs( _rm_id_client,
                                                                           _id_offset );
            _lookAsideCompletionCookiePtrs = _rm.getLookAsideCompletionCookiePtrs( _rm_id_client,
                                                                                   _id_offset );

            _pinInfo = _rm.getPinInfo( _numInjFifos );

	    _rgetPinInfo = _rm.getRgetPinInfo();

	    // Get a handle to the RecFifo info for this client.  Perf optimization.
	    _pinRecFifoHandle = _rm.getPinRecFifoHandle ( _rm_id_client );

	    // Get a handle to the RecFifo info for this client's first context.  
	    // Perf optimization.
	    _pinRecFifoHandleForOffsetZero = _rm.getPinRecFifoHandle ( _rm_id_client, 0 );

            // Initialize the injection channel(s) inside the injection group
            size_t fifo;

            for ( fifo = 0; fifo < _numInjFifos; fifo++ )
              {
                injectionGroup.initialize (fifo,
                                           _injFifos[fifo],
					   globalInjFifoIds[fifo],
                                           (InjGroup::immediate_payload_t*)
                                           _lookAsidePayloadVAs[fifo],
                                           _lookAsidePayloadPAs[fifo],
                                           _lookAsideCompletionFnPtrs[fifo],
                                           _lookAsideCompletionCookiePtrs[fifo],
                                           _rm.getMaxNumDescInInjFifo(),
                                           mu_context_cookie);  // \todo This should be the pami_context_t
              }

	    // ----------------------------------------------------------------
	    // If this context is supposed to use the combining injection
	    // fifo, then
	    // 1.  Store the context-relative fifo number.
	    // 2.  Initialize the next slot in the injectionGroup with this
	    //     injection channel.
	    // Do this if
	    // 1. This client is the one that can use MU combining collectives.
	    // 2. This task is the lowest T coordinate on the node.
	    // 3. This is context 0 in this client.
	    // ----------------------------------------------------------------
	    if ( ( _pamiRM.doesClientOptimizeCombiningCollectivesInMU( _rm_id_client ) ) &&
		 ( _mapping.isLowestT() ) &&
		 ( _id_offset == 0 ) )
	      {
		char                 **globalCombiningInjFifoLookAsidePayloadVAs =
		  _rm.getGlobalCombiningLookAsidePayloadBufferVAs();
		uint64_t              *globalCombiningInjFifoLookAsidePayloadPAs =
		  _rm.getGlobalCombiningLookAsidePayloadBufferPAs();
		pami_event_function  **globalCombiningInjFifoLookAsideCompletionFnPtrs =
		  _rm.getGlobalCombiningLookAsideCompletionFnPtrs();
		void                ***globalCombiningInjFifoLAsideCompletionCookiePtrs =
		  _rm.getGlobalCombiningLookAsideCompletionCookiePtrs();

		_combiningInjFifo = _numInjFifos;

		injectionGroup.initialize (_combiningInjFifo,   /* fnum */
					   _rm.getGlobalCombiningInjFifoPtr (),
                                           _rm.getGlobalCombiningInjFifoId (),
					   (InjGroup::immediate_payload_t*)
                                           globalCombiningInjFifoLookAsidePayloadVAs[0],
                                           globalCombiningInjFifoLookAsidePayloadPAs[0],
                                           globalCombiningInjFifoLookAsideCompletionFnPtrs[0],
                                           globalCombiningInjFifoLAsideCompletionCookiePtrs[0],
                                           _rm.getMaxNumDescInInjFifo(),
					   NULL ); /* channel_cookie */
		TRACE_FORMAT("Context::init():  Context is controlling combining InjChannel.  Context-relative InjFifo Id = %u, t=%zu\n",_combiningInjFifo, _mapping.t());
	      }
	    
	    free( globalInjFifoIds );

            // ----------------------------------------------------------------
            // Initialize the reception channel
            // ----------------------------------------------------------------
            receptionChannel.initialize (_globalRecFifoIds[0],
                                         _recFifos[0],
                                         _mapping.getMuDestinationSelf(),
                                         mu_context_cookie);

	    // Store a pointer in the injectionGroup that points to the MU MMIO
	    // field for clearing interrupts.  This will be used later during
	    // advance() to clear the MU interrrupts.
	    MUSPI_RecFifoSubGroup_t *_recFifoSubgroup =
	      _rm.getRecFifoSubgroup( _rm_id_client,
				      _id_offset );
	    injectionGroup.setClearInterruptsStatusPtr ( _recFifoSubgroup );

	    // Construct a interrupt bitmask indicating which interrupts to clear
	    // 64 bits:
	    // - Bits  0 through 31 clear injection fifo threshold crossing
	    // - Bits 32 through 47 clear reception fifo threshold crossing
	    // - Bits 48 through 63 clear reception fifo packet arrival
	    _interruptMask = _rm.getInterruptMask( _rm_id_client,
						   _id_offset );

	    _rm.init(id_client, id_context, progress);

            // ----------------------------------------------------------------
            // Initialize the deterministic packet connection array.
            // ----------------------------------------------------------------
            size_t i, num_endpoints = _mapping.size() * _id_count;
            
            pami_result_t mmrc;
  	        mmrc = __global.heap_mm->memalign((void **) & _connection, 16, sizeof(void *) * num_endpoints);
            PAMI_assertf(mmrc == PAMI_SUCCESS, "memalign failed for mu connection array, rc=%d\n", mmrc);

            for (i = 0; i < num_endpoints; i++) _connection[i] = NULL;

            // ----------------------------------------------------------------
            // Initialize the MU Counter Pools for this context.
            // 1. Determine how many pools we need based on the number of
            //    counters requested via env var.
            // 2. Allocate space from the heap for these pools.
            // 3. Construct each pool.
            // ----------------------------------------------------------------
            
////////////////////////////////////////////////////////////////////////////////
/// \env{bgq,PAMI_NUMDYNAMICROUTING}
/// Number of simultaneous dynamically routed messages per context.  If more
/// than this many messages are being transferred, the additional messages are
/// deterministically routed.  Dynamic routing can be faster than deterministic
/// routing.  However, dynamically routed messages require more storage to
/// track their progress, hence the reason for this option.  This number
/// should be specified in increments of 64.  A value of 0 forces all messages
/// to be deterministically routed.
/// \default 0
////////////////////////////////////////////////////////////////////////////////
            
////////////////////////////////////////////////////////////////////////////////
/// \env{bgq,PAMI_DYNAMICROUTINGZONE}
/// The zone routing to be used for dynamically routed messages.  There are
/// four zones: 0, 1, 2, and 3.
/// \default 3
////////////////////////////////////////////////////////////////////////////////

            unsigned long numDynamicRouting = 0;
            char *env;
            if ((env = getenv("PAMI_NUMDYNAMICROUTING"))) 
            {
              numDynamicRouting = strtoul(env, NULL, 0);
            }

            unsigned long dynamicRoutingZone = 3;
            if ((env = getenv("PAMI_DYNAMICROUTINGZONE"))) 
            {
              dynamicRoutingZone = strtoul(env, NULL, 0);
            }
            PAMI_assertf( dynamicRoutingZone < 4, "PAMI_DYNAMICROUTINGZONE is %zu. Must be 0, 1, 2, or 3.\n", dynamicRoutingZone);
            _dynamicRoutingZone = (dynamicRoutingZone==0) ? MUHWI_PACKET_ZONE_ROUTING_0 :
                                  (dynamicRoutingZone==1) ? MUHWI_PACKET_ZONE_ROUTING_1 :
                                  (dynamicRoutingZone==2) ? MUHWI_PACKET_ZONE_ROUTING_2 :
                                  MUHWI_PACKET_ZONE_ROUTING_3;

            if ( numDynamicRouting == 0 )
            {
              _numCounterPools = 0;
              _counterPool     = NULL;
            }
            else
            {
              _numCounterPools = numDynamicRouting / 64;
              if ( _numCounterPools == 0 ) _numCounterPools = 1;
              
              mmrc = __global.heap_mm->memalign((void **) & _counterPool, 
                                                8, 
                                                _numCounterPools*sizeof(CounterPool));
              PAMI_assertf(mmrc == PAMI_SUCCESS, "memalign failed for mu counter pool, rc=%d\n", mmrc);
              
              uint64_t poolID;
              for ( poolID=0; poolID<_numCounterPools; poolID++ )
              {
                new ( &_counterPool[poolID] ) CounterPool( (pami_context_t)mu_context_cookie,
                                                           _progressDevice );
              }
            }

            TRACE_FN_EXIT();
            return PAMI_SUCCESS;
          }

#if 1
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // Begin device interface implementations.
          //
          // These implementations should be removed when the device interfaces
          // are eliminated, with the remaining device interfaces mirrored with
          // new model interfaces.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------

          ///
          /// \copydoc Interface::BaseDevice::init
          ///
          inline int init_impl (Memory::MemoryManager * mm,
                                size_t                  clientid,
                                size_t                  num_ctx,
                                pami_context_t          context,
                                size_t                  contextid)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContext
          ///
          inline pami_context_t getContext_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return (pami_context_t) 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextOffset
          ///
          inline size_t getContextOffset_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextCount
          ///
          inline size_t getContextCount_impl ()
          {
            return _id_count;
          }

          ///
          /// \copydoc Interface::BaseDevice::isInit
          ///
          inline bool isInit_impl ()
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::peers
          ///
          inline size_t peers_impl ()
          {
            // All tasks are addressable "peers" to the MU, therefore the peer
            // size is the same as the task size;
            return _mapping.size();
          }

          ///
          /// \copydoc Interface::BaseDevice::task2peer
          ///
          inline size_t task2peer_impl (size_t task)
          {
            // All tasks are addressable "peers" to the MU
            return task;
          }

          ///
          /// \copydoc Interface::BaseDevice::isPeer
          ///
          inline bool isPeer_impl (size_t task)
          {
            // All tasks are addressable "peers" to the MU
            return true;
          }

          ///
          /// \copydoc Interface::BaseDevice::advance
          ///
          int advance_impl ()
          {
//            TRACE_FN_ENTER();

	    // If interrupts are ON, clear them BEFORE advancing.
	    // In this way, we will not miss an interruptable event.
	    // Note that the SPI only does the clear if the interrupt mask
	    // is non-zero.  This is faster than always doing the store
	    // to the MU MMIO interrupts status field.
	    //
	    // Also, storing the MU MMIO pointer and the interrupt mask
	    // inside of the injectionGroup (as opposed to in this context object)
	    // is faster because we need to touch the contents of the
	    // injectionGroup anyway, so one touch should bring it all in.

	    MUSPI_ClearInterruptsDirect( injectionGroup.getClearInterruptsStatusPtr(),
					 injectionGroup.getInterruptMask() );
	    size_t events  = injectionGroup.advance ();
	    events += receptionChannel.advance ();
//            TRACE_FN_EXIT();

	    return events;

          }

          ///
          /// \copydoc Interface::PacketDevice::read
          ///
          inline int read_impl (void * dst, size_t bytes, void * cookie)
          {
            memcpy(dst, cookie, bytes);
            return 0;
          }

          /// \see PAMI::Device::Interface::PacketDevice::Deterministic::clearConnection()
          inline void clearConnection_impl (size_t task, size_t offset)
          {
            size_t index = task * _id_count + offset;

            TRACE_FORMAT("(%zu,%zu) .. _connection[%zu] = %p -> %p", task, offset, index, _connection[index], (void *) NULL);      
            PAMI_assert_debugf(_connection[index] != NULL, "Error. _connection[%zu] was not previously set.\n", index);

            _connection[index] = NULL;
          }
              
          /// \see PAMI::Device::Interface::PacketDevice::Deterministic::getConnection()
          inline void * getConnection_impl (size_t task, size_t offset)
          {
            size_t index = task * _id_count + offset;

            TRACE_FORMAT("(%zu,%zu) .. _connection[%zu] = %p", task, offset, index, _connection[index]);      
            PAMI_assert_debugf(_connection[index] != NULL, "Error. _connection[%zu] was not previously set.\n", index);

            return _connection[index];
          }

          /// \see PAMI::Device::Interface::PacketDevice::Deterministic::setConnection()
          inline void setConnection_impl (void * value, size_t task, size_t offset)
          {
            size_t index = task * _id_count + offset;

            TRACE_FORMAT("(%zu,%zu) .. _connection[%zu] = %p -> %p", task, offset, index, _connection[index], value);      
            PAMI_assert_debugf(_connection[index] == NULL, "Error. _connection[%zu] was previously set.\n", index);

            _connection[index] = value;
          }

          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // End device interface implementations.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
#endif

          ///
          /// \copydoc MU::RecChannel::registerPacketHandler
          ///
          inline bool registerPacketHandler (size_t                      set,
                                             Interface::RecvFunction_t   fn,
                                             void                      * cookie,
                                             uint16_t                  & id)
          {
            TRACE_FN_ENTER();
            bool status = receptionChannel.registerPacketHandler (set, fn, cookie, id);
            TRACE_FN_EXIT();
            return status;
          }

          /// \copydoc Mapping::getMuDestinationSelf
          inline MUHWI_Destination_t * getMuDestinationSelf ()
          {
            return _mapping.getMuDestinationSelf();
          };

          /// \copydoc Mapping::getMuDestinationTask
          inline void getMuDestinationTask (size_t               task,
                    MUHWI_Destination_t &dest,
                    size_t              &tcoord,
                    uint32_t            &fifoPin)
          {
            return _mapping.getMuDestinationTask(task, dest, tcoord, fifoPin);
          };
          inline void getMuDestinationTask (size_t               task,
                    MUHWI_Destination_t &dest)
          {
            return _mapping.getMuDestinationTask(task, dest);
          };

	  ///
	  /// \brief Return Whether This Context Is Using the MU for Combining Operations
	  ///
	  inline bool doesContextOptimizeCombiningCollectivesInMU ()
	    {
	      if ( _combiningInjFifo != combiningInjFifoIdNotSet )
		return true;
	      else
		return false;
	    }

	  ///
          /// \brief Pin Combining Fifo
          ///
          /// The pinCombiningFifo method is used to retrieve
	  /// 1.  The context-relative combining injection fifo number.
	  /// 2.  The global MU reception fifo identification number to receive
	  ///     the result of the combining operation.
	  ///
	  /// This function will assert if called on a context that is not
	  /// managing the combining injection fifo.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          ///
	  /// \param[in]  task  The task id whose reception fifo is to receive
	  ///                   the result of the combining collective.  This
	  ///                   will be the same reception fifo on all nodes
	  ///                   participating in the collective.
          /// \param[out] rfifo   Reception fifo id to receive the result of a
	  ///                     memory fifo combine.
	  ///
          /// \return  Context-relative injection fifo Id of the combining
	  ///          injection fifo.
	  ///          Reception fifo id to receive the result of a
	  ///          memory fifo combining operation.
          ///
          inline uint16_t pinCombiningFifo ( size_t    task,
					     uint16_t &rfifo )
	    {
	      TRACE_FN_ENTER();

	      PAMI_assertf ( _combiningInjFifo != combiningInjFifoIdNotSet,
			     "pinCombiningFifo: Trying to use combining injection fifo in an invalid context");

	      // Calculate the destination reception fifo identifier based on
	      // the destination task.
	      size_t addr[BGQ_TDIMS + BGQ_LDIMS];
	      _mapping.task2global ((pami_task_t)task, addr);
	      size_t tcoord = addr[5];

	      // Get the recFifo to use, for this client's first context, offset by
	      // the destination's T coord.
	      rfifo = _rm.getPinRecFifo( _pinRecFifoHandleForOffsetZero, tcoord );
	      TRACE_FORMAT("client=%zu, context=%zu, tcoord=%zu, rfifo = %u", _id_client, (size_t)0, tcoord, rfifo);
	      TRACE_FN_EXIT();

	      return _combiningInjFifo;
	    }


          ///
          /// \brief Pin Fifo (from Self to Destination)
          ///
          /// The pinFifo method is used for two purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the destination
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor.
          ///
          /// This is a "pinFromSelf" direction.  The data is assumed to
          /// be travelling from ourself to the task/offset destination.
          ///
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task    Destination task identifier
          /// \param[in]  offset  Destination task context offset identifier
          /// \param[out] dest    Destination task node coordinates
          /// \param[out] rfifo   Reception fifo id to address the task+offset
          ///                     This is a global id that can be put into
          ///                     the descriptor.
          /// \param[out] map     Pinned MUSPI torus injection fifo map
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          //template <pinfifo_algorithm_t T>
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 MUHWI_Destination_t & dest,
                                 uint16_t            & rfifo,
                                 uint64_t            & map)
          {
            TRACE_FN_ENTER();

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
            size_t tcoord = 0;
            uint32_t fifoPin = 0;
            _mapping.getMuDestinationTask( task, dest, tcoord, fifoPin );

            // Get the recFifo to use for this client, and the destination's
            // context and T coord.
            rfifo = _rm.getPinRecFifo( _pinRecFifoHandle, offset, tcoord );
            TRACE_FORMAT("client=%zu, context=%zu, tcoord=%zu, rfifo = %u", _id_client, offset, tcoord, rfifo);

            map = _pinInfo->torusInjFifoMaps[fifoPin];

            TRACE_FORMAT("(destTask %zu, destOffset %zu) -> dest = %08x, rfifo = %d, optimalFifoPin = %u, actualFifoPin = %u, map = %016lx, injFifoIds[]=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", task, offset, *((uint32_t *) &dest), rfifo, fifoPin, _pinInfo->injFifoIds[fifoPin], map, _pinInfo->injFifoIds[0], _pinInfo->injFifoIds[1], _pinInfo->injFifoIds[2], _pinInfo->injFifoIds[3], _pinInfo->injFifoIds[4], _pinInfo->injFifoIds[5], _pinInfo->injFifoIds[6], _pinInfo->injFifoIds[7], _pinInfo->injFifoIds[8], _pinInfo->injFifoIds[9], _pinInfo->injFifoIds[10], _pinInfo->injFifoIds[11], _pinInfo->injFifoIds[12], _pinInfo->injFifoIds[13], _pinInfo->injFifoIds[14], _pinInfo->injFifoIds[15] );
            TRACE_FN_EXIT();

            return  _pinInfo->injFifoIds[fifoPin];
          }


          ///
          /// \brief Pin Fifo (from Self to Destination) With RgetPacing Indicator
          ///
          /// The pinFifo method is used for three purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the destination
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor, and to return an
          /// indication whether rget pacing should be considered to this
          /// destination.
          ///
          /// This is a "pinFromSelf" direction.  The data is assumed to
          /// be travelling from ourself to the task/offset destination.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task    Destination task identifier
          /// \param[in]  offset  Destination task context offset identifier
          /// \param[out] dest    Destination task node coordinates
          /// \param[out] rfifo   Reception fifo id to address the task+offset
          ///                     This is a global id that can be put into
          ///                     the descriptor.
          /// \param[out] map     Pinned MUSPI torus injection fifo map
          /// \param[out] paceRgetsToThisDest True if rget pacing should
          ///                     be considered to this destination.
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          //template <pinfifo_algorithm_t T>
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 MUHWI_Destination_t & dest,
                                 uint16_t            & rfifo,
                                 uint64_t            & map,
				 uint32_t            & paceRgetsToThisDest)
          {
            TRACE_FN_ENTER();

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
            size_t tcoord = 0;
            uint32_t fifoPin = 0;
            _mapping.getMuDestinationTask( task, dest, tcoord, fifoPin, paceRgetsToThisDest );

	    // Get the recFifo to use for this client, and the destination's
	    // context and T coord.
            rfifo = _rm.getPinRecFifo( _pinRecFifoHandle, offset, tcoord );
            TRACE_FORMAT("client=%zu, context=%zu, tcoord=%zu, rfifo = %u", _id_client, offset, tcoord, rfifo);

            map = _pinInfo->torusInjFifoMaps[fifoPin];

            TRACE_FORMAT("(destTask %zu, destOffset %zu) -> dest = %08x, rfifo = %d, optimalFifoPin = %u, actualFifoPin = %u, map = %016lx, injFifoIds[]=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", task, offset, *((uint32_t *) &dest), rfifo, fifoPin, _pinInfo->injFifoIds[fifoPin], map, _pinInfo->injFifoIds[0], _pinInfo->injFifoIds[1], _pinInfo->injFifoIds[2], _pinInfo->injFifoIds[3], _pinInfo->injFifoIds[4], _pinInfo->injFifoIds[5], _pinInfo->injFifoIds[6], _pinInfo->injFifoIds[7], _pinInfo->injFifoIds[8], _pinInfo->injFifoIds[9], _pinInfo->injFifoIds[10], _pinInfo->injFifoIds[11], _pinInfo->injFifoIds[12], _pinInfo->injFifoIds[13], _pinInfo->injFifoIds[14], _pinInfo->injFifoIds[15] );
            TRACE_FN_EXIT();

            return  _pinInfo->injFifoIds[fifoPin];
          }

	  ///
	  /// \brief For dput collectives
	  ///
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 MUHWI_Destination_t & dest,
                                 size_t              & tcoord,
                                 uint64_t            & map)
          {
            TRACE_FN_ENTER();

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
            uint32_t fifoPin = 0;
            _mapping.getMuDestinationTask( task, dest, tcoord, fifoPin );

            map = _pinInfo->torusInjFifoMaps[fifoPin];

            TRACE_FN_EXIT();

            return  _pinInfo->injFifoIds[fifoPin];
          }


          ///
          /// \brief Pin Fifo (from Self to Destination)
          ///
          /// The pinFifo method is used for two purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the destination
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor.
          ///
          /// This is a "pinFromSelf" direction.  The data is assumed to
          /// be travelling from ourself to the task/offset destination.
          ///
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task    Destination task identifier
          /// \param[in]  offset  Destination task context offset identifier
          /// \param[out] dest    Destination task node coordinates
          /// \param[out] rfifo   Reception fifo id to address the task+offset
          ///                     This is a global id that can be put into
          ///                     the descriptor.
          /// \param[out] map     Pinned MUSPI torus injection fifo map
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          //template <pinfifo_algorithm_t T>
          inline size_t pinFifo (size_t                task,
                                 size_t                offset,
                                 uint32_t            & dest,
                                 uint16_t            & rfifo,
                                 uint64_t            & map)
          {
            TRACE_FN_ENTER();

            // Calculate the destination recpetion fifo identifier based on
            // the destination task+offset.  This is important for
            // multi-context support.
            size_t tcoord = 0;
            uint32_t fifoPin = 0;
            _mapping.getMuDestinationTask( task, dest, tcoord, fifoPin );

	    // Get the recFifo to use for this client, and the destination's
	    // context and T coord.
            rfifo = _rm.getPinRecFifo( _pinRecFifoHandle, offset, tcoord );
            TRACE_FORMAT("client=%zu, context=%zu, tcoord=%zu, rfifo = %u", _id_client, offset, tcoord, rfifo);

            map = _pinInfo->torusInjFifoMaps[fifoPin];

            TRACE_FORMAT("(destTask %zu, destOffset %zu) -> dest = %08x, rfifo = %d, optimalFifoPin = %u, actualFifoPin = %u, map = %016lx, injFifoIds[]=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", task, offset, *((uint32_t *) &dest), rfifo, fifoPin, _pinInfo->injFifoIds[fifoPin], map, _pinInfo->injFifoIds[0], _pinInfo->injFifoIds[1], _pinInfo->injFifoIds[2], _pinInfo->injFifoIds[3], _pinInfo->injFifoIds[4], _pinInfo->injFifoIds[5], _pinInfo->injFifoIds[6], _pinInfo->injFifoIds[7], _pinInfo->injFifoIds[8], _pinInfo->injFifoIds[9], _pinInfo->injFifoIds[10], _pinInfo->injFifoIds[11], _pinInfo->injFifoIds[12], _pinInfo->injFifoIds[13], _pinInfo->injFifoIds[14], _pinInfo->injFifoIds[15] );
            TRACE_FN_EXIT();

            return  _pinInfo->injFifoIds[fifoPin];
          }


          ///
          /// \brief Pin Fifo (to Self from Remote)
          ///
          /// The pinFifo method is used for two purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the remote
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor.
          ///
          /// This is a "pinToSelf" direction.  The info returned is to be
          /// put into a descriptor that is injected on the remote node.
          /// Data is assumed to be travelling from the specified task/offset
          /// to ourself.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task    Remote task identifier
          /// \param[out] map     Pinned MUSPI torus injection fifo map
          ///
          /// \return Global Remote Get Injection Fifo Number to use on the
          ///         remote node to inject the descriptor that sends data
          ///         back to our node.
          ///
          //template <pinfifo_algorithm_t T>
          inline size_t pinFifoToSelf (size_t                task,
                                       uint64_t            & map)
          {
            TRACE_FN_ENTER();

            // Get the fifo pin value as if we are sending to the remote node.
            // The "toSelf" pin will be the reverse.
            size_t tcoord = 0;
            uint32_t fifoPin = 0;
	    uint32_t reverseFifoPin;
            MUHWI_Destination_t dest;

            _mapping.getMuDestinationTask( task, dest, tcoord, fifoPin );

            // Reverse the fifoPin to the opposite direction.
            reverseFifoPin = injFifoPinReverse[fifoPin];

            // Use the reversed fifoPin to determine the map.
            map = _rgetPinInfo->torusInjFifoMaps[reverseFifoPin];

            TRACE_FORMAT("RemoteTask %zu, mapcacheFifoPin = %u, reverseFifoPin = %u, rgetFifoId=%u, map = %016lx, _rgetPinInfo->injFifoIds[] = %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", task, fifoPin, reverseFifoPin, _rgetPinInfo->injFifoIds[reverseFifoPin], map, _rgetPinInfo->injFifoIds[0],_rgetPinInfo->injFifoIds[1],_rgetPinInfo->injFifoIds[2],_rgetPinInfo->injFifoIds[3],_rgetPinInfo->injFifoIds[4],_rgetPinInfo->injFifoIds[5],_rgetPinInfo->injFifoIds[6],_rgetPinInfo->injFifoIds[7],_rgetPinInfo->injFifoIds[8],_rgetPinInfo->injFifoIds[9],_rgetPinInfo->injFifoIds[10],_rgetPinInfo->injFifoIds[11],_rgetPinInfo->injFifoIds[12],_rgetPinInfo->injFifoIds[13],_rgetPinInfo->injFifoIds[14],_rgetPinInfo->injFifoIds[15]);
            TRACE_FN_EXIT();

            return  _rgetPinInfo->injFifoIds[reverseFifoPin];
          }

          ///
          /// \param[in]  from_task   Origin task identifier
          /// \param[in]  from_offset Origin task context offset identifier
          /// \param[out] map         Pinned MUSPI torus injection fifo map
          ///
          inline void pinInformation (size_t     from_task,
                                      size_t     from_offset,
                                      uint64_t & map)
          {
            TRACE_FN_ENTER();
            // In loopback we send only on AM
            map =  MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM;

            TRACE_FORMAT("(%zu,%zu) -> map = %016lx\n", from_task, from_offset, map);
            TRACE_FN_EXIT();
          }

          inline int64_t allocateCounter( uint64_t poolID )
          {
            return _counterPool[poolID].allocate();
          }

          inline void setCounter ( uint64_t             poolID,
                                   int64_t              counterNum,
                                   uint64_t             value,
                                   pami_event_function  fn,
                                   void                *cookie )
	  {
            _counterPool[poolID].setCounter ( counterNum,
                                              value,
                                              fn,
                                              cookie );
          }

          inline uint64_t getCounterAtomicOffset ( uint64_t poolID,
                                                   int64_t  counterNum )
          {
            return _counterPool[poolID].getCounterAtomicOffset ( counterNum );
          }

          inline int addFenceOperation ( uint64_t                  poolID,
                                         CounterPool::fenceInfo_t *fenceInfoPtr,
                                         pami_event_function       fn,
                                         void                     *cookie )
          {
            return _counterPool[poolID].addFenceOperation ( fenceInfoPtr,
                                                            fn,
                                                            cookie );
          }

          inline uint64_t getNumCounterPools ()
          {
            return _numCounterPools;
          }

          inline uint8_t getDynamicRoutingZone ()
          {
            return _dynamicRoutingZone;
          }

          inline uint32_t getGlobalBatId ()
          {
            return _rm.getGlobalBatId();
          };

          inline uint32_t getSharedCounterBatId ()
          {
            return _rm.getSharedCounterBatId();
          };

          inline uint32_t getShortCollectiveBatId ()
	  {
	    return _rm.getShortCollectiveBatId();
          };

	  inline int32_t setShortCollectiveBatEntry ( uint64_t value )
	  {
	    return _rm.setShortCollectiveBatEntry ( value );
	  }

          inline uint32_t getThroughputCollectiveBufferBatId ()
	  {
	    return _rm.getThroughputCollectiveBufferBatId();
          };

	  inline int32_t setThroughputCollectiveBufferBatEntry ( uint64_t value )
	  {
	    return _rm.setThroughputCollectiveBufferBatEntry ( value );
	  }

          inline uint32_t getThroughputCollectiveCounterBatId ()
	  {
	    return _rm.getThroughputCollectiveCounterBatId();
          };

	  inline int32_t setThroughputCollectiveCounterBatEntry ( uint64_t value )
	  {
	    return _rm.setThroughputCollectiveCounterBatEntry ( value );
	  }

          inline uint32_t getCNShmemCollectiveCounterBatId ()
	  {
	    return _rm.getCNShmemCollectiveCounterBatId();
          };

	  inline int32_t setCNShmemCollectiveCounterBatEntry ( uint64_t value )
	  {
	    return _rm.setCNShmemCollectiveCounterBatEntry ( value );
	  }

          inline uint32_t getCNShmemCollectiveBufferBatId ()
	  {
	    return _rm.getCNShmemCollectiveBufferBatId();
          };

	  inline int32_t setCNShmemCollectiveBufferBatEntry ( uint64_t value )
	  {
	    return _rm.setCNShmemCollectiveBufferBatEntry ( value );
	  }

	  /// \brief Query the Number of Free BAT IDs Within This Context
	  ///
	  /// \retval  numFree
	  inline uint32_t queryFreeBatIds()
	  {
	    return _rm.queryFreeBatIdsForContext( _rm_id_client,
						  _id_offset );
	  }

	  /// \brief Allocate BAT IDs
	  ///
	  /// \param[in]  numBatIds    Number to allocate
	  /// \param[out] globalBatIds Array with numBatIds slots that will
	  ///                          be set to the global BAT IDs that
	  ///                          were allocated.
	  /// \retval  0  Success
	  /// \retval  -1 Failed to allocate the specified number of BatIds.
	  ///             No IDs were actually allocated.
	  inline int32_t allocateBatIds ( size_t    numBatIds,
					  uint16_t *globalBatIds )
	  {
	    return _rm.allocateBatIdsForContext( _rm_id_client,
						 _id_offset,
						 numBatIds,
						 globalBatIds );
	  }

	  /// \brief Free BAT IDs
	  ///
	  /// \param[in]  numBatIds    Number to free
	  /// \param[out] globalBatIds Array with numBatIds slots containing
	  ///                          the global BAT IDs to be freed.
	  inline void freeBatIds( size_t    numBatIds,
				  uint16_t *globalBatIds )
	  {
	    _rm.freeBatIdsForContext( _rm_id_client,
				      _id_offset,
				      numBatIds,
				      globalBatIds );
	  }

	  /// \brief Pin BAT ID
	  ///
	  /// \param[in] t           The T coordinate of the destination 
	  /// \param[in] globalBatId The global BAT id in this context that
	  ///                        would be used for this transfer.  This
	  ///                        is used to find the corresponding BAT ID
	  ///                        at the destination.
	  ///
	  /// \retval pinnedBatId The corresponding BAT ID on the destination.
	  inline uint16_t pinBatId( size_t t,
				    uint16_t globalBatId )
	  {
	    return _rm.getPinBatId ( _rm_id_client,
				     _id_offset,
				     t,
				     globalBatId );
	  }

	  /// \brief Set the Value of A BAT Entry in This Context
	  ///
	  /// \param[in] globalBatId The global BAT ID whose value is to be set
	  /// \param[in] value       The value to be set
	  ///
	  /// \retval  0  Success
	  /// \retval  -1 Failed to set the BAT ID.
	  inline int32_t setBatEntry ( uint16_t  globalBatId,
				       uint64_t  value )
	  {
	    return _rm.setBatEntryForContext ( _rm_id_client,
					       _id_offset,
					       globalBatId,
					       value );
	  }

          inline uint32_t *getRgetInjFifoIds ()
          {
            return _rm.getRgetInjFifoIds();
          };

	  inline uint32_t getGlobalCommAgentRecFifoId()
	  {
	    return _rm.getGlobalCommAgentRecFifoId();
	  }

	  inline bool isCommAgentActive()
	  {
	    return _rm.isCommAgentActive();
	  }

	  inline void commAgent_AllocateWorkRequest( CommAgent_WorkRequest_t **workPtrAddress,
						     uint64_t                 *uniqueIDaddress )
	  {
	    _rm.commAgent_AllocateWorkRequest( workPtrAddress, uniqueIDaddress );
	  }

	  inline int commAgent_RemoteGetPacing_SubmitWorkRequest( CommAgent_RemoteGetPacing_WorkRequest_t *workPtr )
	  {
	    return _rm.commAgent_RemoteGetPacing_SubmitWorkRequest( workPtr );
	  }

	  /// \brief Get the Progress device to post pami work to
	  ///
	  /// \retval generic device pointer
	  ///
	  inline Generic::Device *getProgressDevice() { return _progressDevice; }

	  /// \brief Get the Core Affinity for this Context
	  ///
	  /// \retval  coreId  The core number that this context is affiliated with.
	  ///
	  inline uint32_t affinity()
	    {
	      return _rm.getAffinity( _rm_id_client,
				      _id_offset );
	    }

	  // \brief Set Up Interrupts
	  //
	  // When a commthread acquires or releases this context, update the
	  // interrupt mask stored in the injection group accordingly.
	  // When acquiring, the specified interrupt mask is used.
	  // When releasing, the interrupt mask is set to zero.
	  //
	  // \param[in]  acquire  Boolean indicating what the commthread is doing.
	  //                      - true: Acquiring the context
	  //                      - false: Releasing the context
	  //
	  inline void setInterrupts( bool acquire )
	    {
	      injectionGroup.setInterruptMask( acquire, _interruptMask );
	    }
	  /// \brief Get the number of injection fifos
	  ///
	  ///
	  /// \retval  n number of injection fifos
	  ///
	  inline size_t getNumInjFifos()
	  {
	    return _numInjFifos;
	  }

	  /// \brief Get the MU Context Cookie
	  ///
	  /// This is actually the pointer to the PAMI context
	  ///
	  inline void *getMuContextCookie()
	  {
	    return _mu_context_cookie;
	  }

          RecChannel   receptionChannel; // Reception resources, public access
          InjGroup     injectionGroup __attribute__((__aligned__(16)));   // Injection resources, public access

        protected:

          // Resource Manager allocates resources
          size_t                 _numInjFifos;
          size_t                 _numRecFifos;
	  size_t                 _numBatIds;
          MUSPI_InjFifo_t      **_injFifos;
          MUSPI_RecFifo_t      **_recFifos;
          uint32_t              *_globalRecFifoIds;
          char                 **_lookAsidePayloadVAs;
          uint64_t              *_lookAsidePayloadPAs;
          pami_event_function  **_lookAsideCompletionFnPtrs;
          void                ***_lookAsideCompletionCookiePtrs;
          pinInfoEntry_t        *_pinInfo;
	  pinInfoEntry_t        *_rgetPinInfo;
	  void                  *_pinRecFifoHandle;
	  void                  *_pinRecFifoHandleForOffsetZero;

	  PAMI::ResourceManager & _pamiRM; // PAMI Resource Manager
	  ResourceManager & _rm; // MU Global Resource Manager
          PAMI::Mapping   & _mapping;
          size_t            _id_base;
          size_t            _id_offset;
          size_t            _id_count;
          size_t            _id_client;
          size_t            _rm_id_client;
	  uint32_t          _combiningInjFifo;       // Context-relative fifo id.
	                                             // Initialized to combiningInjFifoIdNotSet
                                                     // if this context does not manage it.
	  uint64_t                 _interruptMask;

	  Generic::Device         *_progressDevice;
    
	  void *            _mu_context_cookie;

        // -------------------------------------------------------------
        // Deterministic packet interface connection array
        // -------------------------------------------------------------
        
        void ** _connection;
	
        CounterPool *_counterPool;
        size_t       _numCounterPools;
        uint8_t      _dynamicRoutingZone;

      }; // class     PAMI::Device::MU::Context
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#if 0
///
/// \brief pinFifo template specialization for runtime pin calculation
///
/// \todo Hook in to the MU::ResourceManager
///
/// \see MU::Context::pinFifo
///
template <>
size_t PAMI::Device::MU::Context::pinFifo<PAMI::Device::MU::Context::pinfifo_algorithm_t::PINFIFO_ALGORITHM_RUNTIME> (size_t                task,
    size_t                offset,
    MUHWI_Destination_t & dest,
    uint16_t            & rfifo,
    uint64_t            & map)
{
  // return _resource_manager.pinFifo (...);
  return 0;
};
#endif

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_Context_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
