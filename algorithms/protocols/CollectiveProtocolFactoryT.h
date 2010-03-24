/**
 * \file algorithms/protocols/CollectiveProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_CollectiveProtocolFactoryT_h__
#define __algorithms_protocols_CollectiveProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "components/memory/MemoryAllocator.h"

namespace CCMI
{
  namespace Adaptor
  {
    ///
    /// \brief choose if this protocol is supports the input geometry
    ///
    typedef void      (*MetaDataFn)   (pami_metadata_t *m);

    template <class T, MetaDataFn get_metadata, class C>
      class CollectiveProtocolFactoryT: public CollectiveProtocolFactory
    {
      class collObj
      {
      public:
        collObj(Interfaces::NativeInterface             * native,
               C                                        * cmgr,
               pami_geometry_t                             geometry,
               pami_xfer_t                               * cmd,
               pami_event_function                         fn,
               void                                     * cookie,
               CollectiveProtocolFactoryT               * factory):
          _obj(native,cmgr,geometry,cmd,fn,cookie),
          _factory(factory),
          _user_done_fn(cmd->cb_done),
          _user_cookie(cmd->cookie)
          {
          }
        T                            _obj;
        CollectiveProtocolFactoryT * _factory;
        pami_event_function           _user_done_fn;
        void                       * _user_cookie;
      };


    public:
      CollectiveProtocolFactoryT (C                           *cmgr,
                                  Interfaces::NativeInterface *native,
                                  pami_dispatch_multicast_fn    cb_head=NULL):
        CollectiveProtocolFactory(),
        _cmgr(cmgr),
        _native(native)
      {
        pami_dispatch_callback_fn fn;
        fn.multicast = (pami_dispatch_multicast_fn) cb_head;
        _native->setDispatch(fn, this);
      }

      virtual ~CollectiveProtocolFactoryT ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      static void done_fn(pami_context_t  context,
                          void          *clientdata,
                          pami_result_t   res)
        {
          collObj *cobj = (collObj *)clientdata;
          cobj->_user_done_fn(context, cobj->_user_cookie, res);
          cobj->_factory->_alloc.returnObject(cobj);
        }


      virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                             void                      * cmd)
        {
          collObj *cobj = (collObj*) _alloc.allocateObject();
          new(cobj) collObj(_native,          // Native interface
                            _cmgr,            // Connection Manager
                            geometry,         // Geometry Object
                            (pami_xfer_t*)cmd, // Parameters
                            done_fn,          // Intercept function
                            cobj,             // Intercept cookie
                            this);            // Factory
          return (Executor::Composite *)&cobj->_obj;
        }

      virtual void metadata(pami_metadata_t *mdata)
        {
          get_metadata(mdata);
        }

      C                                          * _cmgr;
      Interfaces::NativeInterface                * _native;
      PAMI::MemoryAllocator<sizeof(collObj), 16>    _alloc;
    };
  };
};

#endif
