/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/NBCollFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_NBCollFactory_h__
#define __algorithms_protocols_tspcoll_NBCollFactory_h__

#include "algorithms/protocols/tspcoll/NBColl.h"
#include "algorithms/protocols/tspcoll/Barrier.h"
#include "algorithms/protocols/tspcoll/Allgather.h"
#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/BinomBcast.h"
#include "algorithms/protocols/tspcoll/ScBcast.h"

#include "algorithms/protocols/tspcoll/Scatter.h"

namespace TSPColl
{
  template <class T_NI>
  class NBCollFactory
  {
    public:
      NBColl<T_NI> * create (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int id);
      void              initialize();
  };
};


/* ************************************************************************ */
/*                    initialize the factory                                */
/* ************************************************************************ */
template <class T_NI>
void TSPColl::NBCollFactory<T_NI>::initialize ()
{
  // TSPColl::CollExchange::amsend_reg ();
  // TSPColl::Scatter::amsend_reg ();
  // TSPColl::Gather::amsend_reg ();
}

/* ************************************************************************ */
/*                     Collective factory                                   */
/* ************************************************************************ */
template <class T_NI>
TSPColl::NBColl<T_NI> *
TSPColl::NBCollFactory<T_NI>::create (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID)
{
  pami_result_t prc;
  switch (tag)
    {
      case BarrierTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Barrier<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Barrier<T_NI> failed");
        memset (b, 0, sizeof(Barrier<T_NI>));
        new (b) Barrier<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case AllgatherTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Allgather<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Allgather<T_NI> failed");
        memset (b, 0, sizeof(Allgather<T_NI>));
        new (b) Allgather<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case AllgathervTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Allgatherv<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Allgatherv<T_NI> failed");
        memset (b, 0, sizeof(Allgatherv<T_NI>));
        new (b) Allgatherv<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case BcastTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(BinomBcast<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> BinomBcast<T_NI> failed");
        memset (b, 0, sizeof(BinomBcast<T_NI>));
        new (b) BinomBcast<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case BcastTag2:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(ScBcast<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> ScBcast<T_NI> failed");
        memset (b, 0, sizeof(ScBcast<T_NI>));
        new (b) ScBcast<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case ShortAllreduceTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Allreduce::Short<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Allreduce::Short<T_NI> failed");
        memset (b, 0, sizeof(Allreduce::Short<T_NI>));
        new (b) Allreduce::Short<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case LongAllreduceTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Allreduce::Long<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Allreduce::Long<T_NI> failed");
        memset (b, 0, sizeof(Allreduce::Long<T_NI>));
        new (b) Allreduce::Long<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case ScatterTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Scatter<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Scatter<T_NI> failed");
        memset (b, 0, sizeof(Scatter<T_NI>));
        new (b) Scatter<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case ScattervTag:
      {
        NBColl<T_NI> * b;
	prc = __global.heap_mm->memalign((void **)&b, 0, sizeof(Scatterv<T_NI>));
        PAMI_assertf(prc == PAMI_SUCCESS, "alloc of NBColl<T_NI> Scatterv<T_NI> failed");
        memset (b, 0, sizeof(Scatterv<T_NI>));
        new (b) Scatterv<T_NI> (comm, tag, instID, 0);
        return b;
      }
      case GatherTag:
      case GathervTag:
      default:
      {
        PAMI_abort();
      }
    }

  return (NBColl<T_NI> *) NULL;
}


#endif