/**
 * \file algorithms/executor/Allgatherv.h
 * \brief ???
 */
#ifndef __algorithms_executor_Allgatherv_h__
#define __algorithms_executor_Allgatherv_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"

#define MAX_CONCURRENT 32
#define MAX_PARALLEL 20

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements a allgatherv strategy which uses one network link.
     */

      template <class T_Allgather_type>
      struct AllgatherVecType
      {
         //COMPILE_TIME_ASSERT(0==1);
      };

      template<>
      struct AllgatherVecType<pami_allgather_t>
      {
         typedef int base_type;
      };

      template<>
      struct AllgatherVecType<pami_allgatherv_t> {
         typedef size_t base_type;
      };

      template<>
      struct AllgatherVecType<pami_allgatherv_int_t> {
         typedef int base_type;
      };

      template <class T_Allgather_type>
      inline void setAllgatherVec(T_Allgather_type *xfer, void *rdisps, void *rcounts)
      {
         COMPILE_TIME_ASSERT(0==1);
      }

      template <>
      inline void setAllgatherVec<pami_allgather_t> (pami_allgather_t *xfer,
                void *rdisps, void *rcounts)
      {
         return;
      }

      template <>
      inline void setAllgatherVec<pami_allgatherv_t> (pami_allgatherv_t *xfer,
                void *rdisps, void *rcounts)
      {
         *((size_t **)rdisps)   = xfer->rdispls;
         *((size_t **)rcounts)  = xfer->rtypecounts;
         return;
      }

      template <>
      inline void setAllgatherVec<pami_allgatherv_int_t> (pami_allgatherv_int_t *xfer,
                void *rdisps, void *rcounts)
      {
         *((int **)rdisps)   = xfer->rdispls;
         *((int **)rcounts)  = xfer->rtypecounts;
         return;
      }

    template<class T_ConnMgr, class T_Type>
    class AllgathervExec : public Interfaces::Executor
    {
      public:

      protected:
        Interfaces::Schedule           * _comm_schedule;
        Interfaces::NativeInterface    * _native;
        T_ConnMgr                      * _connmgr;

        int                 _comm;
        int                 _buflen;
        char                *_sbuf;
        char                *_rbuf;

        PAMI::PipeWorkQueue _pwq;
        PAMI::PipeWorkQueue _rpwq;

        int                 _curphase;
        int                 _nphases;
        int                 _startphase;
        int                 _lphase;
        int                 _rphase;

        int                 _maxsrcs;

        int                 _senddone;
        int                 _recvdone;
        pami_task_t         _src;
        pami_task_t         _dst;
        PAMI::Topology      _dsttopology;
        PAMI::Topology      _srctopology;
        PAMI::Topology      _selftopology;
        PAMI::Topology      *_gtopology;

        CollHeaderData      _mldata;
        CollHeaderData      _mrdata;
        pami_multicast_t    _mlsend;
        pami_multicast_t    _mrsend;

        typedef typename AllgatherVecType<T_Type>::base_type basetype;

        basetype                *_disps;
        basetype                *_rcvcounts;

        //Private method
        void             sendNext ();

      public:
        AllgathervExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL),
            _comm(-1),
            _sbuf(NULL),
            _rbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _disps(NULL),
            _rcvcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgathervExec()\n", this));
        }

        AllgathervExec (Interfaces::NativeInterface  * mf,
                       T_ConnMgr                    * connmgr,
                       unsigned                       comm,
                       PAMI::Topology               *gtopology) :
            Interfaces::Executor(),
            _comm_schedule (NULL),
            _native(mf),
            _connmgr(connmgr),
            _comm(comm),
            _sbuf(NULL),
            _rbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _dsttopology(),
            _srctopology(),
            _selftopology(mf->myrank()),
            _gtopology(gtopology),
            _disps(NULL),
            _rcvcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgathervExec(...)\n", this));
          _clientdata        =  0;
          _buflen            =  0;

          _senddone  = _recvdone = 0;

          _mldata._comm       = _comm;
          _mldata._root       = -1;
          _mldata._count      = -1; // indicating this is only a sync message
          _mldata._phase      = 0;

          pami_quad_t *info    =  (pami_quad_t*)((void*) & _mldata);
          _mlsend.msginfo       =  info;
          _mlsend.msgcount      =  1;
          _mlsend.roles         = -1U;

          _mrdata._comm       = _comm;
          _mrdata._root       = -1;
          _mrdata._count      = -1;
          _mrdata._phase      = 0;

          info                  =  (pami_quad_t*)((void*) & _mrdata);
          _mrsend.msginfo       =  info;
          _mrsend.msgcount      =  1;
          _mrsend.roles         = -1U;

        }

        virtual ~AllgathervExec ()
        {
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
        }

        // --  Initialization routines
        //------------------------------------------

        void setSchedule (Interfaces::Schedule *ct)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgathervExec::setSchedule()\n", this));
          _comm_schedule = ct;

          _nphases    = _native->numranks() - 1;
          _startphase = 0;
          _curphase   = 0;
          _lphase     = 0;
          _rphase     = 0;

          unsigned connection_id = (unsigned) -1;
          if (_connmgr)
            connection_id = _connmgr->getConnectionId(_comm, (unsigned)-1, 0, (unsigned) - 1, (unsigned) - 1);
        }

        void setConnectionID (unsigned cid)
        {

          //Override the connection id from the connection manager
          _mlsend.connection_id = cid;
          _mrsend.connection_id = cid;

        }

        void  updateBuffers(char *src, char *dst, int len)
        {
          _buflen = len;
          _sbuf   = src;
          _rbuf   = dst;
        }

        void  setBuffers (char *src, char *dst, int len)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgathervExec::setInfo() src %p, dst %p, len %d, _pwq %p\n", this, src, dst, len, &_pwq));

          _buflen = len;
          _sbuf = src;
          _rbuf = dst;

          // setup send PWQ and destination topology
          // what is myrank ??? rank in world geometry or index in topology ???
          unsigned myindex  = _gtopology->rank2Index(_native->myrank());

          unsigned dstindex = (myindex + 1) % _native->numranks();
          _dst              = _gtopology->index2Rank(dstindex);
          new (&_dsttopology) PAMI::Topology(_dst);
          unsigned srcindex = (myindex + _native->numranks() - 1) % _native->numranks();
          _src              = _gtopology->index2Rank(srcindex);
          new (&_srctopology) PAMI::Topology(_src);

        }

        void setVectors(int *disps, int *rcvcounts)
        {
           _disps     = disps;
           _rcvcounts = rcvcounts;
        }

        void setVectors(T_Type *xfer)
        {
           setAllgatherVec<T_Type> (xfer, _disps, _rcvcounts);
        }

        void  updateVectors(T_Type *xfer)
        {
           setAllgatherVec<T_Type> (xfer, _disps, _rcvcounts);
        }


        size_t getSendLength(int phase)
        {
           int index = (_native->myrank() + _native->numranks() - phase) % _native->numranks();
           return (_rcvcounts) ? _rcvcounts[index] : _buflen;
        }

        size_t getRecvLength(int phase)
        {
           int index = (_native->myrank() +  phase + 1) % _native->numranks();
           return (_rcvcounts) ? _rcvcounts[index] : _buflen;
        }

        size_t getSendDisp(int phase)
        {
           int index = (_native->myrank() + _native->numranks() - phase) % _native->numranks();
           return (_disps) ? _disps[index] : index * _buflen;
        }

        size_t getRecvDisp(int phase)
        {
           int index = (_native->myrank() +  phase + 1) % _native->numranks();
           return (_disps) ? _disps[index] : index * _buflen;
        }

        PAMI::PipeWorkQueue *getSendPWQ(int phase)
        {
          size_t sleng = getSendLength(phase);
          size_t sdisp = getSendDisp(phase);
          _pwq.configure (NULL, _rbuf + sdisp, sleng, 0);
          _pwq.reset();
          _pwq.produceBytes(sleng);
          return &_pwq;
        }

        PAMI::PipeWorkQueue *getRecvPWQ(int phase)
        {
          size_t rleng = getRecvLength(phase);
          size_t rdisp = getRecvDisp(phase);
          _rpwq.configure (NULL, _rbuf + rdisp, rleng, 0);
          _rpwq.reset();
          return &_rpwq;
        }

        //------------------------------------------
        // -- Executor Virtual Methods
        //------------------------------------------
        virtual void   start          ();
        virtual void   notifyRecv     (unsigned             src,
                                       const pami_quad_t   & info,
                                       PAMI::PipeWorkQueue ** pwq,
                                       pami_callback_t      * cb_done);

        //-----------------------------------------
        //--  Query functions ---------------------
        //-----------------------------------------
        /*
        unsigned       getRoot   ()
        {
          return _root;
        }
        */
        unsigned       getComm   ()
        {
          return _comm;
        }

        static void notifySendDone (pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE_MSG ((stderr, "<%p>Executor::AllgathervExec::notifySendDone()\n", cookie));
          AllgathervExec<T_ConnMgr, T_Type> *exec =  (AllgathervExec<T_ConnMgr, T_Type> *) cookie;
          exec->_senddone = 1;
          if (exec->_recvdone == 1) {
            exec->_recvdone = exec->_senddone = 0;
            exec->_curphase ++;
            exec->sendNext();
          }
        }

        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::AllgathervExec::notifyRecvDone()\n", cookie));
          AllgathervExec<T_ConnMgr, T_Type> *exec =  (AllgathervExec<T_ConnMgr, T_Type> *) cookie;
          exec->_recvdone = 1;
          if (exec->_senddone == 1) {
            exec->_recvdone = exec->_senddone = 0;
            exec->_curphase ++;
            exec->sendNext();
          }
        }

        static void notifyAvailRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::AllgathervExec::notifyRecvDone()\n", cookie));
          AllgathervExec<T_ConnMgr, T_Type> *exec =  (AllgathervExec<T_ConnMgr, T_Type> *) cookie;
          exec->_rphase ++;
          exec->sendNext();
        }


    };  //-- AllgathervExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending allgatherv data. Only active on the root node
///
template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AllgathervExec<T_ConnMgr, T_Type>::start ()
{
  TRACE_ADAPTOR((stderr, "<%p>Executor::AllgathervExec::start() count%d\n", this, _buflen));

  _curphase  = _startphase;
  _lphase    = _curphase + 1;

  // what is myrank ? in world geometry or in this geometry ?
  unsigned myindex = _native->myrank();
  memcpy(_rbuf + _disps[myindex], _sbuf, _rcvcounts[myindex]);

  sendNext ();
}

template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AllgathervExec<T_ConnMgr, T_Type>::sendNext ()
{
  if (_curphase == _startphase + _nphases) {
    if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);
    return;
  }

  // send buffer available msg to left neighbor
  if (_lphase == _curphase+1) {
    _lphase ++;
    _mldata._phase             = _curphase+1;
    _mlsend.src_participants   = (pami_topology_t *) & _selftopology;
    _mlsend.dst_participants   = (pami_topology_t *) & _srctopology;
    _mlsend.cb_done.function   = NULL;
    _mlsend.cb_done.clientdata = 0;
    _mlsend.src                = NULL;
    _mlsend.dst                = NULL;
    _mlsend.bytes              = 0;
    _native->multicast(&_mlsend);
  }

  if (_rphase == _curphase+1) { // buffer available at the right neighbor
    _mrdata._phase             = _curphase;
    _mrdata._count             = 0; // indicating this is data message
    _mrsend.src_participants   = (pami_topology_t *) & _selftopology;
    _mrsend.dst_participants   = (pami_topology_t *) & _dsttopology;
    _mrsend.cb_done.function   = notifySendDone;
    _mrsend.cb_done.clientdata = this;
    _mrsend.src                = (pami_pipeworkqueue_t *) getSendPWQ(_curphase);
    _mrsend.dst                = NULL;
    _mrsend.bytes              = getSendLength(_curphase);
    _native->multicast(&_mrsend);
  }

  return;
}

template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AllgathervExec<T_ConnMgr, T_Type>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{

  CollHeaderData *cdata = (CollHeaderData*) &info;

  if ((int)cdata->_count == -1) {
    CCMI_assert(src == _src);
    if (_rphase == _curphase) {
      CCMI_assert(cdata->_phase == _curphase);
    } else if (_rphase == _curphase+1) {
      CCMI_assert(cdata->_phase == _curphase+1);
    } else {
      CCMI_assert(0);
    }
    *pwq = NULL;
    cb_done->function   = notifyAvailRecvDone;
    cb_done->clientdata = this;
  } else {
    CCMI_assert(src == _dst);
    CCMI_assert(cdata->_phase == _curphase);
    CCMI_assert(cdata->_count == 0);
    *pwq = getRecvPWQ(_curphase);
    cb_done->function   = notifyRecvDone;
    cb_done->clientdata = this;
  }

  return;
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//