///
/// \file components/devices/bgq/mu2/model/CollectiveMulticombineDmaModel.h
/// \brief ???
///
#ifndef __components_devices_bgq_mu2_model_CollectiveMulticombineDmaModel_h__
#define __components_devices_bgq_mu2_model_CollectiveMulticombineDmaModel_h__

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/CollectiveDmaModelBase.h"
#include "components/devices/bgq/mu2/model/MU_Collective_OP_DT_Table.h"
#include "sys/pami.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      static const size_t mcomb_state_bytes = 0;
      class CollectiveMulticombineDmaModel: public CollectiveDmaModelBase,
          public Interface::MulticombineModel < CollectiveMulticombineDmaModel, MU::Context, mcomb_state_bytes >
      {
        public:
          static const size_t sizeof_msg = mcomb_state_bytes;

          CollectiveMulticombineDmaModel (pami_client_t    client,
                                          pami_context_t   context,
                                          MU::Context                 & device,
                                          pami_result_t               & status) :
              CollectiveDmaModelBase(device, status),
              Interface::MulticombineModel<CollectiveMulticombineDmaModel, MU::Context, mcomb_state_bytes>  (device, status)
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          }

          pami_result_t postMulticombineImmediate_impl(size_t                   client,
                                                       size_t                   context,
                                                       pami_multicombine_t    * mcombine,
                                                       void                   * devinfo = NULL)
          {
            unsigned sizeoftype =  mu_collective_size_table[mcombine->dtype];
            unsigned bytes      =  mcombine->count * sizeoftype;
            unsigned op = mu_collective_op_table[mcombine->dtype][mcombine->optor];
            TRACE_FN_ENTER();

            if (op == unsupported_operation)
              return PAMI_ERROR; //Unsupported operation

            unsigned classroute = 0;

            if (devinfo)
              classroute = ((uint32_t)(uint64_t)devinfo) - 1;

            PipeWorkQueue *spwq = (PipeWorkQueue *) mcombine->data;
            PipeWorkQueue *dpwq = (PipeWorkQueue *) mcombine->results;
            char *src = spwq->bufferToConsume();
            uint32_t sbytes = spwq->bytesAvailableToConsume();

            pami_result_t rc = PAMI_ERROR;

            if ( likely(bytes <= CollectiveDmaModelBase::_collstate._tempSize &&
                        sbytes == bytes) )
              {
                rc = CollectiveDmaModelBase::postShortCollective (op,
                                                                  sizeoftype,
                                                                  bytes,
                                                                  src,
                                                                  dpwq,
                                                                  mcombine->cb_done.function,
                                                                  mcombine->cb_done.clientdata,
                                                                  classroute);
              }

            TRACE_FN_EXIT();
            if (rc == PAMI_SUCCESS)
              return rc;

            return CollectiveDmaModelBase::postCollective (bytes,
                                                           spwq,
                                                           dpwq,
                                                           mcombine->cb_done.function,
                                                           mcombine->cb_done.clientdata,
                                                           op,
                                                           sizeoftype,
                                                           classroute);
          }


          /// \see PAMI::Device::Interface::MulticombineModel::postMulticombine
          pami_result_t postMulticombine_impl(uint8_t (&state)[mcomb_state_bytes],
                                              size_t               client,
                                              size_t               context,
                                              pami_multicombine_t *mcombine,
                                              void                *devinfo = NULL)
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
            // Get the source data buffer/length and validate (assert) inputs
            return PAMI_ERROR;
          }
      };
    };
  };
};
#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG


#endif
