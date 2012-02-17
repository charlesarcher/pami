/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/FCAWrapper.h
 * \brief ???
 */

#ifndef __algorithms_geometry_FCAWrapper_h__
#define __algorithms_geometry_FCAWrapper_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/geometry/Metadata.h"
#include "components/devices/fca/fcafunc.h"

namespace PAMI{namespace CollRegistration{
// The following classes are used to implement a CCMI wrapper
// around the FCA RT classes
// --------------  FCA Wrapper base class -------------
template <class T_Composite>
class FCAFactory;

template <class T_Geometry>
class FCAComposite:public CCMI::Executor::Composite
{
  typedef FCAFactory<FCAComposite> Factory;
public:
  typedef T_Geometry GeometryType;
  FCAComposite(T_Geometry *g, Factory *f):
    _g(g),
    _f(f)
    {
    }
  virtual void start()                  = 0;
  inline  void setComm(fca_comm_t *c)
    {
      _c = c;
    }
protected:
  T_Geometry  *_g;
  Factory     *_f;
  fca_comm_t  *_c;
};

// --------------  FCA Factory base class -------------
template <class T_Composite>
class FCAFactory : public CCMI::Adaptor::CollectiveProtocolFactory
{
public:
  FCAFactory(pami_context_t           ctxt,
             size_t                   ctxt_id,
             pami_mapidtogeometry_fn  cb_geometry):
    CollectiveProtocolFactory(ctxt,ctxt_id,cb_geometry)
  {
  }
  virtual CCMI::Executor::Composite * generate(pami_geometry_t  geometry,
                                               void            *cmd)
  {
    typedef typename T_Composite::GeometryType T_Geometry;
    T_Composite *c    = (T_Composite*)_alloc.allocateObject();
    pami_xfer_t *xfer = (pami_xfer_t *)cmd;
    T_Geometry  *g    = (T_Geometry*)geometry;
    fca_comm_t  *comm = (fca_comm_t*)(g->getKey(this->_context_id,
                                                Geometry::CKEY_FCAGEOMETRYINFO));
    c->setDoneCallback(xfer->cb_done, xfer->cookie);
    c->setContext(_context);
    c->setComm(comm);
    c->setxfer(xfer);
    return c;
  }
  virtual void metadata(pami_metadata_t *mdata)
  {
    new(mdata) PAMI::Geometry::Metadata(_string);
  }
  virtual void returnComposite(T_Composite *composite)
    {
      _alloc.returnObject(composite);
    }
  const char                                    *_string;
  PAMI::MemoryAllocator<sizeof(T_Composite),16>  _alloc;
};

static inline fca_reduce_dtype_t p_dtype_to_fca_dtype(pami_type_t f)
{
  // TODO:  remove branching, use table
  // TODO:  implement this function
  if(f == PAMI_TYPE_SIGNED_INT)
    return  FCA_DTYPE_INT;
  else
    {
      assert(0);
    }
}
static inline fca_reduce_op_t p_func_to_fca_op(pami_data_function d)
{
  // TODO:  remove branching, use table
  // TODO:  implement this function
}

// TODO:  convert endpoint based roots to TASKS
// --------------  FCA Reduce wrapper classes -------------
template <class T_Geometry>
class FCAReduceExec:public FCAComposite<T_Geometry>
{
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
public:
  FCAReduceExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
  {
  }
  virtual void start()
  {
    int rc = FCA_Do_reduce(this->_c, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_reduce failed with rc=%d",rc);
    this->_f->returnComposite(this);
  }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_reduce_t *cmd = &(xfer->cmd.xfer_reduce);
      _spec.root     = cmd->root;
      _spec.sbuf     = cmd->sndbuf;
      _spec.rbuf     = cmd->rcvbuf;
      _spec.dtype    = p_dtype_to_fca_dtype(cmd->stype);
      _spec.length   = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.op       = p_func_to_fca_op(cmd->op);
    }
private:
  fca_reduce_spec_t  _spec;
};

// --------------  FCA Allreduce wrapper classes -------------
template <class T_Geometry>
class FCAAllreduceExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllreduceExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
  {
  }
  virtual void start()
  {
    int rc = FCA_Do_all_reduce(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_all_reduce failed with rc=%d",rc);
    this->_f->returnComposite(this);
  }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allreduce_t *cmd = &(xfer->cmd.xfer_allreduce);
      _spec.root     = -1; // ? this ok?
      _spec.sbuf     = cmd->sndbuf;
      _spec.rbuf     = cmd->rcvbuf;
      _spec.dtype    = p_dtype_to_fca_dtype(cmd->stype);
      _spec.length   = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.op       = p_func_to_fca_op(cmd->op);
    }
private:
  fca_reduce_spec_t _spec;
};

// --------------  FCA Broadcast wrapper classes -------------
template <class T_Geometry>
class FCABroadcastExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCABroadcastExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_bcast(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_bcast failed with rc=%d",rc);
    this->_f->returnComposite(this);
  }
  inline void setxfer(pami_xfer_t *xfer)
  {
    pami_broadcast_t *cmd = &(xfer->cmd.xfer_broadcast);
    _spec.root            = cmd->root;
    _spec.buf             = cmd->buf;
    _spec.size            = cmd->typecount * ((Type*)cmd->type)->GetExtent();
  }
private:
  fca_bcast_spec_t _spec;
};

// --------------  FCA Allgather wrapper classes -------------
template <class T_Geometry>
class FCAAllgatherExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllgatherExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_allgather(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_allgather failed with rc=%d",rc);
    this->_f->returnComposite(this);
  }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allgather_t *cmd = &(xfer->cmd.xfer_allgather);
      _spec.root            = -1;  // this OK?
      _spec.sbuf            = cmd->sndbuf;
      _spec.rbuf            = cmd->rcvbuf;
      _spec.size            = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
    }
private:
  fca_gather_spec_t _spec;
};

// --------------  FCA Allgatherv_int wrapper classes -------------
template <class T_Geometry>
class FCAAllgathervIntExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllgathervIntExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_allgatherv(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_allgatherv failed with rc=%d",rc);
  }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allgatherv_int_t *cmd = &(xfer->cmd.xfer_allgatherv_int);
      _spec.root             = -1;  // this OK?
      _spec.sbuf             = cmd->sndbuf;
      _spec.rbuf             = cmd->rcvbuf;
      _spec.sendsize         = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.recvsizes        = cmd->rtypecounts;
      _spec.displs           = cmd->rdispls;
    }
private:
  fca_gatherv_spec_t _spec;
};

// --------------  FCA Barrier wrapper classes -------------
template <class T_Geometry>
class FCABarrierExec:public FCAComposite<T_Geometry>
{
public:
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCABarrierExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_barrier(this->_comm);
    PAMI_assertf(rc == 0, "FCA_Do_barrier failed with rc=%d",rc);
    this->_f->returnComposite(this);
  }
  inline void setxfer(pami_xfer_t *xfer)
    {
    }
private:
};

}//namespace CollRegistration
}//namespace PAMI


#endif
