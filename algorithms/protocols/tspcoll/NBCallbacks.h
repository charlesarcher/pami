#ifndef ___collective_callbacks__
#define ___collective_callbacks__

/* *********************************************************************** */
/*                   incoming active message                               */
/* *********************************************************************** */
template <class T_Mcast>
inline xmi_quad_t * TSPColl::CollExchange<T_Mcast>::cb_incoming(const xmi_quad_t  * hdr,
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
  struct AMHeader * header = (struct AMHeader *) hdr;
  NBCollManager<T_Mcast> *mc = (NBCollManager<T_Mcast>*) arg;
  void * base0 =  mc->find (header->tag, header->id);
  if (base0 == NULL)
    CCMI_FATALERROR (-1, "incoming: cannot find coll=<%d,%d>",
		     header->tag, header->id);
  
  CollExchange * b = (CollExchange * ) ((char *)base0 + header->offset);
  TRACE((stderr, "INC  tag=%d id=%d ctr=%d phase=%d nphases=%d "
	 "msgctr=%d msgphase=%d\n",
	 header->tag, header->id, b->_counter, 
	 b->_phase, b->_numphases, 
	 header->counter, header->phase));

  assert (b->_header[0].id == header->id);
  assert (b->_numphases > 0);
  if (b->_strict)
    {
      if (header->counter != b->_counter || b->_phase >= b->_numphases) 
	b->internalerror (header, __LINE__);
    }
  
  b->_cmplt[header->phase].counter = header->counter;

  // multisend stuff
  *rcvbuf             = (char*)b->_rbuf[header->phase];
  *rcvlen             = sndlen;
  *pipewidth          = sndlen;
  cb_done->function   = CollExchange::cb_recvcomplete;
  cb_done->clientdata = &b->_cmplt[header->phase];
  
  //  *completionHandler = &CollExchange::cb_recvcomplete;
  //  *arg = &b->_cmplt[header->phase];
  //  __pgasrt_local_addr_t z = (__pgasrt_local_addr_t) b->_rbuf[header->phase];
  //  if (z == NULL) b->internalerror (header, __LINE__);
  //  return z;
  return (xmi_quad_t*)&b->_rreq[header->phase];
}



/* **************************************************************** */
/*            incoming active message                               */
/* **************************************************************** */
//__pgasrt_local_addr_t TSPColl::Scatter::
//cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
//	     void (** completionHandler)(void *, void *),
//	     void ** arg)
template<class T_Mcast>
xmi_quad_t * TSPColl::Scatter<T_Mcast>::cb_incoming(const xmi_quad_t  * hdr,
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
  struct scatter_header * header = (struct scatter_header *) hdr;
  NBCollManager<T_Mcast> *mc = (NBCollManager<T_Mcast>*) arg;
  void * base0 =  mc->find (header->tag, header->id);
  if (base0 == NULL)
    CCMI_FATALERROR (-1, "Scatter/v: <%d,%d> is undefined",
		     header->tag, header->id);
  Scatter * s = (Scatter * ) ((char *)base0 + header->tagoff);
  TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING base=%p ptr=%p\n", 
	 header->tag, header->id, base0, s));

    // multisend stuff
  *rcvbuf             = (char*)s->_rbuf;
  *rcvlen             = sndlen;
  *pipewidth          = sndlen;
  cb_done->function   = Scatter::cb_recvcomplete;
  cb_done->clientdata = s;
  
  TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING RETURING base=%p ptr=%p\n", 
	 header->tag, header->id, base0, s));
  

  return (xmi_quad_t*)&s->_rreq;

  //  *completionHandler = &Scatter::cb_recvcomplete;
  //  *arg = s;
  //  assert (s->_rbuf != NULL);
  //  return (__pgasrt_local_addr_t) s->_rbuf;
}




#endif
