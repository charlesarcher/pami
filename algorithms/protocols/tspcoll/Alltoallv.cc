#include "algorithms/protocols/tspcoll/Alltoallv.h"
#include "algorithms/protocols/tspcoll/Team.h"

/* **************************************************************** */
/*      start a new alltoallv. Old alltoallv has to be complete       */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoallv<T_NI>::reset (const void * s, void * d,
			       const size_t *scnts,
			       const size_t *sdispls,
			       const size_t *rcnts,
			       const size_t *rdispls)
{
  MUTEX_LOCK(&this->_mutex);
  //alltoall common
  this->_odd = (!this->_odd);
  this->_sndcount[this->_odd] = 0;
  this->_rcvcount[this->_odd] = 0;
  this->_rbuf           = (char *)d;
  this->_sbuf           = (const char *)s;
  //altoallv specific
  this->_scnts          = scnts;
  this->_sdispls        = sdispls;
  this->_rcnts          = rcnts;
  this->_rdispls        = rdispls;
  MUTEX_UNLOCK(&this->_mutex);
}

/* **************************************************************** */
/*                   do all the sends in an alltoall                */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoallv<T_NI>::kick    () {
  MUTEX_LOCK(&this->_mutex);
  for (int i=0; i < (int)this->_comm->size(); i++)
    if (i == (int)this->_comm->ordinal())
      {
	memcpy (this->_rbuf + this->_rdispls[i],
		this->_sbuf + this->_sdispls[i],
		this->_scnts[i]);

	this->_sndcount[this->_odd]++;
	this->_rcvcount[this->_odd]++;

	/* UNLOCK */
	MUTEX_UNLOCK(&this->_mutex);
	if (this->_sndcount[this->_odd] >= (int)this->_comm->size() &&
	    this->_rcvcount[this->_odd] >= (int)this->_comm->size())
	  if (this->_cb_complete)
	    this->_cb_complete (this->_pami_ctxt,this->_arg, PAMI_SUCCESS);
      }
    else {
      MUTEX_UNLOCK(&this->_mutex);
//      _headers[i].dest_ctxt = _comm->endpoint(i).ctxt;
      pami_send_t p_send;
      p_send.send.header.iov_base  = &(this->_headers[i]);
      p_send.send.header.iov_len   = sizeof(this->_headers[i]);
      p_send.send.data.iov_base    = (char*) this->_sbuf + this->_sdispls[i];
      p_send.send.data.iov_len     = this->_scnts[i];
      p_send.send.dispatch         = -1;
      memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
      p_send.send.dest             = this->_comm->endpoint (i);
      p_send.events.cookie         = this;
      p_send.events.local_fn       = this->cb_senddone;
      p_send.events.remote_fn      = NULL;
      this->_p2p_iface->send(&p_send);
    }
}

/* **************************************************************** */
/*               reception header handler                           */
/* **************************************************************** */
template<class T_NI>
inline void xlpgas::Alltoallv<T_NI>::cb_incoming_v(pami_context_t    context,
                                                   void            * cookie,
                                                   const void      * hdr,
                                                   size_t            header_size,
                                                   const void      * pipe_addr,
                                                   size_t            data_size,
                                                   pami_endpoint_t   origin,
                                                   pami_recv_t     * recv)
{
  struct Alltoall<T_NI>::AMHeader * header = (struct Alltoall<T_NI>::AMHeader *) hdr;
  //unused:   int ctxt = header->dest_ctxt;
  CollectiveManager<T_NI> *mc = (CollectiveManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->kind, header->tag);


  if (base0 == NULL)
    xlpgas_fatalerror (-1, "%d: Alltoallv<T_NI>/v: <%d,%d> is undefined",
                         XLPGAS_MYNODE, header->tag, header->kind);
  Alltoallv<T_NI> * s = (Alltoallv<T_NI> * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: ALLTOALL: <%d,%d> INCOMING base=%p ptr=%p len=%d\n",
         XLPGAS_MYNODE, header->tag, header->kind, base0, s, s->_len));

  char * rbuf =  s->_rbuf + s->_rdispls[header->senderID];
  if (pipe_addr)
    memcpy(rbuf, pipe_addr, data_size);
  else if (recv)
  {
    recv->cookie        = s;
    recv->local_fn      = Alltoallv<T_NI>::cb_recvcomplete;
    recv->addr          = rbuf;
    recv->type          = PAMI_TYPE_BYTE;
    recv->offset        = 0;
    recv->data_fn       = PAMI_DATA_COPY;
    recv->data_cookie   = (void*)NULL;
    TRACE((stderr, "ALLTOALLV: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
           header->tag, header->id, base0, s));
    return;
  }
  Alltoallv::cb_recvcomplete(context, s, PAMI_SUCCESS);
  return;
}