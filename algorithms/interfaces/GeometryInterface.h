/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/interfaces/GeometryInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_GeometryInterface_h__
#define __algorithms_interfaces_GeometryInterface_h__

#include <pami.h>
#include "util/queue/MatchQueue.h"
#include "Mapping.h"
#include "components/memory/MemoryAllocator.h"
#include "algorithms/geometry/UnexpBarrierQueueElement.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "GeometryPlatform.h"

#define CCMI_EXECUTOR_TYPE void*
#define COMPOSITE_TYPE void*

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Geometry
  {
    typedef enum
    {
      GKEY_COLLFACTORY     = 0,
      GKEY_UEBARRIERCOMPOSITE1,    // CCMI ue barrier (CKEY_BARRIERCOMPOSITE1)
      GKEY_MCAST_CLASSROUTEID,     // Multicast class route id
      GKEY_MCOMB_CLASSROUTEID,     // Multicombine class route id
      GKEY_MSYNC_CLASSROUTEID,     // Multisync class route id
      GKEY_MSYNC_CLASSROUTEID1,    // Multisync class route id1 
      GKEY_MSYNC_LOCAL_CLASSROUTEID, //Multisync id for local hw accel barriers
      PAMI_GKEY_PLATEXTENSIONS
      GKEY_GEOMETRYCSNI,            // native interface for coll shm device
      NUM_GKEYS, 
    } gkeys_t;                     // global keystore keys
    typedef enum
    {
      CKEY_COLLFACTORY     = 0,
      CKEY_BARRIERCOMPOSITE0,      // ?
      CKEY_BARRIERCOMPOSITE1,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE2,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE3,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE4,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE5,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE6,      // CCMI barrier cache
      CKEY_BARRIERCOMPOSITE7,      // CCMI barrier cache
      CKEY_OPTIMIZEDBARRIERCOMPOSITE, // The optimized algorithm for this geometry
      CKEY_BCASTCOMPOSITE0,
      CKEY_BCASTCOMPOSITE1,
      CKEY_BCASTCOMPOSITE2,
      CKEY_BCASTCOMPOSITE3,
    } ckeys_t;                     // context keystore keys    

    typedef enum
    {
      DEFAULT_TOPOLOGY_INDEX      =  0,  // master/global sub-topology
      MASTER_TOPOLOGY_INDEX       =  1,  // master/global sub-topology
      LOCAL_TOPOLOGY_INDEX        =  2,  // local sub-topology
      LOCAL_MASTER_TOPOLOGY_INDEX =  3,  // local master sub-topology
      COORDINATE_TOPOLOGY_INDEX   =  4,  // coordinate topology (if valid)
      LIST_TOPOLOGY_INDEX         =  5,  // (optional) rank list topology
      MAX_NUM_TOPOLOGIES          =  6   // Number of topologies stored
    } topologyIndex_t;             // indices into _topos[] for special topologies

    template <class T_Geometry>
      class Geometry
    {
    public:
      inline Geometry (Geometry     *parent,
                       Mapping      *mapping,
                       unsigned      comm,
                       int           numranks,
                       pami_task_t  *ranks)
        {
          TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
        }
      inline Geometry (Geometry  *parent,
                       Mapping   *mapping,
                       unsigned   comm,
                       int        numranges,
                       pami_geometry_range_t rangelist[])
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
      }
      inline Geometry (Geometry  *parent,
                       Mapping   *mapping,
                       unsigned   comm,
                       PAMI::Topology *topo)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
      }

      // These methods were originally from the CCMI Geometry class
      inline unsigned                   comm();
      inline pami_task_t               *ranks();
      inline pami_task_t               *ranks_sizet();
      inline pami_task_t                nranks();
      inline pami_task_t                myIdx();
      inline void                       generatePermutation();
      inline void                       freePermutation();
      inline unsigned                  *permutation();
      inline pami_topology_t           *getTopology(topologyIndex_t topo_num);
      inline pami_topology_t           *createTopology(topologyIndex_t topo_num);
      inline pami_task_t                localMasterParticipant();
      inline bool                       isLocalMasterParticipant();
      inline void                       generatePermutation_sizet();
      inline void                       freePermutation_sizet();
      inline pami_task_t               *permutation_sizet();
      inline pami_client_t		getClient();
/** \todo  need to replace by attributes */
#if 1
      // no need for these as they are embedded inside attributes
      inline bool                       isRectangle();
      inline bool                       isTorus();
      inline bool                       isTree();
      inline bool                       isGlobalContext();
      inline bool                       isGI();
#endif
      inline unsigned                   getNumColors();
      inline unsigned                   getAllreduceIteration();
      inline void                       freeAllocations ();
      inline void                       setGlobalContext(bool context);
      inline void                       setNumColors(unsigned numcolors);
      inline MatchQueue                &asyncCollectivePostQ();
      inline MatchQueue                &asyncCollectiveUnexpQ();

      // These are CCMI typed methods that introduce CCMI dependencies on
      // the geometry interface
      // Do these belong in some kind of cache object or elsewhere?
#if 0
      inline void                       setBarrierExecutor (EXECUTOR_TYPE bar);
      inline RECTANGLE_TYPE             rectangle();
      inline RECTANGLE_TYPE             rectangle_mesh();
      inline EXECUTOR_TYPE              getLocalBarrierExecutor ();
      inline void                       setLocalBarrierExecutor (EXECUTOR_TYPE bar);
      inline EXECUTOR_TYPE              getCollectiveExecutor (unsigned color=0);
      inline void                       setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                               unsigned color=0);
      inline void                      *getBarrierExecutor();
#endif

      inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage(unsigned i);
      inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage ();
      inline COMPOSITE_TYPE             getAllreduceComposite(unsigned i);
      inline COMPOSITE_TYPE             getAllreduceComposite();
      inline void                       setAllreduceComposite(COMPOSITE_TYPE c);
      inline void                       setAllreduceComposite(COMPOSITE_TYPE c,
                                                              unsigned i);
      inline void                       processUnexpBarrier(MatchQueue * ueb_queue,
                                                            MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> *ueb_allocator);

      // These methods were originally from the PGASRT Communicator class
      inline pami_task_t                 size       (void);
      inline pami_task_t                 rank       (void);
      inline pami_task_t                 virtrank   (void);
      inline pami_task_t                 absrankof  (int rank);
      inline pami_task_t                 virtrankof (int rank);
      inline pami_task_t                 ordinal    ();
      inline pami_task_t                 ordinal    (int rank);
      inline pami_endpoint_t             endpoint   (pami_task_t ordinal);
      inline void                       setKey(gkeys_t key, void*value);
      inline void                       setKey(size_t context_id, ckeys_t key, void*value);
      inline void                      *getKey(gkeys_t key);
      inline void                      *getKey(size_t context_id, ckeys_t key);
      inline pami_result_t              ue_barrier(pami_event_function,void*,size_t,pami_context_t);
      inline void                       resetUEBarrier();
      inline void                       setUEBarrier(CCMI::Adaptor::CollectiveProtocolFactory *f);


      // API support
      inline pami_result_t algorithms_num(pami_xfer_type_t  colltype,
                                         size_t             *lists_lengths,
                                         size_t           context_id);

      inline pami_result_t algorithms_info (pami_xfer_type_t   colltype,
                                           pami_algorithm_t  *algs0,
                                           pami_metadata_t   *mdata0,
                                           size_t               num0,
                                           pami_algorithm_t  *algs1,
                                           pami_metadata_t   *mdata1,
                                           size_t               num1,
                                           size_t            context_id);



      // List management
      inline pami_result_t addCollective(pami_xfer_type_t                            xfer_type,
                                        CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                        size_t                                     context_id);
      inline pami_result_t rmCollective(pami_xfer_type_t                            xfer_type,
                                        CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                        size_t                                     context_id);

      inline pami_result_t addCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id);
      inline pami_result_t rmCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id);




    }; // class Geometry

    template <class T_Geometry>
    inline pami_topology_t* Geometry<T_Geometry>::getTopology(topologyIndex_t topo_num)
    {
      return static_cast<T_Geometry*>(this)->getTopology_impl(topo_num);
    }

    template <class T_Geometry>
    inline pami_topology_t* Geometry<T_Geometry>::createTopology(topologyIndex_t topo_num)
    {
      return static_cast<T_Geometry*>(this)->createTopology_impl(topo_num);
    }

    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isLocalMasterParticipant()
    {
      return static_cast<T_Geometry*>(this)->isLocalMasterParticipant_impl();
    }

    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::localMasterParticipant()
    {
      return static_cast<T_Geometry*>(this)->localMasterParticipant_impl();
    }

    template <class T_Geometry>
    inline unsigned Geometry<T_Geometry>::comm()
    {
      return static_cast<T_Geometry*>(this)->comm_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::ranks()
    {
      return static_cast<T_Geometry*>(this)->ranks_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::ranks_sizet()
    {
      return static_cast<T_Geometry*>(this)->ranks_sizet_impl();
    }

    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::nranks()
    {
      return static_cast<T_Geometry*>(this)->nranks_impl();
    }

    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::myIdx()
    {
      return static_cast<T_Geometry*>(this)->myIdx_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::generatePermutation()
    {
      return static_cast<T_Geometry*>(this)->generatePermutation_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::freePermutation()
    {
      return static_cast<T_Geometry*>(this)->freePermutation_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::permutation()
    {
      return static_cast<T_Geometry*>(this)->permutation_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::generatePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->generatePermutation_sizet_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::freePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->freePermutation_sizet_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::permutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->permutation_sizet_impl();
    }

    template <class T_Geometry>
    inline pami_client_t Geometry<T_Geometry>::getClient()
    {
      return static_cast<T_Geometry*>(this)->getClient_impl();
    }

/** \todo replace by attributes */
#if 1
    // to be removed eventually
    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isRectangle()
    {
      return static_cast<T_Geometry*>(this)->isRectangle_impl();
    }

    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isTorus()
    {
      return static_cast<T_Geometry*>(this)->isTorus_impl();
    }

    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isTree()
    {
      return static_cast<T_Geometry*>(this)->isTree_impl();
    }

    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isGlobalContext()
    {
      return static_cast<T_Geometry*>(this)->isGlobalContext_impl();
    }

    template <class T_Geometry>
    inline bool Geometry<T_Geometry>::isGI()
    {
      return static_cast<T_Geometry*>(this)->isGI_impl();
    }
#endif
    template <class T_Geometry>
    inline unsigned Geometry<T_Geometry>::getNumColors()
    {
      return static_cast<T_Geometry*>(this)->getNumColors_impl();
    }

    template <class T_Geometry>
    inline unsigned Geometry<T_Geometry>::getAllreduceIteration()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceIteration_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::freeAllocations ()
    {
      return static_cast<T_Geometry*>(this)->freeAllocations_impl();
    }
    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setGlobalContext(bool context)
    {
      return static_cast<T_Geometry*>(this)->setGlobalContext_impl(context);
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setNumColors(unsigned numcolors)
    {
      return static_cast<T_Geometry*>(this)->setNumColors_impl(numcolors);
    }

    template <class T_Geometry>
    inline MatchQueue &Geometry<T_Geometry>::asyncCollectivePostQ()
    {
      return static_cast<T_Geometry*>(this)->asyncCollectivePostQ_impl();
    }

    template <class T_Geometry>
    inline MatchQueue &Geometry<T_Geometry>::asyncCollectiveUnexpQ()
    {
      return static_cast<T_Geometry*>(this)->asyncCollectiveUnexpQ_impl();
    }
#if 0
    template <class T_Geometry>
    inline RECTANGLE_TYPE Geometry<T_Geometry>::rectangle()
    {
      return static_cast<T_Geometry*>(this)->rectangle_impl();
    }

    template <class T_Geometry>
    inline RECTANGLE_TYPE Geometry<T_Geometry>::rectangle_mesh()
    {
      return static_cast<T_Geometry*>(this)->rectangle_mesh_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setBarrierExecutor (EXECUTOR_TYPE bar)
    {
      return static_cast<T_Geometry*>(this)->setBarrierExecutor_impl(bar);
    }

    template <class T_Geometry>
    inline EXECUTOR_TYPE Geometry<T_Geometry>::getLocalBarrierExecutor ()
    {
      return static_cast<T_Geometry*>(this)->getLocalBarrierExecutor_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setLocalBarrierExecutor (EXECUTOR_TYPE bar)
    {
      return static_cast<T_Geometry*>(this)->setLocalBarrierExecutor_impl(bar);
    }

    template <class T_Geometry>
    inline EXECUTOR_TYPE Geometry<T_Geometry>::getCollectiveExecutor (unsigned color)
    {
      return static_cast<T_Geometry*>(this)->getCollectiveExecutor_impl(color);
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                                          unsigned color)
    {
      return static_cast<T_Geometry*>(this)->setCollectiveExecutor_impl(exe, color);
    }

    template <class T_Geometry>
    inline void * Geometry<T_Geometry>::getBarrierExecutor()
    {
      return static_cast<T_Geometry*>(this)->getBarrierExecutor_impl();
    }

#endif
    template <class T_Geometry>
    inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage(unsigned i)
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl(i);
    }

    template <class T_Geometry>
    inline COMPOSITE_TYPE Geometry<T_Geometry>::getAllreduceComposite(unsigned i)
    {
      return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl(i);
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setAllreduceComposite(COMPOSITE_TYPE c)
    {
      return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c);
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::setAllreduceComposite(COMPOSITE_TYPE c,
                                                                         unsigned i)
    {
      return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c, i);
    }

    template <class T_Geometry>
    inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage ()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl();
    }

    template <class T_Geometry>
    inline COMPOSITE_TYPE Geometry<T_Geometry>::getAllreduceComposite()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::processUnexpBarrier(MatchQueue * ueb_queue,
                                                          MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> *ueb_allocator)
    {
      return static_cast<T_Geometry*>(this)->processUnexpBarrier_impl(ueb_queue,ueb_allocator);
    }

    // These methods were originally from the PGASRT Communicator class
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::size       (void)
    {
      return static_cast<T_Geometry*>(this)->size_impl();
    }
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::rank       (void)
    {
      return static_cast<T_Geometry*>(this)->rank_impl();
    }
    template <class T_Geometry>
    inline pami_task_t   Geometry<T_Geometry>::virtrank()
    {
      return static_cast<T_Geometry*>(this)->virtrank_impl();
    }
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::absrankof  (int rank)
    {
      return static_cast<T_Geometry*>(this)->absrankof_impl(rank);
    }
    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::virtrankof (int rank)
    {
      return static_cast<T_Geometry*>(this)->virtrankof_impl(rank);
    }
    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::ordinal (int rank)
    {
      return static_cast<T_Geometry*>(this)->ordinal_impl(rank);
    }
    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::ordinal ()
    {
      return static_cast<T_Geometry*>(this)->ordinal_impl();
    }
    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::endpoint (pami_task_t ordinal)
    {
      return static_cast<T_Geometry*>(this)->endpoint_impl(ordinal);
    }
    template <class T_Geometry>
    inline void                        Geometry<T_Geometry>::setKey (gkeys_t key, void *value)
    {
      static_cast<T_Geometry*>(this)->setKey_impl(key, value);
    }
    template <class T_Geometry>
    inline void                        Geometry<T_Geometry>::setKey (size_t context_id, ckeys_t key, void *value)
    {
      static_cast<T_Geometry*>(this)->setKey_impl(context_id, key, value);
    }
    template <class T_Geometry>
    inline void*                       Geometry<T_Geometry>::getKey (gkeys_t key)
    {
      return static_cast<T_Geometry*>(this)->getKey_impl(key);
    }
    template <class T_Geometry>
    inline void*                       Geometry<T_Geometry>::getKey (size_t context_id, ckeys_t key)
    {
      return static_cast<T_Geometry*>(this)->getKey_impl(context_id, key);
    }

    template <class T_Geometry>
    inline pami_result_t               Geometry<T_Geometry>::ue_barrier(pami_event_function      cb_done,
                                                                        void               *cookie,
                                                                        size_t              ctxt_id,
                                                                        pami_context_t      context)
    {
      return static_cast<T_Geometry*>(this)->ue_barrier_impl(cb_done, cookie, ctxt_id, context);
    }

    template <class T_Geometry>
    inline void                        Geometry<T_Geometry>::resetUEBarrier()
    {
      return static_cast<T_Geometry*>(this)->resetUEBarrier_impl();
    }

    template <class T_Geometry>
    inline void                        Geometry<T_Geometry>::setUEBarrier(CCMI::Adaptor::CollectiveProtocolFactory *f)
    {
      return static_cast<T_Geometry*>(this)->setUEBarrier_impl(f);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::algorithms_num(pami_xfer_type_t  colltype,
                                                             size_t             *lists_lengths,
                                                             size_t           context_id)
    {
      return static_cast<T_Geometry*>(this)->algorithms_num_impl(colltype,lists_lengths,context_id);
    }

    template <class T_Geometry>
    inline pami_result_t  Geometry<T_Geometry>::algorithms_info (pami_xfer_type_t   colltype,
                                                                pami_algorithm_t  *algs0,
                                                                pami_metadata_t   *mdata0,
                                                                size_t               num0,
                                                                pami_algorithm_t  *algs1,
                                                                pami_metadata_t   *mdata1,
                                                                size_t               num1,
                                                                size_t            context_id)
    {
      return static_cast<T_Geometry*>(this)->algorithms_info_impl(colltype,
                                                                  algs0,
                                                                  mdata0,
                                                                  num0,
                                                                  algs1,
                                                                  mdata1,
                                                                  num1,
                                                                  context_id);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::addCollective(pami_xfer_type_t                            xfer_type,
                                                            CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                            size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->addCollective_impl(xfer_type,factory,context_id);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::rmCollective(pami_xfer_type_t                            xfer_type,
                                                            CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                            size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->rmCollective_impl(xfer_type,factory,context_id);
    }


    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::addCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                                                 CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                                 size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->addCollectiveCheck_impl(xfer_type,factory,context_id);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::rmCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                                                 CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                                 size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->rmCollectiveCheck_impl(xfer_type,factory,context_id);
    }


  }; // namespace Geometry
}; // namespace PAMI

#undef TRACE_ERR

#endif
