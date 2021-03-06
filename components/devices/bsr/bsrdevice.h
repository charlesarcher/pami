/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bsr/bsrdevice.h
 * \brief ???
 */

#ifndef __components_devices_bsr_bsrdevice_h__
#define __components_devices_bsr_bsrdevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/bsr/bsrmessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x)// fprintf x
#else
#define TRACE(x)// fprintf x
#endif


namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  namespace Device
  {
    class BSRDevice: public Interface::BaseDevice<BSRDevice>
    {
      
    public:
      class BSRMsyncMessage
      {
      public:

        typedef enum
        {
          UNINITIALIZED, // Create the syncgroup
          ENTERING,      // enter the barrier
          CHECKINGDONE,
          NOTREACHED
        } bsr_state_t;
        
        BSRMsyncMessage(BSRDevice           *device,
                        BSRGeometryInfo     *bsrinfo,
                        pami_event_function  done_fn,
                        void                *cookie):
          _state(UNINITIALIZED),
          _device(device),
          _bsrinfo(bsrinfo),
          _done_fn(done_fn),
          _cookie(cookie),
          _ready_dev_type(SaOnNodeSyncGroup::SA_TYPE_NONE)
          {
          }

        pami_result_t advance()
          {
            switch (_state)
            {
                case UNINITIALIZED:
                {
                  // Action:  Initialize
                  if (!_bsrinfo->_sync_group.IsInitialized())
                  {
                    SyncGroup::RC sg_rc =
                      _bsrinfo->_sync_group.CheckInitDone(&_ready_dev_type);
                    switch (sg_rc) {
                      case SyncGroup::SUCCESS:
                        switch (_ready_dev_type) {
                          case SaOnNodeSyncGroup::SA_TYPE_BSR:
                            ((LapiImpl::Context*)(_device->_context))->bsr_counter ++;
                            break;
                          case SaOnNodeSyncGroup::SA_TYPE_SHMARRAY:
                            ((LapiImpl::Context*)(_device->_context))->bsr_emu_counter ++;
                            break;
                          default:
                            PAMI_assertf(0, "Device type should be either BSR or SHM.");
                        }
                        _state = ENTERING;
                        break;
                      case SyncGroup::PROCESSING:
                        return PAMI_EAGAIN;
                      default:
                       assert(0);
                    }
                  } else {
                    _state = ENTERING;
                  }
                }
                  // no break, fallthrough
                case ENTERING:
                {
                  // Action:  enter barrier
                  _bsrinfo->_sync_group.NbBarrier();
                  _state  = CHECKINGDONE;
                  // no break, fallthrough
                }
                case CHECKINGDONE:
                {
                  // Action:  check if done
                  if(!_bsrinfo->_sync_group.IsNbBarrierDone())                    
                    return PAMI_EAGAIN;
                  else
                  {
                    _state = NOTREACHED;
                    _done_fn(_device->_context, _cookie, PAMI_SUCCESS);
                    return PAMI_SUCCESS;
                  }
                  break;
                }
                default:
                  PAMI_assertf(0, "BSR Message, unreached state");
            }
            PAMI_assertf(0, "BSR Message, unreached state 2");
            return PAMI_ERROR;            
          }
        bsr_state_t                 _state;
        BSRDevice                  *_device;
        BSRGeometryInfo            *_bsrinfo;
        pami_event_function         _done_fn;
        void                       *_cookie;
        Generic::GenericThread     *_workfcn;
        SaOnNodeSyncGroup::SaType   _ready_dev_type;
      };

      class BSRMcastMessage
      {
      };
      
      class BSRMcombineMessage
      {
      };






    public:
      BSRDevice(): _initialized(false)
	{
        }

      inline void          init(pami_client_t  client,
                                pami_context_t context,
                                size_t         context_id,
                                pami_task_t    my_task_id,
                                bool           affinity_checked)
        {
          _client     = client;
          _context    = context;
          _context_id = context_id;
          _my_task_id = my_task_id;
          // Check whether to enable BSR
          _initialized = affinity_checked;
        }

      pami_context_t getContext_impl()
        {
          return _context;
        }
      pami_context_t getClient_impl()
        {
          return _client;
        }

      bool isInit_impl()
        {
          return _initialized;
        }

      pami_task_t taskid()
        {
          return _my_task_id;
        }

      void       setGenericDevices(Generic::Device *generics)
        {
          _generics = generics;
        }

      Generic::GenericThread * postWork( pami_work_function work_fn, void *cookie)
        {
          Generic::GenericThread *work =
            (Generic::GenericThread *) _work_alloc.allocateObject();
          work = new (work) Generic::GenericThread(work_fn, cookie);
          work->setStatus(Ready);
          _generics[_context_id].postThread(work);
          return work;
        }

      void freeWork(Generic::GenericThread *work)
        {
          _work_alloc.returnObject(work);
        }

    private:
      bool                                                        _initialized;
      pami_task_t                                                 _my_task_id;
      pami_client_t                                               _client;
      size_t                                                      _context_id;
      pami_context_t                                              _context;
      Generic::Device                                            *_generics;
      PAMI::MemoryAllocator<sizeof(Generic::GenericThread), 16>   _work_alloc;
    };
  };
};
#endif // __components_devices_bsr_bsrpacketdevice_h__
