/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/cdi/shmem/dma/bgp/cnk/ShmemDmaModelBgpCnk.h
 * \brief ???
 */

#ifndef __dcmf_cdi_shmem_dma_bgp_cnk_model_h__
#define __dcmf_cdi_shmem_dma_bgp_cnk_model_h__

#include "dcmf.h"
#include "SysDep.h"
#include "../../../../DmaModel.h"
#include "../../../../bgp/cnk/DmaMemregionBgpCnk.h"
#include "errno.h"

#define MAX_BGP_PROCS 4

#ifndef TRACE
#define TRACE(x)
#endif
#define WEAK_CONSISTENCY
namespace DCMF
{
  namespace CDI
  {
    ///
    /// \brief Common Device Interface for shared memory dma models.
    ///
    /// The CDI shared memory dma model contains the shared memory
    /// packet header which is block-copied into a shared memory message
    /// when it is generated.
    ///
    /// \see ShmemPacket
    ///
    template <class T_Device, class T_Memregion, class T_Object>
    class ShmemDmaModelBgpCnk : public DMA::Model< ShmemDmaModelBgpCnk<T_Device, T_Memregion, T_Object>, T_Device, T_Memregion, T_Object >
    {
      public:

        typedef struct dma_metadata
        {
          void  * paddr;
          void  * vaddr;
          size_t  bytes;
          uint8_t peer;
        } dma_metadata_t;

        ///
        /// \brief Construct a Common Device Interface shared memory dma model.
        ///
        /// \param[in] device  CDI shared memory device
        ///
        ShmemDmaModelBgpCnk (T_Device & device) :
            DMA::Model< ShmemDmaModelBgpCnk<T_Device, T_Memregion, T_Object>, T_Device, T_Memregion, T_Object > (device),
            _device (device)
        {
        };

        bool init_impl (size_t origin_rank)
        {
          if (!_device.isDmaAvailable()) return false;

          // register a "put" dispatch function....
          _dispatch_id = _device.registerRecvFunction (dispatch_put_packet, this);

          return true;
        }

        inline bool postDmaPut_impl (T_Object        * obj,
                                     DCMF_Callback_t & cb,
                                     size_t            target_rank,
                                     T_Memregion     * local_memregion,
                                     size_t            local_offset,
                                     T_Memregion     * remote_memregion,
                                     size_t            remote_offset,
                                     size_t            bytes)
        {
          size_t global, peer;
          _device.getMapping()->rank2node (target_rank, global, peer);

          dma_metadata_t metadata;
          metadata.paddr = (void *) (local_memregion->getBasePhysicalAddress() + local_offset);
          metadata.vaddr = (void *) ((uint8_t *)remote_memregion->getBaseVirtualAddress() + remote_offset);
          metadata.bytes = bytes;
          metadata.peer  = peer;

#if 0

          if (_device.isSendQueueEmpty (peer) &&
              _device.writeSinglePacket (peer, _dispatch_id, metadata,
                                         payload, bytes) == DCMF_SUCCESS)
            {
              if (cb.function) cb.function (cb.clientdata, NULL);

              return true;
            }

#endif
          new (obj) T_Object (cb, _dispatch_id, (void *)&metadata, sizeof(dma_metadata_t));
          obj->enableRemoteCompletion ();
          _device.post (peer, obj);

          return false;
        }

        inline bool postDmaGet_impl (T_Object        * obj,
                                     DCMF_Callback_t & cb,
                                     size_t            target_rank,
                                     T_Memregion     * local_memregion,
                                     size_t            local_offset,
                                     T_Memregion     * remote_memregion,
                                     size_t            remote_offset,
                                     size_t            bytes)
        {
          size_t global, peer;
          _device.getMapping()->rank2node (target_rank, global, peer);

          void * remote_paddr = (void *) (remote_memregion->getBasePhysicalAddress() + local_offset);
          void * local_vaddr  = (void *) ((uint8_t *)local_memregion->getBaseVirtualAddress() + remote_offset);
          int rc = _device.mmap_copy (peer, local_vaddr, remote_paddr, bytes);

          DCMF_assert_debug(rc == 0);

          if (cb.function != NULL) cb.function(cb.clientdata, NULL);

          return true;
        }

      protected:
        T_Device & _device;
        size_t     _dispatch_id;

      private:
        static int dispatch_put_packet (int      channel,
                                        void   * metadata,
                                        void   * payload,
                                        size_t   bytes,
                                        void   * recv_func_parm)
        {
          dma_metadata_t * m = (dma_metadata_t *) metadata;
          ShmemDmaModelBgpCnk<T_Device, T_Memregion, T_Object> * model =
            (ShmemDmaModelBgpCnk<T_Device, T_Memregion, T_Object> *) recv_func_parm;

          model->_device.mmap_copy (m->peer, m->vaddr, m->paddr, m->bytes);

          return 0;
        }
    };
  };
};
#undef TRACE
#endif /* __dcmf_cdi_shmem_dma_bgp_cnk_model_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
