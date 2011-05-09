/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_CollectiveProtocolFactoryT_h__
#define __algorithms_protocols_CollectiveProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "components/memory/MemoryAllocator.h"
#include "util/ccmi_util.h"

namespace CCMI
{
namespace Adaptor
{
///
/// \brief choose if this protocol is supports the input geometry
///
typedef void      (*MetaDataFn)   (pami_metadata_t *m);

template <class T_Composite, MetaDataFn get_metadata, class T_Conn>
class CollectiveProtocolFactoryT: public CollectiveProtocolFactory
{
public:
    class collObj
    {
    public:
        collObj(Interfaces::NativeInterface             * native,
                T_Conn                                   * cmgr,
                pami_geometry_t                            geometry,
                pami_xfer_t                              * cmd,
                pami_event_function                        fn,
                void                                     * cookie,
                CollectiveProtocolFactoryT               * factory):
            _factory(factory),
            _user_done_fn(cmd->cb_done),
            _user_cookie(cmd->cookie),
            _obj(native, cmgr, geometry, cmd, fn, cookie)
        {
            TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::collObj()\n",this));
            DO_DEBUG((templateName<T_Composite>()));
        }
        void done_fn( pami_context_t   context,
                      pami_result_t    result )
        {
            _user_done_fn(context, _user_cookie, result);
        }

        CollectiveProtocolFactoryT * _factory;
        pami_event_function          _user_done_fn;
        void                       * _user_cookie;
        T_Composite                            _obj;
        unsigned                     _connection_id;
    };

public:
    CollectiveProtocolFactoryT (T_Conn                      *cmgr,
                                Interfaces::NativeInterface *native,
                                pami_dispatch_multicast_function cb_head = NULL):
        CollectiveProtocolFactory(),
        _cmgr(cmgr),
        _native(native)
    {
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT() %p\n",this, native));
        DO_DEBUG((templateName<MetaDataFn>()));
        if(_native != NULL)
          _native->setMulticastDispatch(cb_head, this);
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
                        void           *clientdata,
                        pami_result_t   res)
    {
        collObj *cobj = (collObj *)clientdata;
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::done_fn()\n",cobj));
        cobj->done_fn(context, res);
        cobj->_factory->_alloc.returnObject(cobj);
    }


    virtual Executor::Composite * generate(pami_geometry_t             geometry,
                                           void                      * cmd)
    {
        collObj *cobj = (collObj*) _alloc.allocateObject();
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::generate()\n",cobj));
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
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::metadata()\n",this));
        DO_DEBUG((templateName<MetaDataFn>()));
        get_metadata(mdata);
    }

    T_Conn                                     * _cmgr;
    Interfaces::NativeInterface                * _native;
    PAMI::MemoryAllocator < sizeof(collObj), 16 >   _alloc;
};//CollectiveProtocolFactoryT

};//Adaptor
};//CCMI

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
