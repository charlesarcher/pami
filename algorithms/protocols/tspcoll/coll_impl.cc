#include "algorithms/protocols/tspcoll/Collective.h"
#include "algorithms/protocols/tspcoll/Team.h"
#include "algorithms/protocols/tspcoll/Collectives.h"

extern "C" void xlpgas_tspcoll_barrier (int ctxt, int commID)
{
  xlpgas::Collective * b;
  if(XLPGAS_SMPTHREADS <= 1){//specialized
     b = xlpgas::Team::get(ctxt, commID)->coll (xlpgas::BarrierKind);
  }
  else {//generic PP
    b = xlpgas::Team::get(ctxt, commID)->coll (xlpgas::BarrierPPKind);
  }
  assert (b != NULL);
  b->reset();
  b->kick();
  while (!b->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_allreduce   (int              ctxt,
					    int               teamID,
					    const void      * sbuf,
					    void            * rbuf,
					    xlpgas_ops_t      op,
					    xlpgas_dtypes_t   dtype,
					    unsigned          nelems,
					    user_func_t*      uf=NULL)
{
  xlpgas::Collective * a;
  if(op == XLPGAS_OP_UFUNC || op == XLPGAS_OP_MIN || XLPGAS_SMPTHREADS > 1){
    //generic version; specialized version can't handle user functions
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AllreducePPKind);
  }
  else{ //specialized
    if(nelems * xlpgas::Allreduce::datawidthof(dtype) > 512)//to be fixed for other platforms than PERCS
      a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::LongAllreduceKind);
    else
      a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::ShortAllreduceKind);
  }
  assert (a != NULL);
  a->reset (sbuf, rbuf, op, dtype, nelems, uf);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}


extern "C" void* xlpgas_tspcoll_allreduce_start   (int              ctxt,
						  int               teamID,
						  const void      * sbuf,
						  void            * rbuf,
						  xlpgas_ops_t      op,
						  xlpgas_dtypes_t   dtype,
						  unsigned          nelems,
						  user_func_t*      uf=NULL)
{
  xlpgas::Collective * a;
  if(nelems * xlpgas::Allreduce::datawidthof(dtype) > 512)//to be fixed for other platforms than PERCS
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::LongAllreduceKind);
  else
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::ShortAllreduceKind);
  assert (a != NULL);
  a->reset (sbuf, rbuf, op, dtype, nelems, uf);
  a->kick();
  return (void*)a;
}

extern "C" void xlpgas_tspcoll_allreduce_wait   (int ctxt, void* _coll) {
  xlpgas::Collective* a = (xlpgas::Collective*)_coll;
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_bcast  (int               ctxt,
					   int               teamID,
					   int               root,
                                           const void      * sbuf,
                                           void            * rbuf,
					   unsigned          nbytes)
{
  xlpgas::Collective * a;
  if(XLPGAS_SMPTHREADS <= 1){//specialized
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::BcastKind);
  }
  else {
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::BcastPPKind);
  }
  assert (a != NULL);
  a->reset (root, sbuf, rbuf, nbytes);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
    a->kick();
  }
}

extern "C" void xlpgas_tspcoll_bcast_amcoll  (int               ctxt,
					    int               teamID,
					    int               root,
					    const void      * sbuf,
					    void            * rbuf,
					    unsigned          nbytes)
{
  xlpgas::Collective * a;
  a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::BcastTreeKind);
  assert (a != NULL);
  a->reset (root, sbuf, rbuf, nbytes);
  a->kick();
  while (! a->isdone() ) {
    xlpgas_tsp_wait (ctxt);
  }

}

extern "C" void xlpgas_tspcoll_allgather  (int               ctxt,
					   int               teamID,
                                           const void      * sbuf,
                                           void            * rbuf,
					   unsigned          nbytes)
{
  xlpgas::Collective * a;
  if(XLPGAS_SMPTHREADS <= 1){//specialized
   a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AllgatherKind);
  }
  else {
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AllgatherPPKind);
  }
  assert (a != NULL);
  a->reset (sbuf, rbuf, nbytes);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_allgatherv  (int               ctxt,
					    int               teamID,
					    const void      * sbuf,
					    void            * rbuf,
					    size_t          * lengths)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AllgathervKind);
  assert (a != NULL);
  a->reset (sbuf, rbuf, lengths);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_alltoall  (int               ctxt,
					  int               teamID,
					  const void      * sbuf,
					  void            * rbuf,
					  unsigned          nbytes)
{
  xlpgas::Collective * a;
  if(XLPGAS_SMPTHREADS <= 1){//specialized
   a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AlltoallKind);
  }
  else {
    a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AlltoallPPKind);
  }
  assert (a != NULL);
  a->reset (sbuf, rbuf, nbytes);
  xlpgas_tspcoll_barrier(ctxt,teamID);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_alltoallv  (int               ctxt,
					   int               teamID,
					   const void      * sbuf,
					   void            * rbuf,
					   const size_t    * scnts,
					   const size_t    * sdispls,
					   const size_t    * rcnts,
					   const size_t    * rdispls)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::AlltoallvKind);
  assert (a != NULL);
  a->reset (sbuf, rbuf, scnts,sdispls,rcnts,rdispls);
  xlpgas_tspcoll_barrier(ctxt,teamID);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_gather  (int               ctxt,
					int               teamID,
					int root,
					const void      * sbuf,
					void            * rbuf,
					unsigned          nbytes)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::GatherKind);
  assert (a != NULL);
  a->reset (root, sbuf, rbuf, nbytes);
  xlpgas_tspcoll_barrier(ctxt,teamID);//!!!
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_scatter  (int               ctxt,
					   int               teamID,
					   int root,
					   const void      * sbuf,
					   void            * rbuf,
					   unsigned          nbytes)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::ScatterKind);
  assert (a != NULL);
  a->reset (root, sbuf, rbuf, nbytes);
  xlpgas_tspcoll_barrier(ctxt,teamID);//!!!
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}


extern "C" void xlpgas_tspcoll_permute  (int               ctxt,
					 int               teamID,
					 int               dest,
					 const void      * sbuf,
					 void            * rbuf,
					 unsigned          nbytes)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::PermuteKind);
  assert (a != NULL);
  a->reset (dest, sbuf, rbuf, nbytes);
  xlpgas_tspcoll_barrier(ctxt,teamID);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}


extern "C" void xlpgas_tspcoll_prefix_sums  (int               ctxt,
					     int               teamID,
					     const void      * sbuf,
					     void            * rbuf,
					     xlpgas_ops_t      op,
					     xlpgas_dtypes_t   dtype,
					     unsigned          nelems)
{
  xlpgas::Collective * a = xlpgas::Team::get (ctxt, teamID)->coll (xlpgas::PrefixKind);
  assert (a != NULL);
  a->reset (sbuf, rbuf, op,dtype,nelems);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
}

extern "C" void xlpgas_tspcoll_am_bcast  (int               ctxt,
					  int               teamID,
					  AMHeader_bcast   *header,
					  int               root,
					  const void      * sbuf,
					  unsigned          nbytes)
{
  xlpgas::AMBcast* a = (xlpgas::AMBcast *) __global.heap_mm->malloc (sizeof(xlpgas::AMBcast));
  assert (a != NULL);
  memset (a, 0, sizeof(xlpgas::AMBcast));
  new (a) xlpgas::AMBcast (ctxt, teamID, header);
  a->reset (root, sbuf, nbytes);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
  a->clear();
  __global.heap_mm->free(a);
}


extern "C" void xlpgas_tspcoll_am_reduce  (int               ctxt,
					   int               teamID,
					   AMHeader_reduce * header,
					   int               root,
					   void            * sbuf,
					   unsigned          nbytes)
{
  xlpgas::AMReduce* a = (xlpgas::AMReduce *) __global.heap_mm->malloc (sizeof(xlpgas::AMReduce));
  assert (a != NULL);
  memset (a, 0, sizeof(xlpgas::AMReduce));
  new (a) xlpgas::AMReduce (ctxt, teamID, header);
  a->reset (root, sbuf, nbytes);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
  a->clear();
  __global.heap_mm->free(a);
}

extern "C" void xlpgas_tspcoll_am_gather  (int               ctxt,
					   int               teamID,
					   AMHeader_gather * header,
					   int               root,
					   void            * sbuf,
					   unsigned          nbytes)
{
  header->rlen = nbytes *  xlpgas::Team::get (ctxt, teamID)->size();
  xlpgas::AMGather* a = (xlpgas::AMGather *) __global.heap_mm->malloc (sizeof(xlpgas::AMGather));
  assert (a != NULL);
  memset (a, 0, sizeof(xlpgas::AMGather));
  new (a) xlpgas::AMGather (ctxt, teamID, header);
  a->reset (root, sbuf, nbytes);
  a->kick();
  while (!a->isdone()) {
    xlpgas_tsp_wait (ctxt);
  }
  a->clear();
  __global.heap_mm->free(a);
}


struct Split
{
  Split (int c, int r, int g, int n, int context) : color(c), rank(r), absrank(g), node(n), ctxt(context) {}

  //we may not need rank/absrank
  int color, rank, absrank, node, ctxt;
};

int __internal_tspcoll_split  (int ctxt,
			       int teamID,
			       int color,
			       int rank,
			       xlpgas_endpoint_t *proclist)
{
  xlpgas::Team* team =  xlpgas::Team::get (ctxt, teamID);

 /* -------------------------------------------------------------- */
  /* prepare local buffer for allgather operation                   */
  /* prepare destination buffer for allgather operation             */
  /* -------------------------------------------------------------- */
  Split * dstbuf = (Split *) __global.heap_mm->malloc (sizeof(Split) * team->size());
  Split   sndbuf(color, rank, team->ordinal(), team->endpoint().node, team->endpoint().ctxt);
  if (!dstbuf) xlpgas_fatalerror (-1, "comm_split: allocation error");

  /* -------------------------------------------------------------- */
  /*      allgather (localbuffer, tempbuffer, 3 * sizeof(int))      */
  /* -------------------------------------------------------------- */
  xlpgas_tspcoll_allgather (ctxt, teamID, &sndbuf, dstbuf, sizeof(Split));

  /* -------------------------------------------------------------- */
  /* collect list of global processor IDs proc enumeration buffer   */
  /* -------------------------------------------------------------- */
  int commsize = 0;
  for (int i=0; i<team->size(); i++)
    if (dstbuf[i].color == color)
      {
        proclist[dstbuf[i].rank].node  = dstbuf[i].node;
	proclist[dstbuf[i].rank].ctxt  = dstbuf[i].ctxt;
        if (dstbuf[i].rank >= commsize) commsize = dstbuf[i].rank + 1;
      }

  __global.heap_mm->free (dstbuf);
  /* -------------------------------------------------------------- */
  /*             test new proc list for consistency                 */
  /* -------------------------------------------------------------- */
  //for (int i=0; i<commsize; i++)
  //  if (proclist[i] == -1)
  //    xlpgas_fatalerror (-1, "SPLIT: Invalid new communicator");

  return commsize;
}


extern "C" void xlpgas_tspcoll_split   (int            ctxt,
					int            teamID,
					int            newID,
					int            color,
					int            rank)
{
  xlpgas::Team* team =  xlpgas::Team::get (ctxt, teamID);
  /* ---------------------------------------------------- */
  /* create a list of all processes in split communicator */
  /* ---------------------------------------------------- */
  int oldsize = team->size();
  xlpgas_endpoint_t * plist = (xlpgas_endpoint_t *) __global.heap_mm->malloc(sizeof(xlpgas_endpoint_t) * oldsize);
  if (!plist) xlpgas_fatalerror (-1, "team_split: allocation error");

  int nsize = __internal_tspcoll_split(ctxt, teamID, color, rank, plist);

  int mynewrank = -1;
  for (int i=0; i<nsize; i++) if (plist[i].node == team->endpoint().node &&
				  plist[i].ctxt == team->endpoint().ctxt) mynewrank=i;
  if (mynewrank<0) xlpgas_fatalerror(-1, "team_setup: Invalid process list");

  /* ---------------------------------------------------- */
  /* create and initialize an enumerated communicator     */
  /* The new team built here takes ownership of the plist ; responsible for freeing it*/
  /* ---------------------------------------------------- */
  team->split(ctxt, newID, mynewrank, nsize, plist);

  /* ---------------------------------------------------- */
  /* barrier to make sure everything is properly initalized */
  /* ---------------------------------------------------- */
  xlpgas_tspcoll_barrier(ctxt, teamID);//barrier in the original team
}

  /* ---------------------------------------------------- */
  /* Non blocking collectives : utilities                 */
  /* ---------------------------------------------------- */
extern "C" int xlpgas_tspcoll_team_size(int ctxt, int team){
  return xlpgas::Team::get(ctxt, team)->size();
}

extern "C" int xlpgas_tspcoll_team_id(int ctxt, int team){
  return xlpgas::Team::get(ctxt, team)->ordinal();
}

extern "C" int xlpgas_tspcoll_allocate_team_id(int ctxt, int team){
  return xlpgas::Team::get (ctxt, team)->allocate_team_id();
}