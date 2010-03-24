/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncCompositeT.h
 * \brief CCMI allreduce async composite template
 *
 * \todo AsyncCompositeT and CompositeT are very similar and could be combined.
 */

#ifndef __algorithms_protocols_allreduce_AsyncCompositeT_h__
#define __algorithms_protocols_allreduce_AsyncCompositeT_h__

#include "./AsyncComposite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // class AsyncCompositeT
      ///
      /// \brief Allreduce protocol composite
      ///
      ///
      ///
      template <class T_Schedule, class T_Executor, class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      class AsyncCompositeT : public CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager>
      {
      protected:
        T_Executor  _executor;
        char  _schedule[sizeof(T_Schedule)];
      public:
        static const char* name;
        /// Default Destructor
        virtual ~AsyncCompositeT()
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::~AsyncCompositeT() ALERT\n",this,name));
        }
        ///
        /// \brief Constructor
        ///
        AsyncCompositeT (PAMI_CollectiveRequest_t  * req,
                         T_Sysdep             * map,
                         T_ConnectionManager  *cmgr,
                         PAMI_Callback_t             cb_done,
                         pami_consistency_t            consistency,
                         T_Mcast   *mf,
                         PAMI_GEOMETRY_CLASS                  * geometry,
                         char                      * srcbuf,
                         char                      * dstbuf,
                         unsigned                    offset,
                         unsigned                    count,
                         pami_dt                     dtype,
                         pami_op                     op,
                         ConfigFlags                 flags,
                         CollectiveProtocolFactory           * factory,
                         unsigned                    iteration,
                         int                         root = -1,
                         CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
          CCMI::Adaptor::Allreduce::AsyncComposite<T_Mcast, T_Sysdep, T_ConnectionManager>( flags, factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), iteration)
        {
          create_schedule(map, geometry, color);
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::AsyncCompositeT() ALERT\n",this,name));
          addExecutor (&_executor);
          initialize (&_executor, req, srcbuf, dstbuf, count,
                      dtype, op, root);
          _executor.setMulticastInterface (mf);
          _executor.setSchedule ((T_Schedule*)&_schedule);
          _executor.reset ();
        }
        // Template implementation must specialize this function.
        void create_schedule(T_Sysdep        * map,
                             PAMI_GEOMETRY_CLASS                  * geometry,
                             CCMI::Schedule::Color       color)
        {
          CCMI_abort();
        }
        // Template implementation must specialize this function.
        static bool analyze (PAMI_GEOMETRY_CLASS *geometry)
        {
          CCMI_abort();
          return false;
        }
      }; // class AsyncCompositeT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
