/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpidevice.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpidevice_h__
#define __components_devices_mpi_mpidevice_h__

#include "Global.h"

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/mpi/mpimessage.h"
#include "components/devices/generic/Device.h"
#include <map>
#include <list>
#include "util/ccmi_debug.h"
#include "SysDep.h"

#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
#define DISPATCH_SET_SIZE 256
    typedef struct mpi_dispatch_info_t
    {
      XMI::Device::Interface::RecvFunction_t  recv_func;
      void                      *recv_func_parm;
    }mpi_dispatch_info_t;

    typedef struct mpi_oldmcast_dispatch_info_t
    {
      xmi_olddispatch_multicast_fn  recv_func;
      void                         *async_arg;
    }mpi_oldmcast_dispatch_info_t;

    typedef struct mpi_mcast_dispatch_info_t
    {
      xmi_dispatch_multicast_fn  recv_func;
      void                         *async_arg;
    }mpi_mcast_dispatch_info_t;

    typedef struct mpi_m2m_dispatch_info_t
    {
      xmi_olddispatch_manytomany_fn  recv_func;
      void                          *async_arg;
    }mpi_m2m_dispatch_info_t;

    class MPIDevice : public Interface::BaseDevice<MPIDevice>,
                      public Interface::PacketDevice<MPIDevice>
    {
    public:
      static const size_t packet_payload_size = 224;
      inline MPIDevice () :
      Interface::BaseDevice<MPIDevice> (),
      Interface::PacketDevice<MPIDevice>(),
      _dispatch_id(0),
      _curMcastTag(MULTISYNC_TAG)
      {
        // \todo These MPI functions should probably move out of the constructor if we keep this class
        MPI_Comm_dup(MPI_COMM_WORLD,&_communicator);
        MPI_Comm_size(_communicator, (int*)&_peers);
        TRACE_DEVICE((stderr,"<%p>MPIDevice()\n",this));
      };

	class Factory : public Interface::FactoryInterface<Factory,MPIDevice,Generic::Device> {
	public:

static inline MPIDevice *generate_impl(size_t clientid, size_t num_ctx, Memory::MemoryManager & mm) {
	size_t x;
	MPIDevice *devs;
	int rc = posix_memalign((void **)&devs, 16, sizeof(*devs) * num_ctx);
	XMI_assertf(rc == 0, "posix_memalign failed for MPIDevice[%zd], errno=%d\n", num_ctx, errno);
	for (x = 0; x < num_ctx; ++x) {
		new (&devs[x]) MPIDevice();
	}
	return devs;
}

static inline xmi_result_t init_impl(MPIDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::Memory::MemoryManager *mm, XMI::Device::Generic::Device *devices) {
	return getDevice_impl(devs, client, contextId).init_impl(mm, client, 0, ctx, contextId);
}

static inline size_t advance_impl(MPIDevice *devs, size_t client, size_t contextId) {
	return getDevice_impl(devs, client, contextId).advance_impl();
}

static inline MPIDevice & getDevice_impl(MPIDevice *devs, size_t client, size_t contextId) {
	return devs[contextId];
}
	}; // class Factory


      // Implement BaseDevice Routines

      inline ~MPIDevice () {};


      int registerRecvFunction (size_t                     dispatch,
                                XMI::Device::Interface::RecvFunction_t  recv_func,
                                void                      *recv_func_parm)
        {
          unsigned i;
          TRACE_DEVICE((stderr,"<%p>MPIDevice::registerRecvFunction dispatch %zd/%zd\n",this,dispatch,dispatch * DISPATCH_SET_SIZE));
          for (i=0; i<DISPATCH_SET_SIZE; i++)
          {
            unsigned id = dispatch * DISPATCH_SET_SIZE + i;
            if (_dispatch_table[id].recv_func == NULL)
            {
              _dispatch_table[id].recv_func=recv_func;
              _dispatch_table[id].recv_func_parm=recv_func_parm;
              _dispatch_lookup[id] = _dispatch_table[id];

              return id;
            }
          }
          return -1;
        }

      int initOldMcast()
      {
        return _oldmcast_dispatch_id++;
      }

      int initMcast()
      {
        return _mcast_dispatch_id++;
      }

      int initM2M()
      {
        return _m2m_dispatch_id++;
      }

      void registerOldMcastRecvFunction (int                           dispatch_id,
                                      xmi_olddispatch_multicast_fn  recv_func,
                                      void                         *async_arg)
      {
        _oldmcast_dispatch_table[dispatch_id].recv_func=recv_func;
        _oldmcast_dispatch_table[dispatch_id].async_arg=async_arg;
        _oldmcast_dispatch_lookup[dispatch_id]=_oldmcast_dispatch_table[dispatch_id];
        TRACE_DEVICE((stderr,"<%p>MPIDevice::registerMcastRecvFunction %d\n",this,_dispatch_id));
      }

      void registerMcastRecvFunction (int                           dispatch_id,
                                      xmi_dispatch_multicast_fn     recv_func,
                                      void                         *async_arg)
      {
        _mcast_dispatch_table[dispatch_id].recv_func=recv_func;
        _mcast_dispatch_table[dispatch_id].async_arg=async_arg;
        _mcast_dispatch_lookup[dispatch_id]=_mcast_dispatch_table[dispatch_id];
        TRACE_DEVICE((stderr,"<%p>MPIDevice::registerMcastRecvFunction %d\n",this,_dispatch_id));
      }

      void registerM2MRecvFunction (int                           dispatch_id,
                                    xmi_olddispatch_manytomany_fn  recv_func,
                                    void                         *async_arg)
      {
        _m2m_dispatch_table[dispatch_id].recv_func=recv_func;
        _m2m_dispatch_table[dispatch_id].async_arg=async_arg;
        _m2m_dispatch_lookup[dispatch_id]=_m2m_dispatch_table[dispatch_id];
        TRACE_DEVICE((stderr,"<%p>MPIDevice::registerM2MRecvFunction %d\n",this,_dispatch_id));
      }

      inline xmi_result_t init_impl (Memory::MemoryManager *mm, size_t clientid, size_t num_ctx, xmi_context_t context, size_t offset)
      {
        _context = context;
        _contextid = offset;

        return XMI_SUCCESS;
      };

      inline xmi_context_t getContext_impl ()
      {
        return _context;
      };

      inline size_t getContextOffset_impl ()
      {
        return _contextid;
      };

      inline bool isInit_impl ()
      {
        assert(0);
        return false;
      };
      inline bool isReliableNetwork ()
      {
        return true;
      };

      inline int advance_impl ()
      {
        static int dbg = 1;
        int flag = 0;
        MPI_Status sts;
        int events=0;

        if(dbg) {
          TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl\n",this));
          dbg = 0;
        }
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
        // Check the P2P *pending* send queue
        while (!_pendingQ.empty())
        {
          MPIMessage * msg = _pendingQ.front();
          _pendingQ.pop_front();
          MPI_Isend (&msg->_p2p_msg,
                     sizeof(msg->_p2p_msg),
                     MPI_CHAR,
                     msg->_target_task,
                     0,
                     _communicator,
                     &msg->_request);
          enqueue(msg);
        }
#endif
        // Check the P2P send queue
        std::list<MPIMessage*>::iterator it_p2p;
        for(it_p2p=_sendQ.begin();it_p2p != _sendQ.end(); it_p2p++)
        {
          flag            = 0;
          MPI_Testall(1,&((*it_p2p)->_request),&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl() p2p\n",this)); dbg = 1;
            xmi_event_function  done_fn = (*it_p2p)->_done_fn;
            void               *cookie  = (*it_p2p)->_cookie;
            xmi_client_t       client = (*it_p2p)->_client;
            size_t       context = (*it_p2p)->_context;
            _sendQ.remove((*it_p2p));
            if((*it_p2p)->_freeme)
              free(*it_p2p);

            if(done_fn)
              done_fn(NULL,//XMI_Client_getcontext(client,context),  \todo fix this
                      cookie,XMI_SUCCESS);
            break;
          }
        }
        // Check the Multicast send queue
        std::list<OldMPIMcastMessage*>::iterator it_mcast;
        for(it_mcast=_oldmcastsendQ.begin();it_mcast != _oldmcastsendQ.end(); it_mcast++)
        {
          int numStatuses = (*it_mcast)->_num;
          flag            = 0;
          MPI_Testall(numStatuses,(*it_mcast)->_req,&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl mc\n",this)); dbg = 1;
            if((*it_mcast)->_cb_done.function )
              (*(*it_mcast)->_cb_done.function)(NULL,//XMI_Client_getcontext((*it_mcast)->_client,(*it_mcast)->_context),   \todo fix this
                                                (*it_mcast)->_cb_done.clientdata, XMI_SUCCESS);
            free ((*it_mcast)->_req);
            free (*it_mcast);
            _oldmcastsendQ.remove((*it_mcast));
            break;
          }
        }
        // Check the M2M send Queue
        std::list<MPIM2MMessage*>::iterator it;
        for(it=_m2msendQ.begin();it != _m2msendQ.end(); it++)
        {
          int numStatuses = (*it)->_num;
          flag            = 0;
          MPI_Testall(numStatuses,(*it)->_reqs,&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl m2m\n",this)); dbg = 1;
            if((*it)->_done_fn )
              ((*it)->_done_fn)(NULL, (*it)->_cookie, XMI_SUCCESS);

            free ((*it)->_reqs);
            free ((*it)->_bufs);
            _m2msendQ.remove((*it));
            free (*it);
            break;
          }
        }

        // Check the msync send Queue
        std::map<int, MPIMsyncMessage*>::iterator it_msync;
        for(it_msync=_msyncsendQ.begin();it_msync != _msyncsendQ.end(); it_msync++)
        {


          MPIMsyncMessage *m = it_msync->second;
          if(m->_sendStarted==false)
              {
                int mpi_rc = MPI_Isend (&m->_p2p_msg,
                                        sizeof(m->_p2p_msg),
                                        MPI_CHAR,
                                        m->_dests[m->_phase],
                                        MULTISYNC_TAG,
                                        _communicator,
                                        &m->_reqs[m->_phase]);
                assert(mpi_rc == MPI_SUCCESS);
                m->_sendStarted = true;
              }
          if(m->_sendDone == false)
              {
                MPI_Test(&m->_reqs[m->_phase],&flag,MPI_STATUS_IGNORE);
                if(flag)
                    {
                      m->_sendDone=true;
                    }
              }
          if(m->_sendDone == true && m->_recvDone == true)
              {
                m->_sendStarted = false;
                m->_sendDone    = false;
                m->_recvDone    = false;
                m->_phase++;
                if(m->_phase == m->_numphases)
                    {
                      m->_cb_done.function(NULL,m->_cb_done.clientdata, XMI_SUCCESS);
                      _msyncsendQ.erase(it_msync);
                      break;
                    }
              }
        }





        flag = 0;
        int rc = MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, _communicator, &flag, &sts);
        assert (rc == MPI_SUCCESS);
        if(flag)
        {
          events++;
          TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl MPI_Iprobe %d\n",this,sts.MPI_TAG)); dbg = 1;
          //p2p messages
          switch(sts.MPI_TAG)
          {
          case P2P_PACKET_TAG: // p2p packet
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIMessage *msg = (MPIMessage *) malloc (sizeof(*msg));
              int rc = MPI_Recv(&msg->_p2p_msg,nbytes,MPI_BYTE,sts.
                                MPI_SOURCE,sts.MPI_TAG,
                                _communicator,&sts);
              assert(rc == MPI_SUCCESS);
              size_t dispatch_id      = msg->_p2p_msg._dispatch_id;
              TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             this, nbytes,dispatch_id));
              _currentBuf = msg->_p2p_msg._payload;
              mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];
              if(mdi.recv_func)
                mdi.recv_func(msg->_p2p_msg._metadata,
                              msg->_p2p_msg._payload,
                              msg->_p2p_msg._payloadsize0+msg->_p2p_msg._payloadsize1,
                              mdi.recv_func_parm,
                              NULL);
              free(msg);
            }
            break;
          case P2P_PACKET_DATA_TAG: // p2p packet + data
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIMessage *msg = (MPIMessage *) malloc (sizeof(*msg)+nbytes);
              int rc = MPI_Recv(&msg->_p2p_msg,nbytes,MPI_BYTE,sts.
                                MPI_SOURCE,sts.MPI_TAG,
                                _communicator,&sts);
              assert(rc == MPI_SUCCESS);
              size_t dispatch_id      = msg->_p2p_msg._dispatch_id;
              TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             this, nbytes,dispatch_id));
              _currentBuf = (char*)msg->_p2p_msg._metadata+msg->_p2p_msg._metadatasize;
              mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];
              if(mdi.recv_func)
                mdi.recv_func(msg->_p2p_msg._metadata,
                              (char*)msg->_p2p_msg._metadata+msg->_p2p_msg._metadatasize,
                              msg->_p2p_msg._payloadsize0+msg->_p2p_msg._payloadsize1,
                              mdi.recv_func_parm,
                              NULL);
              free(msg);
            }
            break;
          case OLD_MULTICAST_TAG: // old multicast
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              OldMPIMcastMessage *msg = (OldMPIMcastMessage *) malloc (nbytes);
              assert(msg != NULL);
              int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, _communicator,&sts);
              XMI_assert (rc == MPI_SUCCESS);
              unsigned         rcvlen;
              char           * rcvbuf;
              unsigned         pwidth;
              xmi_callback_t   cb_done;
              size_t dispatch_id      = msg->_dispatch_id;
              TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             this, nbytes,dispatch_id));
              mpi_oldmcast_dispatch_info_t mdi = _oldmcast_dispatch_lookup[dispatch_id];

              OldMPIMcastRecvMessage *mcast;
              std::list<OldMPIMcastRecvMessage*>::iterator it;
              int found=0;
              for(it=_oldmcastrecvQ.begin();it != _oldmcastrecvQ.end(); it++)
              {
                if( (*it)->_conn == msg->_conn &&
                    (*it)->_dispatch_id == msg->_dispatch_id)
                {
                  found = 1;
                  break;
                }
              }
              OldMPIMcastRecvMessage _m_store;
              if( !found )
              {
                XMI_assert (mdi.recv_func != NULL);

                mdi.recv_func (&msg->_info[0],
                               msg->_info_count,
                               sts.MPI_SOURCE,
                               msg->_size,
                               msg->_conn,
                               mdi.async_arg,
                               &rcvlen,
                               &rcvbuf,
                               &pwidth,
                               &cb_done);
                assert(rcvlen <= (size_t)msg->_size);
//                          mcast = (OldMPIMcastRecvMessage*)malloc(sizeof(*mcast));
                mcast = &_m_store;
                assert(mcast != NULL);
                mcast->_conn     = msg->_conn;
                mcast->_done_fn  = cb_done.function;
                mcast->_cookie   = cb_done.clientdata;
                mcast->_buf      = rcvbuf;
                mcast->_size     = rcvlen;
                mcast->_pwidth   = pwidth;
                mcast->_hint     = XMI_PT_TO_PT_SUBTASK;
                mcast->_op       = XMI_UNDEFINED_OP;
                mcast->_dtype    = XMI_UNDEFINED_DT;
                mcast->_counter = 0;
                mcast->_dispatch_id = dispatch_id;
                enqueue(mcast);
              }
              else
              {
                mcast = (*it);
              }

              if(mcast->_pwidth == 0 && (mcast->_size == 0||mcast->_buf == 0))
              {
                if(mcast->_done_fn)
                  mcast->_done_fn (NULL,//XMI_Client_getcontext(msg->_client, msg->_context),   \todo fix this
                                   mcast->_cookie, XMI_SUCCESS);

                _oldmcastrecvQ.remove(mcast);
                free (msg);
                if(found)
                  free (mcast);

                break;
              }

              int bytes = mcast->_size - mcast->_counter;
              if(bytes > msg->_size) bytes = msg->_size;
              if(mcast->_size)
                memcpy (mcast->_buf + mcast->_counter, msg->buffer(), bytes);

              //printf ("dispatch %d matched posted receive %d %d %d %d\n",
              //	    dispatch_id,
              //	    nbytes, mcast->_pwidth, mcast->_counter,
              //	    mcast->_size);

              //for(unsigned count = 0; count < mcast->_size; count += mcast->_pwidth)
              //if(mcast->_done_fn)
              //  mcast->_done_fn(XMI_Client_getcontext(msg->_client, msg->context), mcast->_cookie, XMI_SUCCESS);

              // XMI_assert (nbytes <= mcast->_pwidth);

              for(; bytes > 0; bytes -= mcast->_pwidth)
              {
                TRACE_DEVICE((stderr,"<%p>MPIDevice::calling done counter %zd, pwidth %zd, bytes %zd, size %zd\n",
                               this, mcast->_counter,mcast->_pwidth, bytes, mcast->_size));
                mcast->_counter += mcast->_pwidth;
                if(mcast->_done_fn)
                  mcast->_done_fn(NULL,//XMI_Client_getcontext(msg->_client, msg->_context),   \todo fix this
                                  mcast->_cookie, XMI_SUCCESS);
              }

              if(mcast->_counter >= mcast->_size)
              {
                _oldmcastrecvQ.remove(mcast);
                if(found)
                  free (mcast);
              }

              free (msg);
            }
            break;
          case OLD_M2M_TAG: // old m2m
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIM2MHeader *msg = (MPIM2MHeader *) malloc (nbytes);
              int rc            = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, _communicator,&sts);
              XMI_assert (rc == MPI_SUCCESS);
              TRACE_DEVICE((stderr,"<%p>MPIDevice::advance_impl MPI_Recv nbytes %d\n",
                             this, nbytes));

              std::list<MPIM2MRecvMessage<size_t>*>::iterator it;
              for(it=_m2mrecvQ.begin();it != _m2mrecvQ.end(); it++)
              {
                if( (*it)->_conn == msg->_conn ) break;
              }

              mpi_m2m_dispatch_info_t mdi = _m2m_dispatch_lookup[msg->_dispatch_id];
              MPIM2MRecvMessage<size_t> * m2m;
              if( it == _m2mrecvQ.end() )
              {
                xmi_callback_t    cb_done;
                char            * buf;
                size_t        * sizes;
                size_t        * offsets;
                size_t        * rcvcounters;
                size_t          nranks;
                mdi.recv_func(msg->_conn,
                              mdi.async_arg,
                              &buf,
                              &offsets,
                              &sizes,
                              &rcvcounters,
                              &nranks,
                              &cb_done );
                m2m = (MPIM2MRecvMessage<size_t> *)malloc(sizeof(MPIM2MRecvMessage<size_t>) );
                XMI_assert ( m2m != NULL );
                m2m->_conn = msg->_conn;
                m2m->_done_fn = cb_done.function;
                m2m->_cookie  = cb_done.clientdata;
                m2m->_num = 0;
                for( unsigned i = 0; i < nranks; i++)
                {
                  if( sizes[i] == 0 ) continue;
                  m2m->_num++;
                }
                if( m2m->_num == 0 )
                {
                  if( m2m->_done_fn )
                    (m2m->_done_fn)(NULL, m2m->_cookie,XMI_SUCCESS);
                  free ( m2m );
                  return NULL;
                }
                m2m->_buf     = buf;
                m2m->_sizes   = sizes;
                m2m->_offsets = offsets;
                m2m->_nranks  = nranks;
                enqueue(m2m);
              }
              else
              {
                m2m = (*it);
              }
              unsigned src = sts.MPI_SOURCE;
              if( m2m )
              {
                unsigned size = msg->_size < m2m->_sizes[src] ? msg->_size : m2m->_sizes[src];
                XMI_assert( size > 0 );
                memcpy( m2m->_buf + m2m->_offsets[src], msg->buffer(), size );
                m2m->_num--;
                if( m2m->_num == 0 )
                {
                  if( m2m->_done_fn )
                  {
                    m2m->_done_fn(NULL, m2m->_cookie,XMI_SUCCESS);
                  }
                  _m2mrecvQ.remove(m2m);
                  free ( m2m );
                }
              }
              free ( msg );
            }
            break;
         case MULTISYNC_TAG: // old m2m
             {
               unsigned conn_id;
               int      nbytes;
               MPI_Get_count(&sts, MPI_BYTE, &nbytes);
               int rc            = MPI_Recv(&conn_id,
                                            sizeof(conn_id),
                                            MPI_BYTE,
                                            sts.MPI_SOURCE,
                                            sts.MPI_TAG,
                                            _communicator,
                                            &sts);
               XMI_assert (rc == MPI_SUCCESS);
               MPIMsyncMessage *m = _msyncsendQ[conn_id];
               if (m)
                 m->_recvDone = true;
               else
                 _msyncsendQ[conn_id] = (MPIMsyncMessage*)0x1;
             }
             break;
         default:
           XMI_abort();
          }
        }
        return events;
      };

      // Implement MessageDevice Routines
      static const size_t metadata_size = 128;
      static const size_t payload_size  = 224;

      // Implement Packet Device Routines
      inline int    read_impl(void * dst, size_t bytes, void * cookie)
      {
        memcpy(dst, _currentBuf, bytes);
        return -1;
      }

      inline size_t peers_impl ()
      {
        return _peers;
      }

      inline size_t task2peer_impl (size_t task)
      {
        assert(task < _peers);
        return task;
      }

      inline bool isPeer_impl (size_t task)
      {
#if 0
        XMI::Interface::Mapping::nodeaddr_t node;
        size_t peer;

        __global.mapping.task2node(task,node);
        xmi_result_t result = __global.mapping.node2peer(node,peer);

        return result == XMI_SUCCESS;
#else
        return false;
#endif
      }


      inline void enqueue(MPIMessage* msg)
      {
        TRACE_DEVICE((stderr,
                      "<%p>MPIDevice::enqueue message size 0 %zd, size 1 %zd, msize %zd\n",
                      this,
                      (size_t)msg->_p2p_msg._payloadsize0,
                      (size_t)msg->_p2p_msg._payloadsize1,
                      (size_t)msg->_p2p_msg._metadatasize));
        _sendQ.push_front(msg);
      }

      inline void enqueue(OldMPIMcastMessage* msg)
      {
        TRACE_DEVICE((stderr,"<%p>MPIDevice::enqueue mcast message size %zd\n",this, (size_t)msg->_size));
        _oldmcastsendQ.push_front(msg);
      }

      inline void enqueue(OldMPIMcastRecvMessage *msg)
      {
        TRACE_DEVICE((stderr,
                      "<%p>MPIDevice::enqueue mcast recv message pwidth %zd size %zd\n",
                      this,
                      (size_t)msg->_pwidth,
                      (size_t)msg->_size));
        _oldmcastrecvQ.push_front(msg);
      }

      inline void enqueue(MPIMcastMessage* msg)
      {
        TRACE_DEVICE((stderr,"<%p>MPIDevice::enqueue mcast message size %zd\n",this, (size_t)msg->_size));
        _mcastsendQ.push_front(msg);
      }

      inline void enqueue(MPIMcastRecvMessage *msg)
      {
        TRACE_DEVICE((stderr,
                      "<%p>MPIDevice::enqueue mcast recv message pwidth %zd size %zd\n",
                      this,
                      (size_t)msg->_pwidth,
                      (size_t)msg->_size));
        _mcastrecvQ.push_front(msg);
      }




      inline void enqueue(MPIM2MRecvMessage<size_t> *msg)
      {
        TRACE_DEVICE((stderr,
                      "<%p>MPIDevice::enqueue m2m recv message size %zd\n",
                      this,
                      (size_t)msg->_sizes[0]));
        _m2mrecvQ.push_front(msg);
      }

      inline void enqueue(MPIM2MMessage *msg)
      {
        TRACE_DEVICE((stderr,
                      "<%p>MPIDevice::enqueue m2m message total size %zd\n",
                      this,
                      (size_t)msg->_totalsize));
        _m2msendQ.push_front(msg);
      }

      inline void enqueue(MPIMsyncMessage *msg)
      {
        if(_msyncsendQ[msg->_p2p_msg._connection_id] == (MPIMsyncMessage*)0x1)
          msg->_recvDone = true;
        _msyncsendQ[msg->_p2p_msg._connection_id] = msg;
      }

      inline void addToNonDeterministicQueue(MPIMessage* msg, unsigned long long random)
      {
        size_t index, insert = random % _pendingQ.size();
        std::list<MPIMessage*>::iterator it;
        for (index = 0; index < insert; index++) it++;
        _pendingQ.insert(it,msg);
      }

      MPI_Comm                                  _communicator;
      char                                     *_currentBuf;
      size_t                                    _peers;
      size_t                                    _dispatch_id;
      size_t                                    _oldmcast_dispatch_id;
      size_t                                    _mcast_dispatch_id;
      size_t                                    _m2m_dispatch_id;
      std::map<int, mpi_dispatch_info_t>        _dispatch_lookup;
      std::map<int, mpi_oldmcast_dispatch_info_t> _oldmcast_dispatch_lookup;
      std::map<int, mpi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::map<int, mpi_m2m_dispatch_info_t>    _m2m_dispatch_lookup;
      std::list<MPIMessage*>                    _sendQ;
      std::list<OldMPIMcastMessage*>            _oldmcastsendQ;
      std::list<MPIMcastMessage*>               _mcastsendQ;
      std::list<MPIM2MMessage*>                 _m2msendQ;
      std::list<OldMPIMcastRecvMessage*>        _oldmcastrecvQ;
      std::list<MPIMcastRecvMessage*>           _mcastrecvQ;
      std::list<MPIM2MRecvMessage<size_t> *>    _m2mrecvQ;
      std::list<MPIMessage*>                    _pendingQ;
      std::map<int,MPIMsyncMessage*>            _msyncsendQ;
      mpi_dispatch_info_t                       _dispatch_table[256*DISPATCH_SET_SIZE];
      mpi_mcast_dispatch_info_t                 _mcast_dispatch_table[256];
      mpi_oldmcast_dispatch_info_t              _oldmcast_dispatch_table[256];
      mpi_m2m_dispatch_info_t                   _m2m_dispatch_table[256];
      int                                       _curMcastTag;
      xmi_context_t                             _context;
      size_t                                    _contextid;
    };
#undef DISPATCH_SET_SIZE
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
