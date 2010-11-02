/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/mu2/messagerate.cc
 * \brief Simple standalone MU software device test.
 */

#include "common/bgq/Global.h"

#include "common/bgq/BgqPersonality.h"
#include "common/bgq/Mapping.h"
#include "common/bgq/BgqMapCache.h"

#include "components/devices/bgq/mu2/global/Global.h"
#include "components/devices/bgq/mu2/global/ResourceManager.h"

#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/DmaModel.h"
#include "components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h"
#include "components/devices/bgq/mu2/model/PacketModel.h"
//#include "components/devices/bgq/mu2/model/PacketModelMemoryFifoCompletion.h"
#include "components/devices/bgq/mu2/model/PacketModelDeposit.h"
#include "components/devices/bgq/mu2/model/PacketModelInterrupt.h"

#include "p2p/protocols/send/eager/Eager.h"

typedef PAMI::Device::MU::Context MuContext;

typedef PAMI::Device::MU::PacketModel MuPacketModel;
//typedef PAMI::Device::MU::PacketModelMemoryFifoCompletion MuPacketModel;
//typedef PAMI::Device::MU::PacketModelDeposit MuPacketModel;
//typedef PAMI::Device::MU::PacketModelInterrupt MuPacketModel;

//typedef PAMI::Device::MU::DmaModel MuDmaModel;
typedef PAMI::Device::MU::DmaModelMemoryFifoCompletion MuDmaModel;

typedef PAMI::Protocol::Send::Eager<MuPacketModel, MuContext> MuEager;

#define MAX_ITER 10
int npackets = 0;

void done_fn       (pami_context_t   context,
                    void           * cookie,
                    pami_result_t    result)
{
  //fprintf (stderr, "done_fn() %zu -> %zu\n", *((size_t *)cookie), *((size_t *)cookie) - 1);
  (*((size_t *)cookie))--;
}

int dispatch_fn    (void   * metadata,
                    void   * payload,
                    size_t   bytes,
                    void   * recv_func_parm,
                    void   * cookie)
{
  fprintf(stderr, "Received packet: recv_func_parm = %zu (%p), MAX_ITER = %d, npackets = %d -> %d\n", (size_t) recv_func_parm, recv_func_parm, MAX_ITER, npackets, npackets+1);
  npackets ++;
  return 0;
}

void recv (
  pami_context_t       context,      /**< IN:  communication context which invoked the dispatch function */
  void               * cookie,       /**< IN:  dispatch cookie */
  const void         * header_addr,  /**< IN:  header address  */
  size_t               header_size,  /**< IN:  header size     */
  const void         * pipe_addr,    /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
  size_t               data_size,    /**< IN:  number of byts of message data, valid regardless of message type */
  pami_endpoint_t      origin,
  pami_recv_t        * recv)        /**< OUT: receive message structure, only needed if addr is non-NULL */
{
  npackets ++;
}

#define MAX_BUF_SIZE  1024
#define MSG_SIZE      1

// PAMI::Global __myGlobal;
// PAMI::ResourceManager __pamiRM;
// PAMI::Device::MU::Global __MUGlobal ( __pamiRM, __myGlobal.mapping, __myGlobal.personality );

template <typename T_Model, typename T_Protocol>
void test (MuContext & mu0, MuContext & mu1, T_Model & model, T_Protocol & protocol, const char * label = "")
{
  char metadata[4];
  char buf[MAX_BUF_SIZE];

  memset (metadata, 0, sizeof(metadata));
  memset (buf, 0, sizeof(buf));

  struct iovec iov[1];
  iov[0].iov_base = buf;
  iov[0].iov_len  = MSG_SIZE;

  volatile size_t active = 0;
  npackets = 0;

  unsigned long start = 0, end = 0;

  // -------------------------------------------------------------------
  // Test immediate packet model
  // -------------------------------------------------------------------
  for (int i = 0; i <= MAX_ITER; i++)
    {
      if (i == 1)
        start = GetTimeBase();

      model.postPacket (__global.mapping.task(),
                        1,
                        (void *)metadata,
                        4,
                        iov);
    }

  while (npackets != MAX_ITER+1)
  {
    mu0.advance();
    mu1.advance();
  }

  end = GetTimeBase();

  printf ("[%s] immediate pingpong time = %d cycles\n", label, (int)((end - start) / MAX_ITER));

  // -------------------------------------------------------------------
  // Test non-blocking packet model
  // -------------------------------------------------------------------
  typedef uint8_t state_t[MuPacketModel::packet_model_state_bytes];
  state_t state[MAX_ITER];
  npackets = 0;
  start = 0;
  end = 0;

  for (int i = 0; i <= MAX_ITER; i++)
    {
      if (i == 1)
        start = GetTimeBase();

      model.postPacket (state[i],
                        done_fn,
                        (void *)&active,
                        __global.mapping.task(),
                        1,
                        (void *)metadata,
                        4,
                        iov);
    }


  while (npackets != MAX_ITER+1)
  {
    mu0.advance();
    mu1.advance();
  }

  end = GetTimeBase();

  printf ("[%s] nonblocking pingpong time = %d cycles\n", label, (int)((end - start) / MAX_ITER));

  // -------------------------------------------------------------------
  // Test non-blocking send protocol
  // -------------------------------------------------------------------
#if 0
  npackets = 0;
  start = 0;
  end = 0;

  for (int i = 0; i <= MAX_ITER; i++)
    {
      if (i == 1)
        start = GetTimeBase();

      protocol.simple ();
      pkt.postPacket (state[i],
                      done_fn,
                      (void *)&active,
                      __global.mapping.task(),
                      0,
                      (void *)metadata,
                      4,
                      iov);
    }

  while (npackets != MAX_ITER+1) mu.advance();

  end = GetTimeBase();

  printf ("[%s] nonblocking pingpong time = %d cycles\n", label, (int)((end - start) / MAX_ITER));
#endif
}

int main(int argc, char ** argv)
{
  // Initialize the MU resources for all contexts for this client
  __MUGlobal.getMuRM().initializeContexts( 0 /*id_client*/, 2 /*id_count*/, NULL /* Generic Device */ );

  MuContext mu0 (__global.mapping, 0, 0, 2);
  mu0.init (0, NULL, NULL); // id_client, mu context "cookie" (usually pami_context_t)

  MuContext mu1 (__global.mapping, 0, 1, 2);
  mu1.init (0, NULL, NULL); // id_client, mu context "cookie" (usually pami_context_t)

  fprintf (stderr, "After mu init\n");

//  uint8_t model00_buf[sizeof(PAMI::Device::MU::PacketModelMemoryFifoCompletion)] __attribute__((__aligned__(32)));
//  PAMI::Device::MU::PacketModelMemoryFifoCompletion &model00 = *(new (model00_buf) PAMI::Device::MU::PacketModelMemoryFifoCompletion(mu0));

//  uint8_t model01_buf[sizeof(PAMI::Device::MU::PacketModelMemoryFifoCompletion)] __attribute__((__aligned__(32)));
//  PAMI::Device::MU::PacketModelMemoryFifoCompletion &model01 = *(new (model01_buf) PAMI::Device::MU::PacketModelMemoryFifoCompletion(mu1));

  uint8_t model10_buf[sizeof(PAMI::Device::MU::PacketModel)] __attribute__((__aligned__(32)));
  PAMI::Device::MU::PacketModel &model10 = *(new (model10_buf) PAMI::Device::MU::PacketModel(mu0));

  uint8_t model11_buf[sizeof(PAMI::Device::MU::PacketModel)] __attribute__((__aligned__(32)));
  PAMI::Device::MU::PacketModel &model11 = *(new (model11_buf) PAMI::Device::MU::PacketModel(mu1));

  fprintf (stderr, "After model constructors\n");

  //pami_result_t result;
  //MuDmaModel dma (mu, result);

//  model00.init (100, dispatch_fn, (void *) 100, NULL, NULL);
//  model01.init (100, dispatch_fn, (void *) 101, NULL, NULL);

  model10.init (101, dispatch_fn, (void *) 100, NULL, NULL);
  model11.init (101, dispatch_fn, (void *) 101, NULL, NULL);

  fprintf (stderr, "After model init\n");


  pami_result_t result;
  MuEager eager (10,      // dispatch set id
                 recv,    //dispatch function
                 NULL,    // dispatch cookie
                 mu0,      // "packet" device reference
                 (pami_endpoint_t) 0,       // origin endpoint
                 (pami_context_t) NULL,
                 result);

  fprintf (stderr, "After eager constructor\n");


//  test (mu0, mu1, model00, eager, "memory fifo completion");
  test (mu0, mu1, model10, eager, "completion array");

  return 0;
}
