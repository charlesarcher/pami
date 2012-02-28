/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/RankBasedConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_RankBasedConnMgr_h__
#define __algorithms_connmgr_RankBasedConnMgr_h__

#include "algorithms/connmgr/ConnectionManager.h"
#include "Global.h"

namespace CCMI
{
  namespace ConnectionManager
  {
    class RankBasedConnMgr : public ConnectionManager<RankBasedConnMgr>
    {
      public:
        RankBasedConnMgr () :
            ConnectionManager<RankBasedConnMgr>(),
            _numConnections(__global.mapping.size())
        {
        }

        inline void setNumConnections_impl (size_t sz)
        {
          _numConnections = sz;
        }

        inline int getNumConnections_impl ()
        {
          return _numConnections;
        }

        inline unsigned getConnectionId_impl (unsigned comm,
                                              unsigned root,
                                              unsigned color,
                                              unsigned phase,
                                              unsigned dst = (unsigned) - 1)
        {
          (void)comm;(void)root;(void)color;(void)phase;(void)dst;
          return __global.mapping.task();
        }

        inline unsigned getRecvConnectionId_impl (unsigned comm,
                                                  unsigned root,
                                                  unsigned src,
                                                  unsigned phase,
                                                  unsigned color)
        {
          (void)comm;(void)root;(void)color;(void)phase;(void)src;
          return src;
        }


      private:
        size_t    _numConnections;
    };
  };
};

#endif
