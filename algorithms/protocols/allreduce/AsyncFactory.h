/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncFactory.h
 * \brief CCMI factory for [all]reduce composite
 */


#ifndef __algorithms_protocols_allreduce_AsyncFactory_h__
#define __algorithms_protocols_allreduce_AsyncFactory_h__

#include "algorithms/protocols/allreduce/AsyncComposite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "math/math_coremath.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // A temporary callback for asynchronous generates. The local
      // restart will set the true callback and replace this one.
      void temp_done_callback(void* cd, xmi_result_t *err)
      {
        CCMI_abort();
      }

      //-- AsyncFactory
      /// \brief Base class for allreduce factory implementations.
      ///
      /// It provides the unexpected multisend callback and the basic
      /// function to retrieve an executor from a geometry (associated
      /// with a single comm id).
      ///
      template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      class AsyncFactory : public CollectiveProtocolFactory
      {
      protected:
        ///
        /// \brief Multisend interface
        ///
        T_Mcast   * _minterface;

        ///
        ///  \brief Connection Manager for the allreduce
        ///
        T_ConnectionManager   * _connmgr;

        ///
        /// \brief get geometry from comm id
        ///
        xmi_mapidtogeometry_fn               _cb_geometry;

        ///
        /// \brief mapping module
        ///
        T_Sysdep                          * _mapping;

        ///
        /// \brief Configuration flags
        //
        ConfigFlags                       _flags;

      ///
      /// \brief Generate a non-blocking allreduce message.
      ///
      static XMI_Request_t *   cb_receiveHead(const xmi_quad_t    * info,
                                               unsigned          count,
                                               unsigned          peer,
                                               unsigned          sndlen,
                                               unsigned          conn_id,
                                               void            * arg,
                                               unsigned        * rcvlen,
                                               char           ** rcvbuf,
                                               unsigned        * pipewidth,
                                               XMI_Callback_t * cb_done)
      {
        TRACE_ADAPTOR((stderr,
                       "<%#.8X>Allreduce::AsyncFactory::cb_receiveHead peer %d, conn_id %d\n",
                       (int)arg, peer, conn_id));
        CCMI_assert (info && arg);
        CollHeaderData  *cdata = (CollHeaderData *) info;
        AsyncFactory *factory = (AsyncFactory *) arg;

        XMI_GEOMETRY_CLASS *geometry = (XMI_GEOMETRY_CLASS *)factory->_cb_geometry(cdata->_comm);
        CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> *composite =
        factory->getAllreduceComposite(geometry, cdata->_iteration);

        CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *allreduce =
        factory->getAllreduce(geometry, cdata->_iteration);

        TRACE_ADAPTOR((stderr,
                       "<%#.8X>Allreduce::AsyncFactory::cb_receiveHead "
                       "comm %#X, root %#X, count %#X, dt %#X, op %#X, iteration %#X,"
                       "composite %#.8X, executor %#.8X %s\n",
                       (int)factory, cdata->_comm, cdata->_root, cdata->_count,
                       cdata->_dt, cdata->_op, cdata->_iteration,
                       (int)composite, (int)allreduce,
                       (composite == NULL?" ":
                        ((composite->isIdle())?"(Idle)":" "))));

        if((allreduce == NULL) || (composite == NULL))
        {
          composite = factory->buildComposite (geometry, cdata);
          allreduce = (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *) composite->getExecutor (0);
        }
        else if(composite->isIdle())
        {
          composite->restartAsync(allreduce,
                                  cdata->_count,
                                  (xmi_dt)(cdata->_dt),
                                  (xmi_op)(cdata->_op),
                                  cdata->_root);
        }

        TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                       "cb_receiveHead(%#X,%#.8X)\n",
                       (int)factory,cdata->_comm,(int)allreduce));

        return allreduce->notifyRecvHead (info,      count,
                                          peer,      sndlen,
                                          conn_id,   arg,
                                          rcvlen,    rcvbuf,
                                          pipewidth, cb_done);
      };

      public:


        /// NOTE: This is required to make "C" programs link
        /// successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for allreduce factory implementations.
        ///
        inline AsyncFactory(T_Sysdep                                           * mapping,
                            T_Mcast        * minterface,
                            xmi_mapidtogeometry_fn                           cb_geometry,
                            ConfigFlags                                   flags ) :
        _minterface (minterface),
        _connmgr    (NULL),
        _cb_geometry(cb_geometry),
        _mapping    (mapping),
        _flags      (flags)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::AsyncFactory::ctor() ALERT:\n",(int)this));
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::AsyncFactory::ctor(%#X) "
                         "flags %#X\n",
                         (int)this, sizeof(*this),*(unsigned*)&_flags));
          _minterface->setCallback(cb_receiveHead, this);
        }

        inline void setConnectionManager
        (T_ConnectionManager  * connmgr)
        {
          _connmgr = connmgr;
        }

        ///
        /// \brief Generate a non-blocking allreduce message.
        ///
        virtual CCMI::Executor::Composite *generate
        (XMI_CollectiveRequest_t * request,
         XMI_Callback_t            cb_done,
         xmi_consistency_t           consistency,
         XMI_GEOMETRY_CLASS                 * geometry,
         char                     * srcbuf,
         char                     * dstbuf,
         unsigned                   count,
         xmi_dt                      dtype,
         xmi_op                      op,
         int                        root = -1 ) = 0;

        ///
        /// \brief Generate a non-blocking async allreduce message
        /// without starting it.
        ///
        virtual CCMI::Executor::Composite *generateAsync
        (XMI_GEOMETRY_CLASS                 * geometry,
         unsigned                   count,
         xmi_dt                      dtype,
         xmi_op                      op,
         unsigned                   iteration,
         int                        root = -1 ) = 0;


        AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager>          * buildComposite (XMI_GEOMETRY_CLASS         * geometry,
                                                  CollHeaderData   * cdata)
        {
          AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> *composite = (CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> *)
                                      generateAsync(geometry,
                                                    cdata->_count,
                                                    (xmi_dt)(cdata->_dt),
                                                    (xmi_op)(cdata->_op),
                                                    cdata->_iteration);
          composite->setQueueing();

          return composite;
        }


        ///
        /// \brief Get the executor associated with a comm id (and
        /// color/iteration id)
        ///
        CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * getAllreduce(XMI_GEOMETRY_CLASS *geometry,
                                                     unsigned iter)
        {
          CCMI::Executor::OldComposite *composite =
	    (CCMI::Executor::OldComposite *)geometry->getAllreduceComposite(iter);

          CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *executor = (composite)?
                                                    (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *) composite->getExecutor (0):
                                                    (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *)NULL;

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                         "getAllreduce(comm id X, color %#X)"
                         " callback %#X, composite %#.8X  executor %#.8X\n",
                         (int)this,// comm,
                         iter,
                         (int) _cb_geometry,
                         (int) composite,
                         (int) executor));
          return executor;
        }
        ///
        /// \brief Get the composite associated with a comm id (and
        /// iteration id).  It is expected to be associated with this Factory,
        /// otherwise destroy it and return NULL.
        ///
        CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> * getAllreduceComposite(XMI_GEOMETRY_CLASS *geometry,
                                                                         unsigned iteration)
        {
          CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> *composite = (CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager> *)
                                                                geometry->getAllreduceComposite(iteration);

          TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::AsyncFactory::"
                         "getAllreduceComposite(comm id X, iteration %#X)"
                         " callback %#X, composite's factory %#.8X  composite %#.8X\n",
                         (int)this, //comm,
                         iteration,
                         (int) _cb_geometry,
                         (int) (composite? composite->getFactory(): NULL),
                         (int)composite));

          if(composite && (composite->getFactory() != this))
          {
            CCMI_assert(!composite->isQueueing() && !composite->isStarted());//unfinished operation?
//            composite->~AsyncComposite();
            composite = NULL;
            geometry->setAllreduceComposite(composite, iteration);
          }

          return composite;
        }
      }; // class AsyncFactory
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
