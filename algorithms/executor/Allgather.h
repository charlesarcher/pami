/**
 * \file algorithms/executor/Allgather.h
 * \brief ???
 */
#ifndef __algorithms_executor_Allgather_h__
#define __algorithms_executor_Allgather_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "common/default/Topology.h"

#define MAX_CONCURRENT 32
#define MAX_PARALLEL 20

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements an allgather strategy
     */
    template<class T_ConnMgr, class T_Schedule>
    class AllgatherExec : public Interfaces::Executor
    {
      public:

        struct RecvStruct {
           pami_task_t         rank;
           int                 subsize;
           PAMI::PipeWorkQueue pwq;
        };

        struct PhaseRecvStr {
           int             donecount;
           int             partnercnt;
           AllgatherExec   *exec;
           RecvStruct      recvstr[MAX_CONCURRENT];
        };

      protected:
        T_Schedule                     * _comm_schedule;
        Interfaces::NativeInterface    * _native;
        T_ConnMgr                      * _connmgr;

        int                 _comm;
        int                 _buflen;   // byte count of a single message, not really buffer length
        char                *_sbuf;
        char                *_rbuf;
        char                *_tmpbuf;

        PhaseRecvStr        *_mrecvstr;

        int                 _curphase;
        int                 _nphases;
        int                 _startphase;
        int                 _donecount;

        int                 _maxsrcs;
        pami_task_t         _dstranks [MAX_CONCURRENT];
        unsigned            _dstlens  [MAX_CONCURRENT];
        pami_task_t         _srcranks [MAX_CONCURRENT];
        unsigned            _srclens  [MAX_CONCURRENT];
        PAMI::Topology      _selftopology;
        PAMI::Topology      _dsttopology [MAX_CONCURRENT];
        PAMI::Topology      *_gtopology;

        PAMI::PipeWorkQueue _pwq [MAX_CONCURRENT];
        CollHeaderData      _mdata [MAX_CONCURRENT];
        pami_multicast_t    _msend [MAX_CONCURRENT];

        //Private method
        void             sendNext ();

      public:
        AllgatherExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL),
            _comm(-1),
            _sbuf(NULL),
            _rbuf(NULL),
            _tmpbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgatherExec()\n", this));
        }

        AllgatherExec (Interfaces::NativeInterface  * mf,
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
            _tmpbuf(NULL),
	    _mrecvstr(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _selftopology(mf->myrank()),
            _gtopology(gtopology)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgatherExec(...)\n", this));
          _clientdata        =  0;
          _buflen            =  0;
        }

        virtual ~AllgatherExec ()
        {
           /// Todo: convert this to allocator ?
           if (_maxsrcs) free (_mrecvstr);
           if ((unsigned)_nphases != _native->numranks() - 1) free (_tmpbuf); // what about the multiport case ???
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
        }

        // --  Initialization routines
        //------------------------------------------

        void setSchedule (T_Schedule *ct)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgatherExec::setSchedule()\n", this));
          _comm_schedule = ct;
          // initialize schedule as if everybody is root
          _comm_schedule->init (_native->myrank(), CCMI::Schedule::SCATTER, _startphase, _nphases, _maxsrcs);
          CCMI_assert(_startphase == 0);
          CCMI_assert(_maxsrcs != 0);
          CCMI_assert(_maxsrcs <= MAX_CONCURRENT);
          CCMI_assert(_nphases <= MAX_PARALLEL);

          _mrecvstr = (PhaseRecvStr *) malloc (_nphases * sizeof(PhaseRecvStr)) ;
          for (int i = 0; i < _nphases; ++i) {
            _mrecvstr[i].donecount  = 0;
            _mrecvstr[i].partnercnt = 0;
            _mrecvstr[i].exec       = NULL;
          }

          for (int i = 0; i < _maxsrcs; ++i) {
            _mdata[i]._comm       = _comm;
            _mdata[i]._root       = -1; // not used on allgather
            _mdata[i]._count      = 0;
            _mdata[i]._phase      = 0;

            pami_quad_t *info    =  (pami_quad_t*)((void*) & _mdata);
            _msend[i].msginfo       =  info;
            _msend[i].msgcount      =  1;
            _msend[i].roles         = -1U;
          }

          unsigned connection_id = (unsigned) -1;
          if (_connmgr)
            connection_id = _connmgr->getConnectionId(_comm, (unsigned)-1, 0, (unsigned) - 1, (unsigned) - 1);
        }

        void setConnectionID (unsigned cid)
        {

          CCMI_assert(_comm_schedule != NULL);

          //Override the connection id from the connection manager
          for (int i=0; i<MAX_CONCURRENT; ++i)
            _msend[i].connection_id = cid;

        }

        void  updateBuffers(char *src, char *dst, int len)
        {
          _buflen = len;
          _sbuf   = src;
          _rbuf   = dst;
        }

        void  setBuffers (char *src, char *dst, int len)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AllgatherExec::setInfo() src %p, dst %p, len %d, _pwq %p\n", this, src, dst, len, &_pwq));

          _buflen = len;
          _sbuf = src;
          _rbuf = dst;

          CCMI_assert(_comm_schedule != NULL);
          size_t buflen = _native->numranks() * len;
          _tmpbuf = (char *) malloc(buflen);
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
        unsigned       getComm   ()
        {
          return _comm;
        }

        static void notifySendDone (pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE_MSG ((stderr, "<%p>Executor::AllgatherExec::notifySendDone()\n", cookie));
          AllgatherExec<T_ConnMgr, T_Schedule> *exec =  (AllgatherExec<T_ConnMgr, T_Schedule> *) cookie;
          exec->_donecount --;
          if (exec->_donecount == 0) {
            exec->_mrecvstr[exec->_curphase].donecount -= exec->_mrecvstr[exec->_curphase].partnercnt;
            if (exec->_mrecvstr[exec->_curphase].donecount == 0) {
              exec->_mrecvstr[exec->_curphase].partnercnt = 0;
              exec->_curphase  ++;
              exec->_donecount  = 0;
              exec->sendNext();
            }
          }
        }

        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::AllgatherExec::notifyRecvDone()\n", cookie));
          PhaseRecvStr  *mrecv = (PhaseRecvStr *) cookie;
          mrecv->donecount ++;
          AllgatherExec<T_ConnMgr, T_Schedule> *exec =  mrecv->exec;
          if (mrecv->donecount == 0){
            exec->_curphase  ++;
            exec->_donecount  = 0;
            exec->sendNext();
          }
        }


    };  //-- AllgatherExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending allgather data. Only active on the root node
///
template <class T_ConnMgr, class T_Schedule>
inline void  CCMI::Executor::AllgatherExec<T_ConnMgr, T_Schedule>::start ()
{
  TRACE_ADAPTOR((stderr, "<%p>Executor::AllgatherExec::start() count%d\n", this, _buflen));

  // Nothing to gather? We're done.
  if ((_buflen == 0) && _cb_done)
    {
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
      return;
    }

  memcpy(_tmpbuf, _sbuf, _buflen);

  _curphase   = _startphase;
  _donecount  = 0;
  sendNext ();
}

template <class T_ConnMgr, class T_Schedule>
inline void  CCMI::Executor::AllgatherExec<T_ConnMgr, T_Schedule>::sendNext ()
{
  CCMI_assert(_comm_schedule != NULL);
  CCMI_assert(_donecount  == 0);

  if (_curphase < _startphase + _nphases) {

    unsigned ndsts, nsrcs;
    _comm_schedule->getList(_curphase, &_srcranks[0], nsrcs, &_dstranks[0], ndsts, &_srclens[0], &_dstlens[0]);
    CCMI_assert(nsrcs == ndsts);

    for (unsigned i = 0; i < nsrcs; ++i) {
      new (&_dsttopology[i]) PAMI::Topology(_dstranks[i]);

      size_t buflen = _dstlens[i] * _buflen;
      _pwq[i].configure (NULL, _tmpbuf, buflen, 0);
      _pwq[i].reset();
      _pwq[i].produceBytes(buflen);

      _mdata[i]._phase             = _curphase;
      _mdata[i]._count             = _buflen;
      _msend[i].src_participants   = (pami_topology_t *) & _selftopology;
      _msend[i].dst_participants   = (pami_topology_t *) & _dsttopology[i];
      _msend[i].cb_done.function   = notifySendDone;
      _msend[i].cb_done.clientdata = this;
      _msend[i].src                = (pami_pipeworkqueue_t *) & _pwq[i];
      _msend[i].dst                = NULL;
      _msend[i].bytes              = buflen;
      _native->multicast(&_msend[i]);
    }

    if (_mrecvstr[_curphase].exec == NULL) {
      CCMI_assert(_mrecvstr[_curphase].donecount == 0);
      for (unsigned i = 0; i < nsrcs; ++i) {
        size_t buflen       = _srclens[i] * _buflen;
        unsigned myindex    = _gtopology->rank2Index(_native->myrank());
        unsigned srcindex   = _gtopology->rank2Index(_srcranks[i]);
        unsigned dist       = (srcindex + _native->numranks() - myindex)% _native->numranks();
        RecvStruct *recvstr = &_mrecvstr[_curphase].recvstr[i];
        recvstr->pwq.configure (NULL, _tmpbuf + dist * _buflen, buflen, 0);
        recvstr->pwq.reset();
        recvstr->subsize = buflen;
        recvstr->rank    = _srcranks[i];
      }
      _mrecvstr[_curphase].partnercnt = nsrcs;
      _mrecvstr[_curphase].exec       = this;
    }

    _donecount = ndsts;

    return;
  }

  memcpy (_rbuf + (_native->myrank()+1) * _buflen, _tmpbuf, (_native->numranks() - _native->myrank()-1)* _buflen);
  memcpy (_rbuf, _tmpbuf+(_native->numranks() - _native->myrank())*_buflen, _native->myrank() * _buflen);

  if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);
  return;
}

template <class T_ConnMgr, class T_Schedule>
inline void  CCMI::Executor::AllgatherExec<T_ConnMgr, T_Schedule>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{

  CollHeaderData *cdata = (CollHeaderData*) &info;

  unsigned srcindex=0;
  unsigned nsrcs;

  if (_mrecvstr[cdata->_phase].exec == NULL) {
    CCMI_assert(_mrecvstr[cdata->_phase].donecount == 0);

    _comm_schedule->getRList(_nphases - cdata->_phase - 1, &_srcranks[0], nsrcs, &_srclens[0]);
    for (unsigned i = 0; i < nsrcs; ++i) {
      size_t buflen       = _srclens[i] * _buflen;
      unsigned myindex    = _gtopology->rank2Index(_native->myrank());
      srcindex            = _gtopology->rank2Index(_srcranks[i]);
      unsigned dist       = (srcindex + _native->numranks() - myindex)% _native->numranks();
      RecvStruct *recvstr = &_mrecvstr[_curphase].recvstr[i];
      recvstr->pwq.configure (NULL, _tmpbuf + dist * _buflen, buflen, 0);
      recvstr->pwq.reset();
      recvstr->subsize = buflen;
      recvstr->rank    = _srcranks[i];
      if (_srcranks[i] == src) {
        srcindex = i;
      }
    }
    _mrecvstr[cdata->_phase].exec       = this;
    _mrecvstr[cdata->_phase].partnercnt = 1; // this could be a problem ???
  } else {
    for (int i = 0; i < _mrecvstr[cdata->_phase].partnercnt; ++i)
      if (src == _mrecvstr[cdata->_phase].recvstr[i].rank) {
        srcindex = i;
        break;
      }
  }

  //CCMI_assert(myindex < nsrcs);

  *pwq = &_mrecvstr[cdata->_phase].recvstr[srcindex].pwq;

  cb_done->function = notifyRecvDone;
  cb_done->clientdata = &_mrecvstr[cdata->_phase];
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//