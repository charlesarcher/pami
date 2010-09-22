/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/scatter/AsyncScatterT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_scatter_AsyncScatterT_h__
#define __algorithms_protocols_scatter_AsyncScatterT_h__

#include "algorithms/ccmi.h"
#include "algorithms/executor/Scatter.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/CollOpT.h"

#if defined DEBUG
#undef DEBUG
#define DEBUG(x) // fprintf x
#else
#define DEBUG(x) // fprintf x
#endif

namespace CCMI
{
  namespace Adaptor
  {
    namespace Scatter
    {
      ///
      /// \brief Asyc Scatter Composite. It is single color right now
      ///

      template <class T_Scatter_type>
      inline void getScatterXfer(T_Scatter_type **xfer, pami_collective_t *coll)
      {
         COMPILE_TIME_ASSERT(0==1);
      }

      template <>
      inline void getScatterXfer<pami_scatter_t>(pami_scatter_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_scatter);
      }

      template <>
      inline void getScatterXfer<pami_scatterv_t>(pami_scatterv_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_scatterv);
      }

      template <>
      inline void getScatterXfer<pami_scatterv_int_t>(pami_scatterv_int_t **xfer, pami_collective_t *coll)
      {
         *xfer =  &(coll->xfer_scatterv_int);
      }

      template <class T_Scatter_type>
      inline void setTempScatterXfer(pami_collective_t *xfer)
      {
         COMPILE_TIME_ASSERT(0 == 1);
      }

      template <>
      inline void setTempScatterXfer<pami_scatter_t> (pami_collective_t *xfer)
      {
         xfer->xfer_scatter.root   = -1;
         xfer->xfer_scatter.sndbuf = NULL;
         xfer->xfer_scatter.stype  = PAMI_BYTE;
         xfer->xfer_scatter.stypecount = 0;
         xfer->xfer_scatter.rcvbuf = NULL;
         xfer->xfer_scatter.rtype  = PAMI_BYTE;
         xfer->xfer_scatter.rtypecount = 0;
      }

      template <>
      inline void setTempScatterXfer<pami_scatterv_t> (pami_collective_t *xfer)
      {
         xfer->xfer_scatterv.root    = -1;
         xfer->xfer_scatterv.sndbuf  = NULL;
         xfer->xfer_scatterv.stype   = PAMI_BYTE;
         xfer->xfer_scatterv.stypecounts = NULL;
         xfer->xfer_scatterv.sdispls = NULL;
         xfer->xfer_scatterv.rcvbuf  = NULL;
         xfer->xfer_scatterv.rtype   = PAMI_BYTE;
         xfer->xfer_scatterv.rtypecount = 0;
      }

      template <>
      inline void setTempScatterXfer<pami_scatterv_int_t> (pami_collective_t *xfer)
      {
         xfer->xfer_scatterv_int.root    = -1;
         xfer->xfer_scatterv_int.sndbuf  = NULL;
         xfer->xfer_scatterv_int.stype   = PAMI_BYTE;
         xfer->xfer_scatterv_int.stypecounts = NULL;
         xfer->xfer_scatterv_int.sdispls = NULL;
         xfer->xfer_scatterv_int.rcvbuf  = NULL;
         xfer->xfer_scatterv_int.rtype   = PAMI_BYTE;
         xfer->xfer_scatterv_int.rtypecount = 0;
      }

      template <class T_Schedule, class T_Conn, ScheduleFn create_schedule, typename T_Scatter_type>
      class AsyncScatterT : public CCMI::Executor::Composite
      {
        protected:
          CCMI::Executor::ScatterExec<T_Conn, T_Schedule, T_Scatter_type>  _executor __attribute__((__aligned__(16)));
          T_Schedule                             _schedule;

        public:
          ///
          /// \brief Constructor
          ///
          AsyncScatterT ()
          {
          };
          AsyncScatterT (Interfaces::NativeInterface   * native,
                           T_Conn                        * cmgr,
                           pami_callback_t                  cb_done,
                           PAMI_GEOMETRY_CLASS            * geometry,
                           void                           *cmd) :





              Executor::Composite(),
              _executor (native, cmgr, geometry->comm(), (PAMI::Topology*)geometry->getTopology(0))
          {
            TRACE_ADAPTOR ((stderr, "<%p>Scatter::AsyncScatterT() \n", this));

            T_Scatter_type *s_xfer;
            getScatterXfer<T_Scatter_type>(&s_xfer, &((pami_xfer_t *)cmd)->cmd);

            /// \todo add datatype support !
            // CCMI_assert(s_xfer->stypecount == s_xfer->rtypecount);
            unsigned bytes = s_xfer->rtypecount;

            COMPILE_TIME_ASSERT(sizeof(_schedule) >= sizeof(T_Schedule));
            create_schedule(&_schedule, sizeof(_schedule), s_xfer->root, native, geometry);
            _executor.setRoot (s_xfer->root);
            _executor.setSchedule (&_schedule);

            _executor.setBuffers (s_xfer->sndbuf, s_xfer->rcvbuf, bytes);
            _executor.setVectors (s_xfer);
            _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
          }

          CCMI::Executor::ScatterExec<T_Conn, T_Schedule, T_Scatter_type> &executor()
          {
            return _executor;
          }

          typedef T_Scatter_type xfer_type;
      }; //- AsyncScatterT


      template <class T_Composite, MetaDataFn get_metadata, class C,  ConnectionManager::GetKeyFn getKey>
      class AsyncScatterFactoryT: public CollectiveProtocolFactory
      {
        protected:
          ///
          /// \brief get geometry from comm id
          ///
          pami_mapidtogeometry_fn      _cb_geometry;

          ///
          /// \brief free memory pool for async scatter operation objects
          ///
          CCMI::Adaptor::CollOpPoolT<pami_xfer_t,  T_Composite>   _free_pool;

          ///
          /// \brief memory allocator for early arrival descriptors
          ///
          PAMI::MemoryAllocator < sizeof(EADescriptor), 16 > _ead_allocator;

          ///
          /// \brief memory allocator for early arrival buffers
          ///
          PAMI::MemoryAllocator<32768, 16>                 _eab_allocator;

          C                                             * _cmgr;
          Interfaces::NativeInterface                   * _native;

        public:
          AsyncScatterFactoryT (C                           *cmgr,
                                  Interfaces::NativeInterface *native):
              CollectiveProtocolFactory(),
              _cmgr(cmgr),
              _native(native)
          {
            native->setMulticastDispatch(cb_async, this);
          }

          virtual ~AsyncScatterFactoryT ()
          {
          }

          /// NOTE: This is required to make "C" programs link successfully with virtual destructors
          void operator delete(void * p)
          {
            CCMI_abort();
          }

          //Override the connection manager in this call
          unsigned myGetKey   (unsigned                 root,
                             unsigned                 iconnid,
                             PAMI_GEOMETRY_CLASS    * geometry,
                             C                     ** connmgr)
          {
            return getKey(root,
                          iconnid,
                          geometry,
                          (ConnectionManager::BaseConnectionManager**)connmgr);
          }

          virtual void metadata(pami_metadata_t *mdata)
          {
            TRACE_ADAPTOR((stderr, "<%p>AsyncScatterFactoryT::metadata()\n",this));
            DO_DEBUG((templateName<MetaDataFn>()));
            get_metadata(mdata);
          }

          char *allocateBuffer (unsigned size)
          {
            if (size <= 32768)
              return(char *)_eab_allocator.allocateObject();

            char *buf = (char *)malloc(size);
            return buf;
          }

          void freeBuffer (unsigned size, char *buf)
          {
            if (size <= 32768)
              return _eab_allocator.returnObject(buf);

            free(buf);
          }

          typedef typename T_Composite::xfer_type scatter_type;

          virtual Executor::Composite * generate(pami_geometry_t              g,
                                                 void                      * cmd)
          {
            T_Composite* a_scatter = NULL;
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co = NULL;
            scatter_type *scatter_xfer;
            getScatterXfer<scatter_type>(&scatter_xfer, &((pami_xfer_t *)cmd)->cmd);

            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
            C *cmgr = _cmgr;
            unsigned key = getKey(scatter_xfer->root,
                                  (unsigned) - 1,
                                  (PAMI_GEOMETRY_CLASS*)g,
                                  (ConnectionManager::BaseConnectionManager**)&cmgr);

            if (_native->myrank() == scatter_xfer->root)
              {
                co = _free_pool.allocate(key);
                pami_callback_t  cb_exec_done;
                cb_exec_done.function   = exec_done;
                cb_exec_done.clientdata = co;

                a_scatter = new (co->getComposite())
                T_Composite ( _native,
                              cmgr,
                              cb_exec_done,
                              (PAMI_GEOMETRY_CLASS *)g,
                              (void *)cmd);

                co->setXfer((pami_xfer_t*)cmd);
                co->setFlag(LocalPosted);
                co->setFactory(this);

                //Use the Key as the connection ID
                if (cmgr == NULL)
                  a_scatter->executor().setConnectionID(key);

                DEBUG((stderr, "AsyncScatter: Root starts scatter operation %d\n", key);)
                a_scatter->executor().start();
              }
            else
              {
                co = (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)geometry->asyncCollectiveUnexpQ().findAndDelete(key);

                /// Try to match in active queue
                if (co)
                  {
                    CCMI_assert(co->getFlags() & EarlyArrival);

                    EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
                    CCMI_assert(ead != NULL);
                    CCMI_assert(ead->bytes == scatter_xfer->rtypecount);
                    CCMI_assert(ead->cdata._root == scatter_xfer->root);

                    if (ead->flag == EACOMPLETED)
                      {
                        DEBUG((stderr, "AsyncScatter: early arrival received key = %d\n", key);)
                        if (scatter_xfer->rtypecount)
                          {
                            char *eab = ead->buf;
                            CCMI_assert(eab != NULL);
                            memcpy (scatter_xfer->rcvbuf, eab, scatter_xfer->rtypecount);
                            freeBuffer(scatter_xfer->rtypecount, eab);
                            //_eab_allocator.returnObject(eab);
                          }

                        ead->flag = EANODATA;
                        co->getEAQ()->popTail();
                        _ead_allocator.returnObject(ead);

                        if (((pami_xfer_t *)cmd)->cb_done)
                          {
                            ((pami_xfer_t *)cmd)->cb_done(NULL, ((pami_xfer_t *)cmd)->cookie, PAMI_SUCCESS);
                          }

                        co->getComposite()->~T_Composite();
                        _free_pool.free(co);
                      }
                    else
                      {
                        DEBUG((stderr, "AsyncScatter: early arrival not completed yet, key = %d\n", key);)
                        co->setXfer((pami_xfer_t*)cmd);
                        co->setFlag(LocalPosted);
                        co->setFactory(this);

                        a_scatter = co->getComposite();
                      }
                  }
                /// not found posted CollOp object, create a new one and
                /// queue it in active queue
                else
                  {
                    DEBUG((stderr, "AsyncScatter: call-in does not find early arrival, key = %d\n", key);)
                    co = _free_pool.allocate(key);
                    pami_callback_t  cb_exec_done;
                    cb_exec_done.function   = exec_done;
                    cb_exec_done.clientdata = co;

                    a_scatter = new (co->getComposite())
                    T_Composite ( _native,
                                  cmgr,
                                  cb_exec_done,
                                  (PAMI_GEOMETRY_CLASS *)g,
                                  (void *)cmd);

                    co->setXfer((pami_xfer_t*)cmd);
                    co->setFlag(LocalPosted);
                    co->setFactory(this);

                    //Use the Key as the connection ID
                    if (cmgr == NULL)
                      a_scatter->executor().setConnectionID(key);

                    geometry->asyncCollectivePostQ().pushTail(co);
                  }

                //dev->unlock();
              }

            return NULL; //a_scatter;
          }

          static void cb_async
          (const pami_quad_t     * info,
           unsigned                count,
           unsigned                conn_id,
           size_t                  peer,
           size_t                  sndlen,
           void                  * arg,
           size_t                * rcvlen,
           pami_pipeworkqueue_t ** rcvpwq,
           pami_callback_t       * cb_done)
          {
            AsyncScatterFactoryT *factory = (AsyncScatterFactoryT *) arg;

            CollHeaderData *cdata = (CollHeaderData *) info;
            T_Composite* a_scatter = NULL;

            int comm = cdata->_comm;
            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *) PAMI_GEOMETRY_CLASS::getCachedGeometry(comm);

            if (geometry == NULL)
              {
                geometry = (PAMI_GEOMETRY_CLASS *) factory->getGeometry (comm);
                PAMI_GEOMETRY_CLASS::updateCachedGeometry(geometry, comm);
              }

            C *cmgr = factory->_cmgr;
            unsigned key = factory->myGetKey (cdata->_root, conn_id, geometry, &cmgr);
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *) geometry->asyncCollectivePostQ().findAndDelete(key);

            if (!co)
              {

                DEBUG((stderr, "AsyncScatter: cb_async for key %d - no posted local call-in\n", key);)
                co = factory->_free_pool.allocate(key);
                pami_callback_t cb_exec_done;
                cb_exec_done.function = exec_done;
                cb_exec_done.clientdata = co;

                pami_xfer_t a_xfer;
                setTempScatterXfer<scatter_type>(&(a_xfer.cmd));

                EADescriptor * ead = (EADescriptor *) factory->_ead_allocator.allocateObject();
                memcpy(&(ead->cdata), cdata, sizeof(cdata));
                ead->flag  = EASTARTED;
                // ead->bytes = sndlen;
                ead->bytes = cdata->_count;

                if (sndlen)
                  {
                    ead->buf   = (char *)factory->allocateBuffer(sndlen);//_eab_allocator.allocateObject();
                  }

                scatter_type *scatter_xfer;
                getScatterXfer<scatter_type>(&scatter_xfer, &(a_xfer.cmd));
                scatter_xfer->root       = cdata->_root;
                scatter_xfer->rcvbuf     = ead->buf;
                scatter_xfer->rtypecount = ead->bytes;

                a_scatter = new (co->getComposite())
                T_Composite ( factory->_native,
                              cmgr,
                              cb_exec_done,
                              geometry,
                              (void *)&a_xfer);
                co->getEAQ()->pushTail(ead);
                co->setFlag(EarlyArrival);
                co->setFactory (factory);

                //Use the Key as the connection ID
                if (cmgr == NULL)
                  a_scatter->executor().setConnectionID(key);

                geometry->asyncCollectiveUnexpQ().pushTail(co);
              }
            else
              {

                DEBUG((stderr, "AsyncScatter - cb_async for key %d: found posted call-in\n", key);)
                // use type count for now, need datatype handling !!!
                // CCMI_assert (co->getXfer()->type != PAMI_BYTE);

                // received message length (sndlen) may be greater than rtypecount)
                // CCMI_assert (co->getXfer()->cmd.xfer_scatter.rtypecount == cdata->_count);

                a_scatter = (T_Composite *) co->getComposite();
              }

            // This is tricky, unlike broadcast, the receive pwq could be a temporary buffer
            // different from the ea buffer or application buffer. The executor code is responsible
            // for moving data to those buffers
            a_scatter->executor().notifyRecv(peer, *info, (PAMI::PipeWorkQueue **)rcvpwq, cb_done);
            * rcvlen  = sndlen;

            return;
          }

          static void exec_done (pami_context_t context, void *cd, pami_result_t err)
          {
            CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> * co =
              (CCMI::Adaptor::CollOpT<pami_xfer_t, T_Composite> *)cd;

            DEBUG((stderr, "AsyncScatter: exec_done for key %d\n", co->key());)

            unsigned     flag = co->getFlags();

            if (flag & LocalPosted)
              {
                pami_xfer_t *xfer = co->getXfer();
                scatter_type *scatter_xfer;
                getScatterXfer<scatter_type>(&scatter_xfer, &(co->getXfer()->cmd));

                EADescriptor *ead = (EADescriptor *) co->getEAQ()->popTail();
                AsyncScatterFactoryT *factory = (AsyncScatterFactoryT *)co->getFactory();

                if (flag & EarlyArrival)
                  {
                    CCMI_assert(ead != NULL);

                    if (scatter_xfer->rtypecount)
                      {
                        char *eab = ead->buf;
                        CCMI_assert(eab != NULL);
                        memcpy (scatter_xfer->rcvbuf, eab, scatter_xfer->rtypecount);
                        factory->freeBuffer(scatter_xfer->rtypecount, eab); //_eab_allocator.returnObject(eab);
                      }

                    ead->flag = EANODATA;
                    ead->buf  = NULL;
                    factory->_ead_allocator.returnObject(ead);
                  }
                else
                  {
                    CCMI_assert(ead == NULL);
                  }

                if (xfer->cb_done)
                  xfer->cb_done(NULL, xfer->cookie, PAMI_SUCCESS);

                factory->_free_pool.free(co);
              }
            else if (flag & EarlyArrival)
              {
                EADescriptor *ead = (EADescriptor *) co->getEAQ()->peekTail();
                ead->flag = EACOMPLETED;
              }
            else
              {
                CCMI_assert(0);
              }
          }

      }; //- Async Composite Factory
    }  //- end namespace Scatter
  }  //- end namespace Adaptor
}  //- end CCMI


#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//