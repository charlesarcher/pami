/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cshmem/CollShmDevice.h
 * \brief ???
 */

#ifndef __components_devices_cshmem_CollShmDevice_h__
#define __components_devices_cshmem_CollShmDevice_h__

#include "math/math_coremath.h"
#include "Global.h"
#include "Platform.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/FactoryInterface.h"
#include "components/memory/shmem/CollSharedMemoryManager.h"
#include "common/default/PipeWorkQueue.h"
#include "components/atomic/xlc/XlcBuiltinT.h"

#undef TRACE_ERR
#define TRACE_ERR(x) // fprintf x

#undef TRACE_DBG
#define TRACE_DBG(x) // printf x 

#undef PAMI_ASSERT
#define PAMI_ASSERT(x) assert(x)

#undef INLINE
#define INLINE 
//#define INLINE inline

namespace PAMI {
namespace Device {
namespace CollShm {

typedef union {
    pami_multicast_t mcast;
    pami_multisync_t msync;
    pami_multicombine_t mcombine;
} collshm_multi_t;

typedef enum {
    Generic = 0,
    MultiCast,
    MultiSync,
    MultiCombine
} collshm_msgtype_t; 

template <typename T_Multi> 
struct message_type 
{
   static const unsigned msgtype = Generic;
};
template <> struct message_type<pami_multicast_t> { static const collshm_msgtype_t msgtype = MultiCast; };
template <> struct message_type<pami_multisync_t> { static const collshm_msgtype_t msgtype = MultiSync; };
template <> struct message_type<pami_multicombine_t> { static const collshm_msgtype_t msgtype = MultiCombine; };

///
/// \brief BaseCollShmMessage provides the getMsgType method inherited by 
///        all collective shm messages in the device message queue. 
///
class BaseCollShmMessage : public PAMI::Device::Generic::GenericMessage {

  public :
    ///
    /// \brief Constructor for common collective shmem message
    ///
    INLINE BaseCollShmMessage(GenericDeviceMessageQueue *device, pami_callback_t cb, size_t client, size_t context):
       PAMI::Device::Generic::GenericMessage (device, cb, client, context)
    { }

    ///
    /// \brief get the message type
    ///
    INLINE collshm_msgtype_t getMsgType() {return _msgtype;}

  protected :
    collshm_msgtype_t _msgtype;
}; // BaseCollShmMessage

template <typename T_Multi, class T_CollShmDevice>
class CollShmMessage : public BaseCollShmMessage {
    public:

        ///
        /// \brief Constructor for common collective shmem message 
        ///
        INLINE CollShmMessage(GenericDeviceMessageQueue *device, T_Multi *multi):
          BaseCollShmMessage (device, multi->cb_done, multi->client, multi->context),
          _multi (multi)
        {
          //TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
          _msgtype =  message_type< T_Multi >::msgtype; 
        }

        INLINE pami_context_t postNext(bool devQueued) {
          T_CollShmDevice * qs = static_cast<T_CollShmDevice *>(getQS());
          return qs->postNextMsg();
        }

        ///
        /// \brief get the multi* calling parameters, read only by using a pointer to const     
        ///
        INLINE T_Multi *getMulti() { return _multi; }

    private:
        T_Multi *_multi; 
}; // class CollShmMessage

///
/// \brief CollShmDevice class is message queue on a geometry's local topology. The messags and work threads
///        will be posted on generic devices. The CollShmDevice maintains a number of threads, each representing
///        work posted on one collective shared memory channel. There is a one-to-one relationship
///        between an active posted message, a thread and a collective shared memory channel
///        Template parameters T_NumSyncs and T_SyncCount define the circular queue usage of the threads
///        The array of threads are divided in to T_NumSyncs consecutive groups, each consisting of T_SyncCount
///        threads. Each group of threads has a corresponding barrier counters in shared memory. Whenever a 
///        task completes the whole group of threads locally, it checks in the corresponding barrier. The 
///        group of threads can be reused when the barrier completes. Both T_NumSyncs and T_SyncCount must be
///        power of two. 
///
template <class T_Atomic, class T_MemoryManager, unsigned T_NumSyncs, unsigned T_SyncCount>
class CollShmDevice : public GenericDeviceMessageQueue 
{

public:
    static const unsigned _numsyncs    = T_NumSyncs;
    static const unsigned _synccounts  = T_SyncCount;
    static const unsigned _numchannels = T_NumSyncs * T_SyncCount;

    //typedef CollShmDevice<T_Atomic, T_MemoryManager, T_NumSyncs, T_SyncCount> CollShmDeviceType;

    class CollShmWindow {

      public:

      typedef  enum {
        NODATA    = 0,
        IMMEDIATE,
        COPYINOUT,
        XMEMATT
      } window_content_t;

      typedef struct window_control_t {
        volatile window_content_t content : 8; // type of data
        volatile unsigned char    sync_flag;   // for barrier
        volatile unsigned char    avail_flag;  // 0 indicates no data, non-zero indicates
                                               // data ready and reuse stage.
        volatile unsigned char    flags;       // flags reserved for active message collective,
                                               // non-blocking collective etc
        T_Atomic                  cmpl_cntr;   // for producer only, updated by
                                               // consumers through atomic operations
      } window_control_t;

      ///
      /// \brief for xmem or shared address
      ///
      typedef struct xmem_descriptor_t {
        volatile _css_mem_hndl_t hndl;
        volatile char            *src;
      } xmem_descriptor_t;

      #define IMMEDIATE_CHANNEL_DATA_SIZE (CACHEBLOCKSZ-sizeof(window_control_t)-2*sizeof(char*))

      typedef union window_data_t {
         xmem_descriptor_t xmem_data;
         // char immediate_data[0];
         char immediate_data[IMMEDIATE_CHANNEL_DATA_SIZE];
      } window_data_t;

      //#define COMBINE_DATA(dest, src, len, flag) {\
      //  if (flag) {    \
      //    int i;       \
      //    for (i=0; i<len; i++) { \
      //      ((char *)dest)[i] = ((char *)dest)[i]+ ((char *)src)[i]; \
      //    }                           \
      //  } else {       \
      //    memcpy (dest, src, len);  \
      //  } \
      //}

      CollShmWindow() :
      _buf(NULL),
      _len(0)
      {
        clearCtrl();
        prepData();
      }

      INLINE void combineData(void *dst, void *src, size_t len, int flag, pami_op op, pami_dt dt) 
      {
        if (flag) {
          coremath func = MATH_OP_FUNCS(dt, op, 2);
          int dtshift  = pami_dt_shift[dt];
          void * buf[] = { dst, src };
          unsigned count = len >> dtshift;
  
          func (dst, buf, 2, count);

        } else {
          memcpy (dst, src, len); 
        }
      }

      INLINE void clearCtrl()
      {
        *((long long *)&_ctrl) = 0x0LL;
      }

      INLINE void prepData() { }

      INLINE window_content_t getContent() { return _ctrl.content; }
      INLINE void setContent(window_content_t content) 
      { 
        mem_barrier(); // lwsync();
        _ctrl.content = content; 
      }

      INLINE int getSyncflag() { return _ctrl.sync_flag; }
      INLINE void setSyncflag(int sync_flag) { _ctrl.sync_flag = sync_flag; }

      INLINE size_t getCmpl() { return _ctrl.cmpl_cntr.fetch(); }
      ///
      /// \brief Increment completion counter 
      ///
      INLINE void setCmpl()
      {
        PAMI_ASSERT(_ctrl.cmpl_cntr.fetch() >= 1);
        // COLLSHM_FETCH_AND_ADD((atomic_p)&(_ctrl.cmpl_cntr), -1);
        _ctrl.cmpl_cntr.fetch_and_dec();
      }

      ///
      /// \brief Set data available flag and completion counter to watch for 
      ///        when consumers all finished using the data
      ///
      INLINE pami_result_t setAvail(unsigned avail_value, unsigned cmpl_value)
      {
        PAMI_ASSERT(_ctrl.cmpl_cntr.fetch() == 0);
        _ctrl.cmpl_cntr.fetch_and_add(cmpl_value); // one way to make using the right counter atomic

        mem_barrier(); //lwsync();
        PAMI_ASSERT(_ctrl.avail_flag != avail_value);
        _ctrl.avail_flag = avail_value;

        // immediate satisfaction 
        if (_ctrl.cmpl_cntr.fetch() == 1) {
          _ctrl.cmpl_cntr.clear();
          if (_ctrl.content == XMEMATT) returnBuffer(NULL);
          return PAMI_SUCCESS;
        }
        return PAMI_EAGAIN;
      }

      ///
      /// \brief Check the availibilty of the data or buffer 
      ///
      INLINE pami_result_t isAvail(unsigned value)
      {
        if (_ctrl.avail_flag != value) return PAMI_EAGAIN;
      
        mem_isync(); //isync();
        return PAMI_SUCCESS;
      }

      ///
      /// \brief Producer prepares data or buffer for sharing with consumers
      ///        PipeWorkQueue version
      ///
      /// \param src    source pipeworkqueue
      /// \param length length of data to produce into the window
      /// \parma _csmm  pointer to memeory manager in case a shared memory buffer is needed
      ///
      /// \return length of data produced into the window
      ///         -1 indicates there is no data buffer 
      ///          associated with the window and no memory
      ///          manager specified, or no memory available 

      INLINE size_t produceData(PAMI::PipeWorkQueue &src, size_t length, T_MemoryManager *csmm)
      {

        if (src.bytesAvailableToConsume() < MIN(length, COLLSHM_BUFSZ) ) return 0;

        size_t reqbytes = MIN (length, src.bytesAvailableToConsume());

        if (reqbytes < IMMEDIATE_CHANNEL_DATA_SIZE)
        {
          _len = reqbytes;
          memcpy (_data.immediate_data, src.bufferToConsume(), _len);
          src.consumeBytes(_len);
          _ctrl.content = IMMEDIATE;
  
        } else if (reqbytes <= XMEM_THRESH) {

          _len = MIN(reqbytes, COLLSHM_BUFSZ);
          if (!_buf) {
            if (NULL != csmm) {
              _buf = (char *)csmm->getDataBuffer(1);
            }
            if (!_buf) return -1;
          }

          memcpy (_buf, src.bufferToConsume(), _len);
          src.consumeBytes(_len);
          _ctrl.content = COPYINOUT;
 
        } else {
#ifdef __64BIT__
          _css_shmem_reg_info_t  reg;
          // PAMI_ASSERT( XMEM );
          reg.command  = CSS_SHMEM_REG;
          reg.pointer  = (long long)src.bufferToConsume();
          reg.len      = (long long)length;
          reg.hndl_out = ZCMEM_HNDL_NULL;
          PAMI_ASSERT(! _css_shmem_register((zcmem_t)&reg));
          _len = reqbytes;

          _data.xmem_data.src    = (volatile char *)src.bufferToConsume();
          _data.xmem_data.hndl   = reg.hndl_out;
          _ctrl.content    = XMEMATT;
#endif
        }

        return _len;
      }

      ///
      /// \brief Producer prepares data or buffer for sharing with consumers
      ///        Flat buffer version, src is a pipeworkqueue
      ///
      INLINE size_t produceData(char *src, size_t length, T_MemoryManager *csmm)
      {
        if (length < IMMEDIATE_CHANNEL_DATA_SIZE)
        {
          _len = length;
          memcpy (_data.immediate_data, src, _len);
          _ctrl.content = IMMEDIATE;

        } else if (length <= XMEM_THRESH) {

          _len = MIN(length, COLLSHM_BUFSZ);
          if (!_buf) {
            if (NULL != csmm) {
              _buf = (char *)csmm->getDataBuffer(1);
            }
            if (!_buf) return -1;
          }

          memcpy (_buf, src, _len);
          _ctrl.content = COPYINOUT;

        } else {
#ifdef __64BIT__
          _css_shmem_reg_info_t  reg;
          //PAMI_ASSERT( XMEM );
          reg.command  = CSS_SHMEM_REG;
          reg.pointer  = (long long)src;
          reg.len      = (long long)length;
          reg.hndl_out = ZCMEM_HNDL_NULL;
          PAMI_ASSERT(! _css_shmem_register((zcmem_t)&reg));
          _len = length;

          _data.xmem_data.src    = (volatile char *)src;
          _data.xmem_data.hndl   = reg.hndl_out;
          _ctrl.content    = XMEMATT;
#endif
        }

        return _len;
      }

      ///
      /// \brief Consumer consumes data, PipeWorkQueue version, destination is a pipeworkqueue 
      ///
      INLINE size_t consumeData(PAMI::PipeWorkQueue &dest, size_t length, int combine_flag, pami_op op, pami_dt dt)
      {
        if (dest.bytesAvailableToProduce() < MIN(length, _len)) return 0; 
        size_t reqbytes = MIN (length, dest.bytesAvailableToProduce());
        size_t len      = MIN (reqbytes, _len);
        char *src;
        switch (_ctrl.content)
        {
          case IMMEDIATE:
            combineData(dest.bufferToProduce(), _data.immediate_data, len, combine_flag, op, dt);
            break;
          case COPYINOUT:
            combineData(dest.bufferToProduce(), _buf, len, combine_flag, op, dt);
            break;
#ifdef __64BIT__
          case XMEMATT:
            // PAMI_ASSERT( XMEM );
            PAMI_ASSERT( _data.xmem_data.hndl != ZCMEM_HNDL_NULL);
            _css_shmem_att_info_t  att;
            att.command  = CSS_SHMEM_ATT;
            att.hndl_att = _data.xmem_data.hndl;
            att.req_ptr  = (long long)_data.xmem_data.src;
            att.req_len = (long long)len;
            att.offset   = (long long) 0;
            PAMI_ASSERT(! _css_shmem_attach((zcmem_t)&att));
            PAMI_ASSERT(att.llen_avail ==att.req_len);
            src = (char *) (att.pointer + att.latt_offset);
            combineData(dest.bufferToProduce(), src, len, combine_flag, op, dt);
            att.command = CSS_SHMEM_DET;
            att.hndl_det = att.hndl_att;
            PAMI_ASSERT(! _css_shmem_attach((zcmem_t)&att) );
            break;
#endif
          default:
            TRACE_DBG(("valus of content is %d\n", _ctrl.content));
            PAMI_ASSERT(0);
        }

        dest.produceBytes(len);
        return len;
      }

      ///
      /// \brief Consumer consumes data, destination is a flat buffer
      ///
      INLINE size_t consumeData(char *dest, size_t length, int combine_flag, pami_op op, pami_dt dt)
      {
        size_t len      = MIN (length, _len);
        char *src;
        switch (_ctrl.content)
        {
          case IMMEDIATE:
            combineData(dest, _data.immediate_data, len, combine_flag, op, dt);
            break;
          case COPYINOUT:
            combineData(dest, _buf, len, combine_flag, op, dt);
            break;
#ifdef __64BIT__
          case XMEMATT:
            // PAMI_ASSERT( XMEM );
            PAMI_ASSERT( _data.xmem_data.hndl != ZCMEM_HNDL_NULL);
            _css_shmem_att_info_t  att;
            att.command  = CSS_SHMEM_ATT;
            att.hndl_att = _data.xmem_data.hndl;
            att.req_ptr  = (long long)_data.xmem_data.src;
            att.req_len = (long long)len;
            att.offset   = (long long) 0;
            PAMI_ASSERT(! _css_shmem_attach((zcmem_t)&att));
            PAMI_ASSERT(att.llen_avail ==att.req_len);
            src = (char *) (att.pointer + att.latt_offset);
            combineData(dest, src, len, combine_flag, op, dt);
            att.command = CSS_SHMEM_DET;
            att.hndl_det = att.hndl_att;
            PAMI_ASSERT(! _css_shmem_attach((zcmem_t)&att) );
            break;
#endif
          default:
            TRACE_DBG(("valus of content is %d\n", _ctrl.content));
            PAMI_ASSERT(0);
        }
        return len;
      }

      ///
      /// \brief Get data buffer, can be used to directly manipulate data from multiple windows
      ///
      INLINE char *getBuffer(T_MemoryManager *csmm) 
      {
         char *buf = NULL;
         switch (_ctrl.content)
         {
           case IMMEDIATE:
             buf = &(_data.immediate_data[0]);
             break;
           case COPYINOUT:
             if (!_buf) {
               if (NULL != csmm) {
                 _buf = (char *)csmm->getDataBuffer(1);
               }
               PAMI_ASSERT(_buf); 
             }

             buf = _buf;
             break;
#ifdef __64BIT__
           case XMEMATT:
             // PAMI_ASSERT( XMEM );
             PAMI_ASSERT( _data.xmem_data.hndl != ZCMEM_HNDL_NULL);
             _css_shmem_att_info_t  att;
             att.command  = CSS_SHMEM_ATT;
             att.hndl_att = _data.xmem_data.hndl;
             att.req_ptr  = (long long)_data.xmem_data.src;
             att.req_len  = (long long)_len;
             att.offset   = (long long) 0;
             PAMI_ASSERT(! _css_shmem_attach((zcmem_t)&att));
             PAMI_ASSERT(att.llen_avail ==att.req_len);
             buf = (char *) (att.pointer + att.latt_offset);
             break;
#endif
           default:
             PAMI_ASSERT(0);
          }
          return buf;
       }

       ///
       /// \brief Return data buffer, only really needed to detach buffer in Xmem attach case 
       ///
       INLINE void returnBuffer(char *buf) 
       {
#ifdef __64BIT__
         PAMI_ASSERT (_ctrl.content == XMEMATT);
         PAMI_ASSERT( _data.xmem_data.hndl != ZCMEM_HNDL_NULL);

         _css_shmem_reg_info_t  reg;
         reg.command  = CSS_SHMEM_REL;
         reg.hndl_in  = _data.xmem_data.hndl;
         PAMI_ASSERT(! _css_shmem_register((zcmem_t)&reg));
#endif
       }

       protected:
         window_control_t _ctrl;
         char             *_buf;
         volatile size_t  _len;
         window_data_t    _data;
    } __attribute__((__aligned__(CACHEBLOCKSZ) ));  // class CollShmWindow


    class CollShmThread : public PAMI::Device::Generic::GenericAdvanceThread 
    {
       public:

          typedef enum {
            NOACTION = 0,
            CSOSYNC,
            READFROM,
            SHAREWITH,
            EXCHANGE,
            OPDONE,
          } collshm_action_t;

          typedef unsigned collshm_mask_t;

          typedef enum {
            NOROLE = 0,
            PARENT,
            CHILD,
            BOTH
          } collshm_role_t;

          static pami_result_t advanceThread(pami_context_t context, void *thr) 
          {
             CollShmThread *t = (CollShmThread *) thr;
             return t->_advanceThread(context);
          } 

          // children in the returned array are in ascending order
          static void getchildren_knary(uint8_t rank, uint8_t k, uint8_t tasks, 
                   uint8_t *children, uint8_t *numchildren, uint8_t *parent)
          {
            int i;
            PAMI_ASSERT(k);
            if (rank * k + 1 > tasks)
              *numchildren = 0;
            else 
              *numchildren = MIN((tasks - (rank * k +1)), k);
            for (i =0 ; i< *numchildren; i++) {
              children[i]= rank*k+1+i;
            }
            *parent = (uint8_t) (rank+k-1)/k -1;
          }

          CollShmThread() : PAMI::Device::Generic::GenericAdvanceThread()
          {}

          CollShmThread(int idx, CollShmDevice *device) :
          PAMI::Device::Generic::GenericAdvanceThread(),
          _idx(idx),
          _step(0),
          _action(NOACTION),
          _partners(0),
          _sync_flag(0),
          _target_cntr(0),
          _role(NOROLE),
          _root((unsigned char) -1),
          _arank(device->getRank()),
          _nranks(device->getSize()),
          _device(device)
          { 
             setAdv(advanceThread);
          }

          ///
          /// \brief Reset some of the fields of the Threas object 
          ///
          INLINE void resetThread() 
          {
            _step        = 0;
            _action      = NOACTION;
            _partners    = 0x0;
            _sync_flag   = 0;
            _target_cntr = 0;
          }

          ///
          /// \brief check the communicaiton schedule
          ///
          INLINE void getSchedule(int *parent, int *nchildren, int **children)
          {
          //   *parent    = _parent;
          //   *nchildren = _nchildren;
          //   *children  = _children;
          }

          /// 
          /// \brief setSchedule 
          ///
          INLINE void setSchedule(int parent, int nchildren, int *children) 
          {
          //  _parent    = parent;
          //  _nchildren = nchildren;
          //  for (int i = 0; i < nchildren; ++i) 
          //     _children[i] = children[i];
          }

          ///
          /// \brief find out what action needs to be taken
          ///
          INLINE collshm_action_t getAction() { return _action; }

          ///
          /// \brief set the action to be taken next
          ///
          INLINE void setAction(collshm_action_t action) { _action = action; }

          ///
          /// \brief find out index of the thread
          ///
          INLINE unsigned int getIdx() { return _idx; }

          /// 
          /// \brief progress engine for the collective shmem device
          ///
          INLINE pami_result_t _advanceThread(pami_context_t context) 
          {

            pami_result_t rc = PAMI_SUCCESS;
            // CollShmMessage *msg = getMsg();
            BaseCollShmMessage *msg = (BaseCollShmMessage *)getMsg();
            CollShmWindow  *window;
            int pollcnt = 10;
       
            // common action handling 
            switch (_action) {
              case NOACTION: // just started 
                break;
 
              case CSOSYNC:  
                while (_partners && pollcnt--) {
                  for (unsigned partner = 0x1, i = 0; i < _nranks; ++i, partner <<= 1) {
                    if (_partners & partner) {
                      window = _device->getWindow(0, i, _idx);
                      if (window->getSyncflag() == _sync_flag) {
                        _partners ^= partner;
                        pollcnt = 10;
                      } else break;
                    }
                  }
                }
                if (_partners) return PAMI_EAGAIN;
                break;

              case SHAREWITH:
                window = _device->getWindow(0, _arank, _idx);  
                if (window->getCmpl() != 1) return PAMI_EAGAIN;
                window->setCmpl(); 
                if (window->getContent() == CollShmWindow::XMEMATT) window->returnBuffer(NULL);
                break;

              case READFROM:
                for (unsigned partner = 0; partner< _nranks; ++partner)
                {
                  if (_partners & (0x1 <<partner))
                  {
                     window = _device->getWindow(0,partner,_idx);
                    if (window->isAvail(_target_cntr) == PAMI_SUCCESS) 
                      _partners = _partners & (~(0x1 << partner)) ;
                    else  
                      return PAMI_EAGAIN;
                  }
                }
                break;

              case EXCHANGE: // not implemented yet
              default:
                PAMI_ASSERT(0);
                break;
            }

            // type specific progress handling
            if (msg) { // if there is still valid message pointer, then more progress must be needed
              switch ( msg->getMsgType() ) {
                case MultiCast:
                  rc = progressMulticast(static_cast< CollShmMessage<pami_multicast_t, CollShmDevice> *>(msg));
                  break;
                case MultiSync:
                  rc = progressMultisync(static_cast< CollShmMessage<pami_multisync_t,CollShmDevice> *>(msg));
                  break;
                case MultiCombine:
                  rc = progressMulticombine(static_cast< CollShmMessage<pami_multicombine_t, CollShmDevice> *>(msg));
                  break;
                default:
                  PAMI_ASSERT(0);
                  break;
              }
            } 

            // return thread to device
            if (rc == PAMI_SUCCESS) _device->setThreadAvail(getIdx());
            return rc;
          }

          INLINE void _setPartners() 
          {
            _partners        = 0x0;
            for (int j = 0; j < _nchildren; ++j) {
              _partners = _partners | (0x1 << ((_children[j] + _root) % _nranks ));
            }
              
          }

          INLINE void _setRole() 
          {
            if (_parent != (unsigned char)-1 && _nchildren > 0)
               _role = BOTH;
            else if (_parent != (unsigned char)-1)
               _role = CHILD;
            else if (_nchildren > 0)
               _role = PARENT;
            else
               PAMI_ASSERT(0);
          }

          ///
          /// \brief Type specific thread initialization 
          ///        Some of the initialization is probably redundant 
          ///
          INLINE void initThread( collshm_msgtype_t msgtype )
          {
             unsigned char k = _nranks-1;
             size_t root; 
             PAMI::Topology *topo;
             pami_multicast_t *mcast;
             pami_multicombine_t *mcombine;

             switch (msgtype) 
             {
               case MultiCast:
                 mcast  = (pami_multicast_t *) static_cast<CollShmMessage<pami_multicast_t, CollShmDevice> *>(_msg)->getMulti();
                 topo   = (PAMI::Topology *)mcast->src_participants;
                 root   = _device->getTopo()->rank2Index(topo->index2Rank(0));
                 _len   = mcast->bytes;
                 _wlen  = 0;
                 // Need to compare flat XMEM_ATTACH vs. pipeline tree COPYINOUT
                 // if (_len > XMEM_THRESH) k = _nranks-1; 
                 break;
               case MultiCombine: 
                 mcombine = (pami_multicombine_t *) static_cast<CollShmMessage<pami_multicombine_t, CollShmDevice> *>(_msg)->getMulti();
                 topo     = (PAMI::Topology *)mcombine->results_participants;
                 root     = _device->getTopo()->rank2Index(topo->index2Rank(0));
                 _len     = mcombine->count << pami_dt_shift[mcombine->dtype] ;
                 _wlen    = 0;
                 /// \todo need to handle (_len > XMEM_THRESH) case 
                 break;
               case MultiSync:
                 _sync_flag    = 1;
                 root          = 0;
                 break;
               default:
                 PAMI_ASSERT(0);
                 break;
             }

             _rrank = (_arank + _nranks - root) % _nranks;

             TRACE_DBG(("root = %d, _root = %d\n", (int)root, (int)_root));

             if (root != _root) 
             {
               // recalculate tree structure               
               _root = root;
               getchildren_knary(_rrank, MIN(k, _nranks-1), _nranks, &_children[0], &(_nchildren), &(_parent));
             }
             _setRole(); // _role may change even if _root does not change
          }  

          ///
          /// \brief Further progress on multisync
          ///
          INLINE pami_result_t progressMultisync( CollShmMessage<pami_multisync_t, CollShmDevice> *msg)
          {
            // pami_result_t rc;
            // PAMI_ASSERT(_sync_flag == 0);
            // if (_sync_flag != _target_cntr)  rc = _device->sync(_idx, _sync_flag);
            // return rc;
            int pollcnt = 10;
            CollShmWindow *window;

            if (_action == NOACTION) {
              _setPartners();
              _action = CSOSYNC;
              unsigned partner = 0x1;
              int i = 0;
              while (_partners && pollcnt--) {
                for (; i < _nranks; ++i, partner <<= 1) {
                  if (_partners & partner) {
                    window = _device->getWindow(0, i, _idx);
                    if (window->getSyncflag() == _sync_flag) {
                      _partners ^= partner; 
                      pollcnt = 10;
                    } else break;
                  }
                }
              }
              if (_partners) return PAMI_EAGAIN;
            }

            if (!_step) {
              mem_barrier(); // lwsync();
              window = _device->getWindow(0, _arank, _idx);
              TRACE_DBG(("window addr = %x\n", window));
              window->setSyncflag(_sync_flag);
              _step = 1;
            }

            pollcnt = 10;
            window = _device->getWindow(0, 0, _idx);
            while(window->getSyncflag() != _sync_flag && (pollcnt--));

            if (window->getSyncflag() != _sync_flag)
              return PAMI_EAGAIN;
            else {
              mem_isync(); //isync();
              msg->setStatus(PAMI::Device::Done);
              setMsg(NULL);
              _device->decActiveMsg();
              TRACE_DBG(("shm_sync, _sync_flag =%d\n", _sync_flag));
              return PAMI_SUCCESS;
            }
          }

         ///
         /// \brief Further progress on multicombine
         ///
         INLINE pami_result_t progressMulticombine( CollShmMessage<pami_multicombine_t, CollShmDevice> *msg)
         {
            pami_result_t rc = PAMI_SUCCESS;
            pami_multicombine_t *mcombine = msg->getMulti();
            CollShmWindow *window = _device->getWindow(0, _arank, _idx);
            int i, prank;

            TRACE_DBG(("shm_reduce %d\n", _idx));

            while (_len != 0) {
              if (_role == CHILD) { 
                _wlen = window->produceData(*(PAMI::PipeWorkQueue *)mcombine->data, _len, _device->getSysdep());
                PAMI_ASSERT(_wlen >= 0);
                if (_wlen == 0) return PAMI_EAGAIN;
                _len  -= _wlen;
                ++_step ;
                rc = window->setAvail(_step, 2);
                if (rc != PAMI_SUCCESS) {
                  _action = SHAREWITH;
                  break;
                }
              } else {  // PARENT or BOTH
                if (_action == READFROM) {
                  if (_wlen > 0) {  // skip reduction if result of previous round has not been consumed 
                    for (i = 0; i < _nchildren; ++i) {
                      prank = (_children[i] + _root) % _nranks;
                      CollShmWindow *pwindow = _device->getWindow(0, prank, _idx);
                      size_t len = pwindow->consumeData(window->getBuffer(NULL), _wlen, 1, mcombine->optor, mcombine->dtype);
                      PAMI_ASSERT(len == _wlen);
                      pwindow->setCmpl();
                    }
                    _len  -= _wlen;
                  }

                  if (_role == BOTH) {
                    _action = SHAREWITH;
                    rc = window->setAvail(_step, 2);
                    if (rc != PAMI_SUCCESS) break; 
                  } else { // PARENT
                    size_t len = window->consumeData(*(PAMI::PipeWorkQueue *)mcombine->results, _wlen, 0, PAMI_UNDEFINED_OP, PAMI_UNDEFINED_DT);               
                    if (len == 0) { 
                      _wlen = len;
                      return PAMI_EAGAIN;
                    }
                  }

                  if (_len == 0) break;
                }

                // produce new data
                _wlen = window->produceData(*(PAMI::PipeWorkQueue *)mcombine->data, _len, _device->getSysdep());
                PAMI_ASSERT(_wlen >= 0);
                if (_wlen == 0) return PAMI_EAGAIN;

                ++_step ;
                _action = READFROM;
                _setPartners();
                unsigned partner = 0x1;
                for (int i=0 ; i < _nranks; ++i, partner <<= 1) {
                  if (_partners & partner) {
                    CollShmWindow *pwindow = _device->getWindow(0, i, _idx);
                    if (pwindow->isAvail(_step) == PAMI_SUCCESS) {
                      _partners ^= partner;
                    } else {
                      _target_cntr = _step;
                      return PAMI_EAGAIN;
                    }
                  }
                }
              }
            }

            if (_len == 0) {
              if (rc == PAMI_SUCCESS || _wlen <= XMEM_THRESH) {
                msg->setStatus(PAMI::Device::Done);
                setMsg(NULL);
                _device->decActiveMsg();
              }
            }
            return rc;

          }

          ///
          /// \brief Further progress on multicast, with pipelining support
          ///
          INLINE pami_result_t progressMulticast( CollShmMessage<pami_multicast_t, CollShmDevice> *msg)
          {
            pami_result_t rc = PAMI_SUCCESS;
            pami_multicast_t *mcast = msg->getMulti();
            CollShmWindow *window = _device->getWindow(0, _arank, _idx);
            int prank;

            TRACE_DBG(("shm_bcast %d\n", _idx));

            while (_len != 0) {
              if (_role == PARENT) { 
                _wlen = window->produceData(*(PAMI::PipeWorkQueue *)mcast->src, _len, _device->getSysdep());
                PAMI_ASSERT(_wlen >= 0);
                _len  -= _wlen;
                ++_step ;
                rc = window->setAvail(_step, _nchildren+1);
                if (rc != PAMI_SUCCESS) {
                  _action = SHAREWITH;
                  break;
                }
              } else { // CHILD or BOTH
                prank = (_parent + _root) % _nranks;
                CollShmWindow *pwindow = _device->getWindow(0, prank, _idx);
                if (_action == READFROM) {
                  _wlen = pwindow->consumeData(*(PAMI::PipeWorkQueue *)mcast->dst, _len, 0, PAMI_UNDEFINED_OP, PAMI_UNDEFINED_DT);
                  if (_wlen == 0) return PAMI_EAGAIN;
                  pwindow->setCmpl();
                  _len  -= _wlen;

                  if (_role == BOTH) {
                    size_t len = window->produceData(*(PAMI::PipeWorkQueue *)mcast->dst, _wlen, _device->getSysdep());
                    PAMI_ASSERT(len == _wlen);
                    _action = SHAREWITH;
                    rc = window->setAvail(_step, _nchildren+1);
                    if (rc != PAMI_SUCCESS) break;
                  }
                  if (_len == 0) break;
                }

                ++_step ;
                _action = READFROM;
                if (pwindow->isAvail(_step) != PAMI_SUCCESS) {
                  _target_cntr   = _step;
                  _partners      = 1 << prank;
                  return PAMI_EAGAIN;
                }
              }
            }

            if (_len == 0) {
              if (rc == PAMI_SUCCESS || _wlen <= XMEM_THRESH) {
                msg->setStatus(PAMI::Device::Done);
                setMsg(NULL);
                _device->decActiveMsg();
              }
            }
            return rc;
          }

       protected:
          unsigned char  _idx;    
          unsigned       _step;
          collshm_action_t _action;
          collshm_mask_t _partners;
          unsigned char  _sync_flag;
          unsigned char  _target_cntr; 
          size_t         _len;
          size_t         _wlen;
          uint8_t        _root; 
          uint8_t        _rrank;               // relative on node rank
          uint8_t        _arank;               // absolute on node rank
          uint8_t        _nranks;              // total number of on node ranks for the topology
          uint8_t        _parent;              // these needs to be optimized
          uint8_t        _nchildren;
          uint8_t        _children[32];        // enough to support flat tree of 32 on node tasks
          collshm_role_t _role;
          // GenericDeviceMessageQueue *_device;
          CollShmDevice  *_device;

     }; // CollShmThread

    typedef struct collshm_wgroup_t {
       collshm_wgroup_t  *next;
       unsigned          context_id : 16;
       unsigned          num_tasks  : 8;
       unsigned          task_rank  : 8;
       T_Atomic          barrier[sizeof(int)][_numsyncs]; // only the first two rows are used
       // char           pad[CACHEBLOCKSZ-(((_numsyncs*sizeof(int)+1)*sizeof(int)+sizeof(void *))%CACHEBLOCKSZ)];
       CollShmWindow     windows[_numchannels];
    } collshm_wgroup_t __attribute__ ((__aligned__ (CACHEBLOCKSZ)));

       static pami_result_t advanceQueue(pami_context_t context, void *dev)
       {
         CollShmDevice *d = (CollShmDevice *)dev;
         return d->_advanceQueue(context);
       }

     CollShmDevice(PAMI::Device::Generic::Device *devices, unsigned gid, PAMI::Topology *topo, T_MemoryManager *csmm, void *str) :
       GenericDeviceMessageQueue(),
       _topo(topo),
       _csmm(csmm),
       _generics(devices),
       _gid(gid),
       _ntasks(topo->size()),
       _tid(topo->rank2Index(__global.mapping.task())),
       _nactive(0),
       _syncbits(0),
       _round(0),
       _head(_numchannels),
       _tail(0)
       {

         int num = _synccounts;
         while (num >>= 1) ++_syncbits;

         PAMI_ASSERT(str != NULL);
         collshm_wgroup_t *ctlstr = (collshm_wgroup_t *)str;

         for (int i = 0; i < _numchannels; ++i) {
           new (&_threads[i]) CollShmThread(i, this);
         }

         new (&_threadm) PAMI::Device::Generic::GenericAdvanceThread();
         _threadm.setStatus(PAMI::Device::Idle);
         _threadm.setFunc(advanceQueue, this);

         // initialize increments to 1 for both rounds
         for (int i = 0; i < 2; ++i)
         {
           _increments[i] = 1;
           for (int j = 0; j < _numsyncs; ++j)
             _completions[i][j] = 0;
         }

         // initialize shm channels
         for (int i = 0; i < _ntasks; ++i)
         {
            TRACE_DBG(("ctlstr is %p\n", ctlstr));
            _wgroups[i] = ctlstr;
            // ctlstr      = (collshm_wgroup_t *)(*(collshm_wgroup_t **)ctlstr);
            ctlstr      = ctlstr->next;

            if (_tid == 0)
            {
              //bzero(_wgroups[i],sizeof(collshm_wgroup_t)); // need to create window explictly ?
              //_wgroups[i]->next       = ctlstr;
              _wgroups[i]->context_id = _gid;
              _wgroups[i]->num_tasks  = _ntasks;
              _wgroups[i]->task_rank  = i;

              for (int j = 0; j < 2; ++j)
                for (int k = 0; k < _numsyncs; ++k) {
                  //_wgroups[i]->barrier[j][k] = 0;
                  _wgroups[i]->barrier[i][j].clear();
                }
             }

          }
          /// \todo there needs to be a sync
        }

        /// \brief Accessor for arrays of generic devices
        ///
        /// \return  Array of generic devices
        ///
        INLINE PAMI::Device::Generic::Device *getGenerics() {
             return _generics;
        }

        /// \brief Accessor for QS pointer to use with this object
        ///
        /// \return     Array of generic devices for client
        ///
        INLINE GenericDeviceMessageQueue * getQS() {
             return this;
        }

        ///
        /// \brief Advance the head index
        ///
        /// \return number of advances made, non-zero means there is
        ///         definitely more available channel/thread in the device
        ///
        INLINE unsigned _advanceHead()
        {
          int head  = (_head >> _syncbits) & ( _numsyncs - 1); //int head  = (_head / _synccounts) % _numsyncs;
          bool cur_round = head < (_tail >> _syncbits);        //bool     cur_round = head < (_tail / _synccounts);
          unsigned round = cur_round ? _round : ((_round+1) & 0x1); // unsigned round     = cur_round ? _round : (_round+1)% 2;
          int increment = _increments[round];
          unsigned adv       = 0;

          while (_completions[round][head] == _synccounts)
          {
            TRACE_DBG(("round = %d, head = %d, barrier = %d, completion = %d\n", round, head, _wgroups[0]->barrier[round][head].fetch(), _completions[round][head]));
            if (_wgroups[0]->barrier[round][head].fetch() == ((increment == 1) ? _ntasks : 0))
            {
              ++adv;
              _completions[round][head] = 0;
              head = (head+1) & (_numsyncs -1); // head = (head+1) % _numsyncs;
              if (head == 0) {
                _increments[round] = - _increments[round]; // flip the increment value
                round              = (round + 1) & 0x1;
                increment          = _increments[round];
                cur_round          = true;
              }
            } else {
              break;
            }
          }
          _head += (adv * _synccounts);

          TRACE_DBG(("advance head ..._head = %d\n", _head));

          return adv;
        }

        /// \brief Check if there is available Thread in the device
        ///
        INLINE bool _isThreadAvailable() { return (_tail < _head || _advanceHead()); }

        ///
        /// \brief Advance the tail index and return the next available thread
        ///
        /// \return available channel/thread index, -1 if there is no slot available
        ///
        INLINE int _advanceTail()
        {
          int position = -1;

          TRACE_DBG(("before advance tail, tail = %d\n", (int)_tail));
          if (_isThreadAvailable())
          {
            position = _tail++;
            if (_tail == _numchannels)
            {
              _tail   = 0;
              _head   -= _numchannels;
              _round  = (_round+1) & 0x1;
             }
          }
          TRACE_DBG(("after advance tail, tail =%d\n", (int)_tail));

          return position;
        }

        ///
        /// \brief monitors if the circular queue can move ahead
        ///
        INLINE pami_result_t _advanceQueue(pami_context_t context)
        {
          TRACE_DBG(("advanceQueue activated\n"));
          if (_isThreadAvailable()) {
            postNextMsg();
            _threadm.setStatus(PAMI::Device::Idle); // potential race condition here !!!
            return PAMI_SUCCESS;
          }

          return PAMI_EAGAIN;
        }

        ///
        /// \brief Get number of threads/channels of the device
        ///
        /// \return Number of threads/channels in the device
        ///
        INLINE unsigned getNumThreads() { return _numchannels(); }

        /// \brief Acessor for thread objects for this send queue
        ///        Get the next available thread from the device
        ///        The thread represents shared memory channel
        ///
        /// \return Pointer of the next available Thread
        ///         Return Null when no Thread is currently available
        ///
        INLINE CollShmThread *getAvailThread()
        {
          if (_isThreadAvailable()) {
            int ind = _advanceTail();
            _threads[ind].resetThread();
            return &_threads[ind];
            //return  (new (&_threads[ind]) CollShmThread(ind, this));
          } else {
            return NULL;
          }
        }

        ///
        /// \brief Return the channel to the device
        ///
        /// \param channel_id Index of the channel to turn
        INLINE void setThreadAvail(int channel_id)
        {

          //PAMI_ASSERT(_threads[channel_id].getStatus() == PAMI::Device::Complete);
          _threads[channel_id].setStatus(PAMI::Device::Idle);

          unsigned idx = channel_id >> _syncbits; //unsigned idx       = channel_id / _synccounts;
          bool     cur_round = channel_id < _tail;
          unsigned round     = cur_round ? _round : ((_round+1) & 0x1);

          TRACE_DBG(("Operation %d completed, thread set to free, completions = %d\n", channel_id, _completions[round][idx]));
          if ((++ _completions[round][idx]) < _synccounts) return;

          int      increment  = _increments[round];
          int      arrived;

          do {
            arrived    = _wgroups[0]->barrier[round][idx].fetch();
            // needs to recheck if it is the last one to take care of race condition
            if (arrived == (increment == 1 ? _ntasks-1 : 1)) {
              for (int grp = 0; grp < _ntasks; ++grp) {
                 for (int w = 0; w < _synccounts; ++w)  
                  (_wgroups[grp]->windows[idx*_synccounts + w]).clearCtrl();
              }
            }
          } //while(!(COLLSHM_COMPARE_AND_SWAP((atomic_p)&(_wgroups[0]->barrier[round][idx]),&arrived, arrived+increment)))
          while(!_wgroups[0]->barrier[round][idx].compare_and_swap(arrived, arrived+increment)) ;

          // try to advance head index
          _advanceHead();
        }

        ///
        /// \brief Get peer window address
        ///
        /// \param geometry Geometry ID
        /// \param peer     Rank of the peer task
        /// \param channel  Index of the channel interested in
        ///                 For source rank based channel organization
        ///                 this can be the geometry collective
        ///                 sequence number
        /// \return pointer to the corresponding window
        INLINE CollShmWindow * getWindow (unsigned geometry, size_t peer, unsigned channel)
        {
          TRACE_DBG(("_wgroup[%d] window[%d]=%p, %p\n", peer, channel, _wgroups[peer], &(_wgroups[peer]->windows[channel])));
          return &(_wgroups[peer]->windows[channel]);
        }

        INLINE T_MemoryManager *getSysdep() { return _csmm; }

        /// \brief Post message to device
        ///
        /// \param[in] msg      Message to be posted
        ///
        INLINE void postMsg(BaseCollShmMessage *msg) {
          size_t x = msg->getContextId();
          PAMI::Device::Generic::Device *g = getGenerics();

          TRACE_DBG(("postMsg %p\n",msg));

          if (_isThreadAvailable()) {
            CollShmThread *thr = getAvailThread();
            thr->setMsg(msg);
            thr->initThread(msg->getMsgType());
            thr->setStatus(PAMI::Device::Ready);
            msg->setStatus(PAMI::Device::Active);
            incActiveMsg();

            pami_result_t rc = thr->_advanceThread(g[x].getContext());
            if (msg->getStatus() == PAMI::Device::Done) {
              // setThreadAvail(thr->getIdx());
              msg->executeCallback(g[x].getContext());

              if (rc == PAMI_EAGAIN) g[x].postThread(thr);
              return;
            }

            g[x].postMsg(msg);
            g[x].postThread(thr);
            TRACE_DBG(("message posted\n"));
          } else {
            msg->setStatus(PAMI::Device::Initialized);

            if (!_nactive && _threadm.getStatus() == PAMI::Device::Idle) {
              _threadm.setStatus(PAMI::Device::Ready);
              g[x].postThread(&_threadm);
            } 
            TRACE_DBG(("%d message initalized\n", _nactive));
          }

          enqueue(msg);
          return;
        }

        ///
        /// \brief Scan the posted message queue and start as many as possible
        ///
        INLINE pami_context_t postNextMsg() {
          BaseCollShmMessage *msg, *nextmsg;

          for (msg = (BaseCollShmMessage *)peek(); msg; msg = nextmsg) {
             TRACE_DBG(("postNextMsg %p\n",msg));

             nextmsg = (BaseCollShmMessage *)next(msg);
             if (msg->getStatus() != PAMI::Device::Initialized) continue;

             if (_isThreadAvailable()) {
               size_t x = msg->getContextId();
               PAMI::Device::Generic::Device *g = getGenerics();
               msg->setStatus(PAMI::Device::Active);
               incActiveMsg();

               CollShmThread *thr = getAvailThread();
               thr->setMsg(msg);
               thr->initThread(msg->getMsgType());
               thr->setStatus(PAMI::Device::Ready);
               pami_result_t rc = thr->_advanceThread(g[x].getContext());

               if (msg->getStatus() == PAMI::Device::Done) {
                 // setThreadAvail(thr->getIdx());
                 deleteElem(msg);
                 msg->executeCallback(g[x].getContext());

                 if (rc == PAMI_EAGAIN) g[x].postThread(thr);
                 continue;
               }

               g[x].postMsg(msg);
               g[x].postThread(thr);
             } else {
               break;
             }
          }

          return NULL;
        }

        INLINE void incActiveMsg() { ++_nactive; }
        INLINE void decActiveMsg() { --_nactive; }
 
        INLINE size_t getRank() { return _tid; }
        INLINE size_t getSize() { return _ntasks; }
        INLINE PAMI::Topology *getTopo() {return _topo;} 

protected:
        PAMI::Topology                *_topo;
        T_MemoryManager               *_csmm;   ///< collective shmem memory manager
        PAMI::Device::Generic::Device *_generics; ///< generic device arrays
        CollShmThread _threads[_numchannels];   ///< Threads for active posted messages on device
        PAMI::Device::Generic::GenericAdvanceThread 
                         _threadm; // this thread is not associated with shared memory channel
                                   // it is a pure work item making sure enqueued message gets
                                   // a chance to run

        unsigned         _gid;     // id for the geometry
        unsigned         _ntasks;  // number of tasks sharing the device
        unsigned         _tid;     // task idndex in the device
        unsigned         _nactive; // number of posted messages in pending queue

        unsigned  char   _syncbits;
        unsigned  char   _head;    // logical index to the next unavailable channel
        unsigned  char   _tail;    // index of the oldest in-use channel
        unsigned  char   _round;   // wrap around count
        unsigned  char   _completions[2][_numsyncs]; // counters for local completions
        int              _increments [2];             // increment values for updating
                                                      // counters of local completions
        collshm_wgroup_t *_wgroups[PAMI_MAX_PROC_PER_NODE]; // pointer to the shm channels
}; // class CollShmDevice

//typedef PAMI::Device::Generic::NillSubDevice NillCollShmDevice;
typedef PAMI::Device::Generic::Device NillCollShmDevice;

template <class T_CollShmDevice, class T_MemoryManager>
class CollShmModel : 
   public PAMI::Device::Interface::MulticastModel<CollShmModel<T_CollShmDevice, T_MemoryManager>,NillCollShmDevice,sizeof(CollShmMessage<pami_multicast_t, T_CollShmDevice>)>, 
   public PAMI::Device::Interface::MultisyncModel<CollShmModel<T_CollShmDevice, T_MemoryManager>,NillCollShmDevice,sizeof(CollShmMessage<pami_multisync_t, T_CollShmDevice>)>, 
   public PAMI::Device::Interface::MulticombineModel<CollShmModel<T_CollShmDevice, T_MemoryManager>,NillCollShmDevice,sizeof(CollShmMessage<pami_multicombine_t, T_CollShmDevice>)> 
{
public:
        static const size_t sizeof_msg              = sizeof(CollShmMessage<collshm_multi_t, T_CollShmDevice>);
        static const size_t sizeof_multicast_msg    = sizeof(CollShmMessage<pami_multicast_t, T_CollShmDevice>);
        static const size_t sizeof_multisync_msg    = sizeof(CollShmMessage<pami_multisync_t, T_CollShmDevice>);
        static const size_t sizeof_multicombine_msg = sizeof(CollShmMessage<pami_multicombine_t, T_CollShmDevice>);

        CollShmModel(PAMI::Device::Generic::Device *device, unsigned commid, PAMI::Topology *topology, T_MemoryManager *csmm) :
        PAMI::Device::Interface::MulticastModel<CollShmModel,NillCollShmDevice,sizeof_msg>(*device, _status),
        PAMI::Device::Interface::MultisyncModel<CollShmModel,NillCollShmDevice,sizeof_msg>(*device, _status),
        PAMI::Device::Interface::MulticombineModel<CollShmModel,NillCollShmDevice,sizeof_msg>(*device, _status),
        _peer(topology->rank2Index(__global.mapping.task())),
        _npeers(topology->size()),
        _csdevice(device, commid, topology, csmm, csmm->getWGCtrlStr())
        {
            //TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
            // assert(device == _g_l_bcastwq_dev);

            _csdevice.getWindow(0,0,0); // just simple checking
        }

        INLINE pami_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                               pami_multicast_t *mcast);
        INLINE pami_result_t postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                               pami_multisync_t *msync);
        INLINE pami_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg],
                                               pami_multicombine_t *mcombine); 
private:
        unsigned _peer;
        unsigned _npeers;

        pami_result_t           _status;

        T_CollShmDevice         _csdevice;  

}; // class CollShmModel

template <class T_CollShmDevice, class T_MemoryManager>
INLINE pami_result_t CollShmModel<T_CollShmDevice, T_MemoryManager> ::postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                                  pami_multicast_t *mcast) {
        // PAMI::Topology *src_topo = (PAMI::Topology *)mcast->src_participants;
        // unsigned rootpeer = __global.topology_local.rank2Index(src_topo->index2Rank(0));
        CollShmMessage<pami_multicast_t, T_CollShmDevice> *msg =
          new (&state) CollShmMessage<pami_multicast_t, T_CollShmDevice> (&_csdevice, mcast);
        _csdevice.postMsg(msg);
        return PAMI_SUCCESS;
}

template <class T_CollShmDevice, class T_MemoryManager>
INLINE pami_result_t CollShmModel<T_CollShmDevice, T_MemoryManager>::postMultisync_impl(uint8_t (&state)[sizeof_msg],
                                                  pami_multisync_t *msync) {

        CollShmMessage<pami_multisync_t, T_CollShmDevice> *msg =
          new (&state) CollShmMessage<pami_multisync_t, T_CollShmDevice> (&_csdevice, msync);
        _csdevice.postMsg(msg);
        return PAMI_SUCCESS;
}

template <class T_CollShmDevice, class T_MemoryManager>
INLINE pami_result_t CollShmModel<T_CollShmDevice, T_MemoryManager>::postMulticombine_impl(uint8_t (&state)[sizeof_msg],
                                                  pami_multicombine_t *mcombine) {
        // PAMI::Topology *src_topo = (PAMI::Topology *)mcombine->data_participants;
        // unsigned rootpeer = __global.topology_local.rank2Index(src_topo->index2Rank(0));
        CollShmMessage<pami_multicombine_t, T_CollShmDevice> *msg =
          new (&state) CollShmMessage<pami_multicombine_t, T_CollShmDevice> (&_csdevice,mcombine);
        _csdevice.postMsg(msg);
        return PAMI_SUCCESS;
}


}; // namespace CollShm
}; // namespace Device
}; // namespace PAMI

#undef PAMI_ASSERT
#undef TRACE_ERR
#undef TRACE_DBG

#endif // __pami_components_devices_cshmem_CollShmDevice_h__