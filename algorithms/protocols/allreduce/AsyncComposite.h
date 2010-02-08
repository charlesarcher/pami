/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncComposite.h
 * \brief CCMI composite adaptor for allreduce with barrier support
 */

#ifndef __algorithms_protocols_allreduce_AsyncComposite_h__
#define __algorithms_protocols_allreduce_AsyncComposite_h__

#include "./Composite.h"
//  #include "Geometry.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/executor/AllreduceBase.h"
#include "math/math_coremath.h"

namespace CCMI
{
  namespace Adaptor
  {

    namespace Allreduce
    {

      // Forward declare prototype
      extern void getReduceFunction(xmi_dt, xmi_op, unsigned,
                                    unsigned&, coremath&) __attribute__((noinline));

      //-- AsyncComposite
      /// \brief The Async Composite for the Allreduce kernel executor.
      ///
      /// It does common initialization for all subclasses (protocols)
      /// such as mapping the operator and datatype to a function and
      /// calling various setXXX() functions in the kernel executor.
      ///
      template <class T_Mcast, class T_Sysdep, class T_ConnectionManager>
      class AsyncComposite : public BaseComposite
      {
      protected:
        ///
        /// \brief The number of done callbacks to be called - both
        /// barrier and allreduce callbacks.  It will vary based on
        /// whether subclasses are using the barrier or not.
        ///
        int                             _doneCountdown;

        /// \brief The asynchronous state of the operation.
        /// Idle/Done - no operation in progress
        /// Started - a local operation has started
        /// Queueing - a unexpected/asynchronous message arrived before
        /// a local operation has started
        int                             _asyncState;
        static const int                _isIdle     = 0;
        static const int                _isStarted  = 1;
        static const int                _isQueueing = 2;
        static const int                _isDone     = 3;

        ///
        /// \brief Configuration flags
        ///
        ConfigFlags                     _flags;

        ///
        /// \brief Client's callback to call when the allreduce has
        /// finished
        ///
        void               (* _myClientFunction)(void *, xmi_result_t *);
        void                * _myClientData;
      public:

#ifdef CCMI_DEBUG
        unsigned                          _count;
        xmi_dt                            _dt;
        xmi_op                            _op;
        unsigned                          _iteration;
        unsigned                          _root;
#endif // CCMI_DEBUG

        inline void setDone()
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::setDone()\n",this));_asyncState = _isDone;
        }
        inline void setIdle()
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::setIdle()\n",this));_asyncState = _isIdle;
        }
        inline void setStarted()
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::setStarted()\n",this));_asyncState = _isStarted;
        }
        inline void setQueueing()
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::setQueueing()\n",this));_asyncState = _isQueueing;
        }
        inline bool isDone()
        {
          return _asyncState == _isDone;
        }
        inline bool isIdle()
        {
          return(_asyncState == _isIdle) || (_asyncState == _isDone);
        }
        inline bool isStarted()
        {
          return _asyncState == _isStarted;
        }
        inline bool isQueueing()
        {
          return _asyncState == _isQueueing;
        }

        AsyncComposite () :
        BaseComposite (NULL),
        _doneCountdown(1),  // default to just a composite done needed
        _asyncState(_isIdle)
        {
          CCMI_abort();
        }

        AsyncComposite ( ConfigFlags                       flags,
                         CollectiveProtocolFactory                 * factory,
                         XMI_Callback_t                   cb_done):

        BaseComposite (factory),
        _doneCountdown(1),  // default to just a composite done needed
        _asyncState(_isIdle),
        _flags(flags),
        _myClientFunction (cb_done.function),
        _myClientData (cb_done.clientdata)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::AsyncComposite::ctor() ALERT:\n",this));
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::ctor() flags(%#X) factory(%#X)\n",this,
                         *(unsigned*)&flags, (int) factory));
        }

        /// Default Destructor
        virtual ~AsyncComposite()
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::AsyncComposite::dtor() ALERT:\n",this));
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::dtor()\n",this));
        }

        void operator delete (void *p)
        {
          CCMI_abort();
        }

        ///
        /// \brief initialize should be called after the executors
        /// have been added to the composite
        ///
        void initialize ( CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * allreduce,
                          XMI_CollectiveRequest_t        * request,
                          char                            * srcbuf,
                          char                            * dstbuf,
                          unsigned                          count,
                          xmi_dt                            dtype,
                          xmi_op                            op,
                          int                               root,
                          unsigned                          pipelineWidth = 0,// none specified, calculate it
                          void                           (* cb_done)(void *, xmi_result_t *) = cb_compositeDone,
                          void                            * cd = NULL
                        )
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::initialize()\n",this));
          allreduce->setSendState(request);
          allreduce->setRoot( root );
          allreduce->setDataInfo(srcbuf, dstbuf);

          allreduce->setDoneCallback( cb_done, cd == NULL? this: cd );

          if((op != allreduce->getOp()) || (dtype != allreduce->getDt()) ||
             (count != allreduce->getCount()))
          {
            coremath func;
            unsigned sizeOfType;
            CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, count,
                                                        sizeOfType, func);

            unsigned min_pwidth = MIN_PIPELINE_WIDTH;
            if(dtype == XMI_DOUBLE && op == XMI_SUM)
              min_pwidth = MIN_PIPELINE_WIDTH_SUM2P;

            /* Select pipeline width.
                Zero which means calculate it.
                -1 means no pipelining so use count*sizeOfType.
            */

            /*
               First, the function parameter overrides the config value.
            */
            unsigned pwidth = pipelineWidth ? pipelineWidth : _flags.pipeline_override;
            /*
               If -1, disable pipelining or use specified value
            */
            pwidth = (pwidth == (unsigned)-1)? count*sizeOfType : pwidth;
            /*
               Use specified (non-zero) value or calculate (if zero is specified)
            */
            pwidth = pwidth ? pwidth : computePipelineWidth (count, sizeOfType, min_pwidth);

            allreduce->setReduceInfo ( count, pwidth, sizeOfType,
                                       func, op, dtype );
          }
        }

        unsigned computePipelineWidth (unsigned count, unsigned sizeOfType, unsigned min_pwidth)
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::computePipelineWidth() count %#X, size %#X, min %#X\n",this,
                         count, sizeOfType, min_pwidth));
          unsigned pwidth = min_pwidth;

          if(count * sizeOfType > 1024 * pwidth)
            pwidth *= 32;
          else if(count * sizeOfType > 256  * pwidth)
            pwidth *= 16;
          else if(count * sizeOfType > 64 * pwidth)
            pwidth *= 8;
          else if(count * sizeOfType > 16 * pwidth)
            pwidth *= 4;

          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::computePipelineWidth() pwidth %#X\n",this,
                         pwidth));
          return pwidth;
        }

        ///
        /// \brief At this level we only support single color
        /// collectives
        ///
        virtual unsigned restart   ( XMI_CollectiveRequest_t  * request,
                                     XMI_Callback_t           & cb_done,
                                     xmi_consistency_t          consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     size_t                      count,
                                     xmi_dt                      dtype,
                                     xmi_op                      op,
                                     size_t                      root = (size_t)-1)
        {
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::restart()\n",this));
          _myClientFunction = cb_done.function;
          _myClientData     = cb_done.clientdata;

          CCMI_assert_debug (getNumExecutors() == 1);
          CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * allreduce =
          (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *) getExecutor(0);

          initialize (allreduce, request, srcbuf, dstbuf,
                      count, dtype, op, root);
          if(isIdle())
          {
            allreduce->reset();
          }
          else
          {
            // We have a dstbuf now, need to reset that much but not full reset()
            allreduce->resetDstBuf();
#ifdef CCMI_DEBUG
            TRACE_ADAPTOR((stderr,"count %#X == %#X\n",count,_count));
            TRACE_ADAPTOR((stderr,"dtype %#X == %#X\n",dtype,_dt));
            TRACE_ADAPTOR((stderr,"op %#X == %#X\n",op,_op));
            //TRACE_ADAPTOR((stderr,"iter %#X == %#X\n",iteration,_iteration));
            TRACE_ADAPTOR((stderr,"root %#X == %#X\n",root,_root));
#endif
          }
          if(isDone()) setIdle();
          allreduce->setConsistency(consistency);

          _doneCountdown = 1; // default to just a composite done needed

          allreduce->start();

          if(!isDone()) setStarted();

          return XMI_SUCCESS;
        }

        ///
        /// \brief At this level we only support single color
        /// collectives
        ///
        virtual unsigned restartAsync ( CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * allreduce,
                                        unsigned                    count,
                                        xmi_dt                      dtype,
                                        xmi_op                      op,
                                        int                         root=-1)
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::AsyncComposite::restartAsync() ALERT:\n",this));
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::restartAsync()\n",this));

          initialize (allreduce,(XMI_CollectiveRequest_t  *) NULL, NULL, NULL,
                      count, dtype, op, root);
          if(isIdle())
          {
            allreduce->reset();
          }

          _doneCountdown = 1; // default to just a composite done needed

          setQueueing();

          return XMI_SUCCESS;
        }

        virtual void start()
        {
          CCMI_assert(_asyncState != _isStarted);
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::start()\n",this));
          setStarted();
          getExecutor(0)->start();
        }

        void done()
        {
          _doneCountdown --;
          TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::done() "
                         "_doneCountdown:%#X %#X/%#X \n",this,
                         _doneCountdown,(int)_myClientFunction,
                         (int)_myClientData));
          if(!_doneCountdown)  //allreduce done and (maybe) barrier done
          {
            setDone();
            if(_myClientFunction) (*_myClientFunction) (_myClientData, NULL);
            ((CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *) getExecutor(0))->getAllreduceState()->freeAllocations(_flags.reuse_storage_limit);
            TRACE_ADAPTOR((stderr,"<%p>Allreduce::AsyncComposite::DONE() \n",
                           this));
          }
        }

        ///
        /// \brief The default done call back to be called when
        /// [all]reduce finishes
        ///
        /// It means this composite (and kernel executor) is done
        ///
        static void cb_compositeDone(void *me, xmi_result_t *err)
        {
          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::AsyncComposite::cb_compositeDone()\n",
                         (int)me));
          AsyncComposite *composite = (AsyncComposite *) me;
          composite->done();
          TRACE_ADAPTOR((stderr,
                         "<%p>Allreduce::AsyncComposite::cb_compositeDone() 2\n",
                         (int)me));
        }

      };  //-- AsyncComposite
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
