/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/bgp/BgpMapping.h
/// \brief ???
///
#ifndef __components_mapping_bgp_bgpmapping_h__
#define __components_mapping_bgp_bgpmapping_h__

#include "sys/xmi.h"

#include "components/mapping/BaseMapping.h"
#include "components/mapping/TorusMapping.h"
#include "components/mapping/NodeMapping.h"

#include "components/sysdep/bgp/BgpGlobal.h"
#include "components/memory/shmem/SharedMemoryManager.h"

#define XMI_MAPPING_CLASS XMI::Mapping::BgpMapping

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#define XMI_BGP_NETWORK_DIMS	3
#define XMI_BGP_LOCAL_DIMS	1

/// \brief how to get the global part of an estimated task
#define ESTIMATED_TASK_GLOBAL(x,y,z,t,xSize,ySize,zSize,tSize)	\
	  ESTIMATED_TASK(x,y,z,0,xSize,ySize,zSize,1)

/// \brief how to get the local part of an estimated task
#define ESTIMATED_TASK_LOCAL(x,y,z,t,xSize,ySize,zSize,tSize)	\
	  (t)

/// \brief how to get the estimated task back from global+local
///
/// This is closely tied to ESTIMATED_TASK_GLOBAL and ESTIMATED_TASK_LOCAL!
#define ESTIMATED_TASK_NODE(global,local,xSize,ySize,zSize,tSize)	\
	  ((local * xSize * ySize * zSize) + global)

namespace XMI
{
  namespace Mapping
  {
    class BgpMapping : public Interface::Base<BgpMapping, XMI::Memory::SharedMemoryManager>,
        public Interface::Torus<BgpMapping, XMI_BGP_NETWORK_DIMS>,
        public Interface::Node<BgpMapping, XMI_BGP_LOCAL_DIMS>
    {
      protected:

        typedef struct nodeaddr
        {
          union
          {
            kernel_coords_t coords;
            size_t          global;
          };
          size_t            local;
        } nodeaddr_t;


      public:
        inline BgpMapping () :
            Interface::Base<BgpMapping, XMI::Memory::SharedMemoryManager>(),
            Interface::Torus<BgpMapping, XMI_BGP_NETWORK_DIMS>(),
            Interface::Node<BgpMapping, XMI_BGP_LOCAL_DIMS> (),
            _task (0),
            _size (0),
            _nodes (0),
            _peers (0),
            _x (__global.personality.xCoord()),
            _y (__global.personality.yCoord()),
            _z (__global.personality.zCoord()),
            _t (__global.personality.tCoord()),
            _mapcache (NULL),
            _rankcache (NULL)
        {
        };

        inline ~BgpMapping () {};

      protected:
        size_t _task;
        size_t _size;
        size_t _nodes;
        size_t _peers;
        size_t _x;
        size_t _y;
        size_t _z;
        size_t _t;

        XMI::Mapping::Interface::nodeaddr_t _nodeaddr;

#warning These elements need to be moved or replaced
        size_t _numActiveRanksLocal;
        size_t _numActiveRanksGlobal;
        size_t _numActiveNodesGlobal;
        size_t _fullSize;

        size_t * _mapcache;
        size_t * _rankcache;
        size_t   _peercache[NUM_CORES];
        unsigned _localranks[NUM_CORES];

      public:

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        /// \see XMI::Mapping::Interface::Base::init()
        ///
        inline xmi_result_t init_impl (xmi_coord_t &ll, xmi_coord_t &ur,
                                       size_t &min_rank, size_t &max_rank,
                                       XMI::Memory::SharedMemoryManager &mm);

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Mapping::Interface::Base::task()
        ///
        inline size_t task_impl()
        {
          return _task;
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Mapping::Interface::Base::size()
        ///
        inline size_t size_impl()
        {
          return _size;
        }
#if 0
        ///
        /// \brief Number of physical active nodes in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveNodesGlobal()
        ///
        inline size_t numActiveNodesGlobal_impl ()
        {
          return _nodes;
        }

        ///
        /// \brief Number of physical active tasks in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveRanksGlobal()
        ///
        inline size_t numActiveTasksGlobal_impl ()
        {
          return _size;
        }
#endif
        ///
        /// \brief Determines if two global tasks are located on the same physical node.
        /// \see XMI::Mapping::Interface::Base::isPeer()
        ///
        inline bool isPeer_impl (size_t task1, size_t task2)
        {
          unsigned xyzt1 = _mapcache[task1];
          unsigned xyzt2 = _mapcache[task2];
          return ((xyzt1 >> 8) == (xyzt2 >> 8));
        }
        inline xmi_result_t network2task_impl (const xmi_coord_t  * addr,
                                               size_t                     * task,
                                               xmi_network               * type)
        {
          size_t xSize = __global.personality.xSize();
          size_t ySize = __global.personality.ySize();
          size_t zSize = __global.personality.zSize();
          size_t tSize = __global.personality.tSize();

          if (addr->network != XMI_DEFAULT_NETWORK &&
              addr->network != XMI_N_TORUS_NETWORK)
            {
              return XMI_INVAL;
            }

          *type = XMI_N_TORUS_NETWORK;
          size_t x = addr->u.n_torus.coords[0];
          size_t y = addr->u.n_torus.coords[1];
          size_t z = addr->u.n_torus.coords[2];
          size_t t = addr->u.n_torus.coords[3];

          if ((x >= xSize) || (y >= ySize) ||
              (z >= zSize) || (t >= tSize))
            {
              return XMI_INVAL;
            }

          size_t estimated_task = ESTIMATED_TASK(x, y, z, t, xSize, ySize, zSize, tSize);

          if (unlikely(_rankcache [estimated_task] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          *task = _rankcache [estimated_task];
          return XMI_SUCCESS;
        }

        inline xmi_result_t task2network_impl (size_t                task,
                                               xmi_coord_t * addr,
                                               xmi_network          type)
        {
          unsigned xyzt = _mapcache[task];
          addr->network = XMI_N_TORUS_NETWORK;
          addr->u.n_torus.coords[0] = (xyzt >> 24) & 0x0ff;
          addr->u.n_torus.coords[1] = (xyzt >> 16) & 0x0ff;
          addr->u.n_torus.coords[2] = (xyzt >> 8) & 0x0ff;
          addr->u.n_torus.coords[3] = (xyzt >> 0) & 0x0ff;
          return XMI_SUCCESS;
        }
        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Return the BGP torus x coordinate (dimension 0) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        inline size_t x ()
        {
          return __global.personality.xCoord();
        }
        inline size_t y ()
        {
          return __global.personality.yCoord();
        }
        inline size_t z ()
        {
          return __global.personality.zCoord();
        }
        inline size_t t ()
        {
          return __global.personality.tCoord();
        }

        inline size_t xSize ()
        {
          return __global.personality.xSize();
        }

        inline size_t ySize ()
        {
          return __global.personality.ySize();
        }

        inline size_t zSize ()
        {
          return __global.personality.zSize();
        }

        inline size_t tSize ()
        {
          return __global.personality.tSize();
        }


#if 0

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusCoord_impl<0> () const
        {
          return __global.personality.xCoord();
        }

        ///
        /// \brief Return the BGP torus y coordinate (dimension 1) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<1> () const
        {
          return __global.personality.yCoord();
        }

        ///
        /// \brief Return the BGP torus z coordinate (dimension 2) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<2> () const
        {
          return __global.personality.zCoord();
        }

        ///
        /// \brief Return the BGP torus t coordinate (dimension 3) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<3> () const
        {
          return __global.personality.tCoord();
        }

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<0> () const
        {
          return __global.personality.xSize();
        }

        ///
        /// \brief Return the size of the BGP torus y dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<1> () const
        {
          return __global.personality.ySize();
        }

        ///
        /// \brief Return the size of the BGP torus z dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<2> () const
        {
          return __global.personality.zSize();
        }

        ///
        /// \brief Return the size of the BGP torus t dimension
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<3> () const
        {
          return __global.personality.tSize();
        }
#endif
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Mapping::Interface::Torus::torusDims()
        ///
        inline const size_t torusDims_impl() const
        {
          return XMI_BGP_NETWORK_DIMS;
        }
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Mapping::Interface::Torus::torusDims()
        ///
        inline const size_t globalDims_impl() const
        {
          return XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS;
        }
        ///
        /// \brief Get the BGP torus address for this task
        /// \see XMI::Mapping::Interface::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS])
        {
          addr[0] = _x;
          addr[1] = _y;
          addr[2] = _z;
          addr[3] = _t;
        }

        ///
        /// \brief Get the BGP torus address for a task
        /// \see XMI::Mapping::Interface::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS])
        {
          unsigned xyzt = _mapcache[task];
          addr[0] = (xyzt & 0xFF000000) >> 24;
          addr[1] = (xyzt & 0xFF0000) >> 16;
          addr[2] = (xyzt & 0xFF00) >> 8;
          addr[3] = (xyzt & 0xFF);
          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the global task for a BGP torus address
        /// \see XMI::Mapping::Interface::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t torus2task_impl (size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS], size_t & task)
        {
          size_t xSize = __global.personality.xSize();
          size_t ySize = __global.personality.ySize();
          size_t zSize = __global.personality.zSize();
          size_t tSize = __global.personality.tSize();

          if ((addr[0] >= xSize) ||
              (addr[1] >= ySize) ||
              (addr[2] >= zSize) ||
              (addr[3] >= tSize))
            {
              return XMI_INVAL;
            }

          size_t estimated_task = ESTIMATED_TASK(addr[0], addr[1], addr[2], addr[3], xSize, ySize, zSize, tSize);

          if (unlikely(_rankcache [estimated_task] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          task = _rankcache [estimated_task];
          return XMI_SUCCESS;
        };


        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Node interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        /// \brief Get the number of possible tasks on a node
        /// \return Dimension size
        inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          TRACE_ERR((stderr, "BgpMapping::nodeTasks_impl(%zd) >>\n", global));
#warning implement this!
          TRACE_ERR((stderr, "BgpMapping::nodeTasks_impl(%zd) <<\n", global));
          return XMI_UNIMPL;
        };

        ///
        /// \brief Number of active tasks on the local node.
        /// \see XMI::Mapping::Interface::Node::nodePeers()
        ///
        inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          peers = _peers;
          return XMI_SUCCESS;
        };

        /// \brief Get the node address for the local task
        inline void nodeAddr_impl (Interface::nodeaddr_t & addr)
        {
          TRACE_ERR((stderr, "BgpMapping::nodeAddr_impl() >>\n"));
          addr = _nodeaddr;
          //global = _nodeaddr.global;
          //local  = _nodeaddr.local;
          TRACE_ERR((stderr, "BgpMapping::nodeAddr_impl(%zd, %zd) <<\n", addr.global, addr.local));
        };

        /// \brief Get the node address for a specific task
        inline xmi_result_t task2node_impl (size_t task, Interface::nodeaddr_t & addr)
        {
          TRACE_ERR((stderr, "BgpMapping::task2node_impl(%zd) >>\n", task));
          //fprintf(stderr, "BgpMapping::task2node_impl() .. _mapcache[%zd] = 0x%08x\n", task, _mapcache[task]);
          //fprintf(stderr, "BgpMapping::task2node_impl() .. _mapcache[%zd] = 0x%08x &_mapcache[%zd] = %p\n", task, _mapcache[task], task, &_mapcache[task]);
          size_t coords[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS];
          task2torus_impl(task, coords);
          addr.global = ESTIMATED_TASK_GLOBAL(coords[0], coords[1], coords[2], coords[3], xSize(), ySize(), zSize(), tSize());
          addr.local = ESTIMATED_TASK_LOCAL(coords[0], coords[1], coords[2], coords[3], xSize(), ySize(), zSize(), tSize());
          TRACE_ERR((stderr, "BgpMapping::task2node_impl(%zd, %zd, %zd) <<\n", task, addr.global, addr.local));
          return XMI_SUCCESS;
        };

        /// \brief Get the task associated with a specific node address
        inline xmi_result_t node2task_impl (Interface::nodeaddr_t addr, size_t & task)
        {
          TRACE_ERR((stderr, "BgpMapping::node2task_impl(%zd, %zd) >>\n", addr.global, addr.local));
          size_t estimated_task = ESTIMATED_TASK_NODE(addr.global, addr.local, xSize(), ySize(), zSize(), tSize());
          task = _rankcache [estimated_task];
          TRACE_ERR((stderr, "BgpMapping::node2task_impl(%zd, %zd, %zd) <<\n", addr.global, addr.local, task));
          return XMI_SUCCESS;
        };

        /// \brief Get the peer identifier associated with a specific node address
        inline xmi_result_t node2peer_impl (Interface::nodeaddr_t & addr, size_t & peer)
        {
          TRACE_ERR((stderr, "BgpMapping::node2peer_impl(%zd, %zd) >>\n", addr.global, addr.local));

          peer = _peercache[addr.local];

          TRACE_ERR((stderr, "BgpMapping::node2peer_impl(%zd, %zd, %zd) <<\n", addr.global, addr.local, peer));
          return XMI_SUCCESS;
        };
    };
  };
};

xmi_result_t XMI::Mapping::BgpMapping::init_impl (xmi_coord_t &ll, xmi_coord_t &ur,
                                                  size_t &min_rank, size_t &max_rank,
                                                  XMI::Memory::SharedMemoryManager &mm)
{
  //fprintf (stderr, "BgpMapping::init_impl >>\n");
  _mapcache  = __global.getMapCache();
  _rankcache = __global.getRankCache();
  _task = __global.getTask();
  _size = __global.getSize();

  __global.getMappingInit(ll, ur, min_rank, max_rank);

  unsigned i;

  for (i = 0; i < _size; i++) task2node (i, _nodeaddr);

  task2node (_task, _nodeaddr);

  size_t peer = 0;
  size_t task;
  xmi_network dummy;
  xmi_coord_t c;
  c.network = XMI_N_TORUS_NETWORK;
  c.u.n_torus.coords[0] = _x;
  c.u.n_torus.coords[1] = _y;
  c.u.n_torus.coords[2] = _z;
  _peers = 0;

  for (c.u.n_torus.coords[3] = 0; c.u.n_torus.coords[3] < __global.personality.tSize(); c.u.n_torus.coords[3]++)
    {
      if (network2task(&c, &task, &dummy) == XMI_SUCCESS)
        {
          //fprintf (stderr, "BgpMapping::init_impl .. _peercache[%zd] = %zd\n", c.n_torus.coords[3], peer);
          _peercache[c.u.n_torus.coords[3]] = peer++;
          _localranks[_peers] = task;
          _peers++;
        }
    }

  // Find the maximum task id and the minimum task id.
  for (c.u.n_torus.coords[0] = 0; c.u.n_torus.coords[0] < __global.personality.xSize(); c.u.n_torus.coords[0]++)
    {
      for (c.u.n_torus.coords[1] = 0; c.u.n_torus.coords[1] < __global.personality.ySize(); c.u.n_torus.coords[1]++)
        {
          for (c.u.n_torus.coords[2] = 0; c.u.n_torus.coords[2] < __global.personality.zSize(); c.u.n_torus.coords[2]++)
            {
              for (c.u.n_torus.coords[3] = 0; c.u.n_torus.coords[3] < __global.personality.xSize(); c.u.n_torus.coords[3]++)
                {
                  if (network2task (&c, &task, &dummy) == XMI_SUCCESS)
                    {
                      if (task < min_rank) min_rank = task;

                      if (task > max_rank) max_rank = task;
                    }
                }
            }
        }
    }

  //fprintf (stderr, "BgpMapping::init_impl <<\n");

  return XMI_SUCCESS;
};
#undef TRACE_ERR
#endif // __components_mapping_bgp_bgpmapping_h__
