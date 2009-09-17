/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/mpi/mpicollfactory.h
 * \brief ???
 */

#ifndef   __xmi_mpicollfactory__h__
#define   __xmi_mpicollfactory__h__


#define XMI_COLLFACTORY_CLASS XMI::CollFactory::MPI<Device::MPIDevice<SysDep::MPISysDep>, SysDep::MPISysDep >

#include "sys/xmi.h"
#include "components/geometry/CollFactory.h"
#include "components/geometry/mpi/mpicollinfo.h"
#include "util/common.h"
#include "algorithms/ccmi.h"

namespace XMI
{
  namespace CollFactory
  {
    template <class T_Device, class T_Sysdep>
    class MPI : public CollFactory<XMI::CollFactory::MPI<T_Device, T_Sysdep> >
    {
    public:
      inline MPI(T_Sysdep *sd):
        CollFactory<XMI::CollFactory::MPI<T_Device, T_Sysdep> >(),
        _sd(sd)
        {
        }

      inline RegQueue * getRegQ(xmi_xfer_type_t       collective)
        {
          RegQueue *rq = NULL;
          switch (collective)
              {
                  case XMI_XFER_BROADCAST:
                    rq = &_broadcasts;
                    break;
                  case XMI_XFER_ALLREDUCE:
                    rq = &_allreduces;
                    break;
                  case XMI_XFER_REDUCE:
                    return NULL;
                    break;
                  case XMI_XFER_ALLGATHER:
                    rq = &_allgathers;
                    break;
                  case XMI_XFER_ALLGATHERV:
                    rq = &_allgathervs;
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return NULL;
                    break;
                  case XMI_XFER_SCATTER:
                    rq = &_scatters;
                    break;
                  case XMI_XFER_SCATTERV:
                    rq = &_scattervs;
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    break;
                  case XMI_XFER_BARRIER:
                    rq = &_barriers;
                    break;
                  case XMI_XFER_ALLTOALL:
                    return NULL;
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return NULL;
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return NULL;
                    break;
                  case XMI_XFER_SCAN:
                    return NULL;
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return NULL;
                    break;
                  case XMI_XFER_AMSCATTER:
                    return NULL;
                    break;
                  case XMI_XFER_AMGATHER:
                    return NULL;
                    break;
                  case XMI_XFER_AMREDUCE:
                    return NULL;
                    break;
                  default:
                    return NULL;
              }
          return rq;
        }

      inline xmi_result_t  algorithm_impl(xmi_xfer_type_t       collective,
                                          xmi_algorithm_t      *alglist,
                                          int                  *num)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          int i = rq->size();
          *num = MIN(*num, i);
          for(i=0; i<*num; i++)
            alglist[i] = (size_t)i;
          return XMI_SUCCESS;
        }

      inline size_t        num_algorithm_impl   (xmi_xfer_type_t           collective)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          return rq->size();
        }




      inline xmi_result_t  setGeometry(XMI_GEOMETRY_CLASS *g, XMI_NBCollManager *mgr, T_Device *dev)
        {
          _geometry = g;
          _dev      = dev;
          _barrier    = mgr->allocate (g, TSPColl::BarrierTag);
          _allgather  = mgr->allocate (g, TSPColl::AllgatherTag);
          _allgatherv = mgr->allocate (g, TSPColl::AllgathervTag);
          _bcast      = mgr->allocate (g, TSPColl::BcastTag);
          _bcast2     = mgr->allocate (g, TSPColl::BcastTag2);
          _sar        = mgr->allocate (g, TSPColl::ShortAllreduceTag);
          _lar        = mgr->allocate (g, TSPColl::LongAllreduceTag);
          _sct        = mgr->allocate (g, TSPColl::ScatterTag);
          _sctv       = mgr->allocate (g, TSPColl::ScattervTag);
#if 0
          CCMI::Executor::Executor *exe = NULL;
          exe = _barrier_factory.generate(&_barrier_executors[0], &_ccmi_geometry);
          _ccmi_geometry.setBarrierExecutor(exe);
          exe = _barrier_factory.generate(&_barrier_executors[1], &_ccmi_geometry);
          _ccmi_geometry.setLocalBarrierExecutor(exe);
#endif
	  return XMI_SUCCESS;
        }

      inline xmi_result_t  add_collective(xmi_xfer_type_t          collective,
					  XMI::CollInfo::CollInfo<T_Device>* ci)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          rq->push_back(ci);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  collective_impl      (xmi_xfer_t           *collective)
        {
          switch (collective->xfer_type)
              {
                  case XMI_XFER_BROADCAST:
                    return ibroadcast_impl(&collective->xfer_broadcast);
                    break;
                  case XMI_XFER_ALLREDUCE:
                    return iallreduce_impl(&collective->xfer_allreduce);
                    break;
                  case XMI_XFER_REDUCE:
                    return ireduce_impl(&collective->xfer_reduce);
                    break;
                  case XMI_XFER_ALLGATHER:
                    return iallgather_impl(&collective->xfer_allgather);
                    break;
                  case XMI_XFER_ALLGATHERV:
                    return iallgatherv_impl(&collective->xfer_allgatherv);
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return iallgatherv_int_impl(&collective->xfer_allgatherv_int);
                    break;
                  case XMI_XFER_SCATTER:
                    return iscatter_impl(&collective->xfer_scatter);
                    break;
                  case XMI_XFER_SCATTERV:
                    return iscatterv_impl(&collective->xfer_scatterv);
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    return iscatterv_int_impl(&collective->xfer_scatterv_int);
                    break;
                  case XMI_XFER_BARRIER:
                    return ibarrier_impl(&collective->xfer_barrier);
                    break;
                  case XMI_XFER_ALLTOALL:
                    return ialltoall_impl(&collective->xfer_alltoall);
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return ialltoallv_impl(&collective->xfer_alltoallv);
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return ialltoallv_int_impl(&collective->xfer_alltoallv_int);
                    break;
                  case XMI_XFER_SCAN:
                    return iscan_impl(&collective->xfer_scan);
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return ambroadcast_impl(&collective->xfer_ambroadcast);
                    break;
                  case XMI_XFER_AMSCATTER:
                    return amscatter_impl(&collective->xfer_amscatter);
                    break;
                  case XMI_XFER_AMGATHER:
                    return amgather_impl(&collective->xfer_amgather);
                    break;
                  case XMI_XFER_AMREDUCE:
                    return amreduce_impl(&collective->xfer_amreduce);
                    break;
                  default:
                    return XMI_UNIMPL;
              }
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibroadcast_impl      (xmi_broadcast_t      *broadcast)
        {
          XMI::CollInfo::PGBroadcastInfo<T_Device> *info =
            (XMI::CollInfo::PGBroadcastInfo<T_Device> *)_broadcasts[broadcast->algorithm];
          if (!_bcast->isdone()) _dev->advance();

          ((TSPColl::BinomBcast<MPIMcastModel> *)_bcast)->reset (_geometry->virtrankof(broadcast->root),
                                                                 broadcast->buf,
                                                                 broadcast->buf,
                                                                 broadcast->typecount);
          _bcast->setComplete(broadcast->cb_done, broadcast->cookie);
          _bcast->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallreduce_impl      (xmi_allreduce_t      *allreduce)
        {
          XMI::CollInfo::PGAllreduceInfo<T_Device> *info =
            (XMI::CollInfo::PGAllreduceInfo<T_Device> *)_allreduces[allreduce->algorithm];
          unsigned datawidth;
          coremath cb_allreduce;
          CCMI::Adaptor::Allreduce::getReduceFunction(allreduce->dt,
                                                      allreduce->op,
                                                      allreduce->stypecount,
                                                      datawidth,
                                                      cb_allreduce);
          if (datawidth * allreduce->stypecount < (unsigned)TSPColl::Allreduce::Short<MPIMcastModel>::MAXBUF)
              {
                if (!_sar->isdone()) _dev->advance();
                ((TSPColl::Allreduce::Short<MPIMcastModel> *)_sar)->reset (allreduce->sndbuf,
                                                                           allreduce->rcvbuf,
                                                                           allreduce->op,
                                                                           allreduce->dt,
                                                                           allreduce->stypecount);
                _sar->setComplete(allreduce->cb_done, allreduce->cookie);
                _sar->kick(&info->_model);
                return XMI_SUCCESS;
              }
          else
              {
                if (!_lar->isdone()) _dev->advance();
                ((TSPColl::Allreduce::Long<MPIMcastModel> *)_lar)->reset (allreduce->sndbuf,
                                                                          allreduce->rcvbuf,
                                                                          allreduce->op,
                                                                          allreduce->dt,
                                                                          allreduce->stypecount);
                _lar->setComplete(allreduce->cb_done, allreduce->cookie);
                _lar->kick(&info->_model);
                return XMI_SUCCESS;
              }
          return XMI_SUCCESS;
        }

      inline xmi_result_t  ireduce_impl         (xmi_reduce_t         *reduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgather_impl      (xmi_allgather_t      *allgather)
        {
          XMI::CollInfo::PGAllgatherInfo<T_Device> *info =
            (XMI::CollInfo::PGAllgatherInfo<T_Device> *)_allgathers[allgather->algorithm];
          if (!_allgather->isdone()) _dev->advance();
          ((TSPColl::Allgather<MPIMcastModel> *)_allgather)->reset (allgather->sndbuf,
                                                                    allgather->rcvbuf,
                                                                    allgather->stypecount);
          _allgather->setComplete(allgather->cb_done, allgather->cookie);
          _allgather->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallgatherv_impl     (xmi_allgatherv_t     *allgatherv)
        {

          XMI::CollInfo::PGAllgathervInfo<T_Device> *info =
            (XMI::CollInfo::PGAllgathervInfo<T_Device> *)_allgathervs[allgatherv->algorithm];
          if (!_allgatherv->isdone()) _dev->advance();
          ((TSPColl::Allgatherv<MPIMcastModel> *)_allgatherv)->reset (allgatherv->sndbuf,
                                                                      allgatherv->rcvbuf,
                                                                      allgatherv->rtypecounts);
          _allgatherv->setComplete(allgatherv->cb_done, allgatherv->cookie);
          _allgatherv->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallgatherv_int_impl (xmi_allgatherv_int_t *allgatherv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatter_impl        (xmi_scatter_t        *scatter)
        {
          XMI::CollInfo::PGScatterInfo<T_Device> *info =
            (XMI::CollInfo::PGScatterInfo<T_Device> *)_scatters[scatter->algorithm];
          if (!_sct->isdone()) _dev->advance();
          ((TSPColl::Scatter<MPIMcastModel> *)_sct)->reset (_geometry->virtrankof(scatter->root),
                                                            scatter->sndbuf,
                                                            scatter->rcvbuf,
                                                            scatter->stypecount);
          _sct->setComplete(scatter->cb_done, scatter->cookie);

          while(!_barrier->isdone()) _dev->advance();
          ((TSPColl::Barrier<MPIMcastModel> *)_barrier)->reset();
          _barrier->setComplete(NULL, NULL);
          _barrier->kick(&info->_bmodel);
          while(!_barrier->isdone()) _dev->advance();

          _sct->kick(&info->_smodel);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iscatterv_impl       (xmi_scatterv_t       *scatterv)
        {
          XMI::CollInfo::PGScattervInfo<T_Device> *info =
            (XMI::CollInfo::PGScattervInfo<T_Device> *)_scattervs[scatterv->algorithm];
          if (!_sctv->isdone()) _dev->advance();
          ((TSPColl::Scatterv<MPIMcastModel> *)_sctv)->reset (_geometry->virtrankof(scatterv->root),
                                                              scatterv->sndbuf,
                                                              scatterv->rcvbuf,
                                                              scatterv->stypecounts);
          _sctv->setComplete(scatterv->cb_done, scatterv->cookie);

          while(!_barrier->isdone()) _dev->advance();
          ((TSPColl::Barrier<MPIMcastModel> *)_barrier)->reset();
          _barrier->setComplete(NULL, NULL);
          _barrier->kick(&info->_bmodel);
          while(!_barrier->isdone()) _dev->advance();

          _sctv->kick(&info->_smodel);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iscatterv_int_impl   (xmi_scatterv_int_t   *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibarrier_impl        (xmi_barrier_t        *barrier)
        {
          XMI::CollInfo::PGBarrierInfo<T_Device> *info = (XMI::CollInfo::PGBarrierInfo<T_Device> *)_barriers[barrier->algorithm];
          while(!_barrier->isdone()) _dev->advance();
          ((TSPColl::Barrier<MPIMcastModel> *)_barrier)->reset();
          _barrier->setComplete(barrier->cb_done, barrier->cookie);
          _barrier->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  ialltoall_impl       (xmi_alltoall_t       *alltoall)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_impl      (xmi_alltoallv_t      *alltoallv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_int_impl  (xmi_alltoallv_int_t  *alltoallv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscan_impl           (xmi_scan_t           *scan)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ambroadcast_impl     (xmi_ambroadcast_t    *ambroadcast)
        {
          XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep> *info =
            (XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep> *)_ambroadcasts[ambroadcast->algorithm];

          if(ambroadcast->stypecount == 0)
            ambroadcast->cb_done(NULL, ambroadcast->cookie, XMI_SUCCESS);
          else
              {
#if 0
//                CCMI_assert (((CCMI::Adaptor::Geometry *) geometry)->getBarrierExecutor() != NULL);
                xmi_callback_t cb_done_ccmi;
                cb_done_ccmi.function   = ambroadcast->cb_done;
                cb_done_ccmi.clientdata = ambroadcast->cookie;
                XMI_CollectiveRequest_t *req = (XMI_CollectiveRequest_t *)malloc(sizeof(XMI_Request_t));
                factory->generate(req,
                                  sizeof(XMI_CollectiveRequest_t),
                                  cb_done_ccmi,
                                  XMI_MATCH_CONSISTENCY,
                                  _geometry,
                                  _sd.mapping->task(), //root
                                  ambroadcast->sndbuf,
                                  ambroadcast->stypecount);
#endif
              }
          return XMI_SUCCESS;
        }

      inline xmi_result_t  amscatter_impl       (xmi_amscatter_t      *amscatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amgather_impl        (xmi_amgather_t       *amgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amreduce_impl        (xmi_amreduce_t       *amreduce)
        {
          return XMI_UNIMPL;
        }
      T_Device                        *_dev;
      XMI_GEOMETRY_CLASS              *_geometry;
      T_Sysdep                        *_sd;
      RegQueue                         _broadcasts;
      RegQueue                         _ambroadcasts;
      RegQueue                         _allgathers;
      RegQueue                         _allgathervs;
      RegQueue                         _scatters;
      RegQueue                         _scattervs;
      RegQueue                         _allreduces;
      RegQueue                         _barriers;
      TSPColl::NBColl<MPIMcastModel>  *_barrier;
      TSPColl::NBColl<MPIMcastModel>  *_allgather;
      TSPColl::NBColl<MPIMcastModel>  *_allgatherv;
      TSPColl::NBColl<MPIMcastModel>  *_bcast, *_bcast2;
      TSPColl::NBColl<MPIMcastModel>  *_sar,   *_lar;
      TSPColl::NBColl<MPIMcastModel>  *_sct,   *_sctv;
    }; // class CollFactory
  };  // namespace CollFactory
}; // namespace XMI


#endif
