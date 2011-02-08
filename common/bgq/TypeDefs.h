/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgq/TypeDefs.h
 * \brief ???
 */

#ifndef __common_bgq_TypeDefs_h__
#define __common_bgq_TypeDefs_h__

#include "util/ccmi_debug.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/devices/shmem/ShmemDmaModel.h"
#include "components/devices/shmem/shaddr/BgqShaddrReadOnly.h"
#include "components/devices/shmem/shaddr/BgqShaddrPacketModel.h"
#include "components/devices/shmem/wakeup/WakeupBGQ.h"
#include "components/devices/shmem/wakeup/WakeupSemaphore.h"
#include "components/fifo/FifoPacket.h"
#include "components/fifo/linear/LinearFifo.h"
#include "components/fifo/wrap/WrapFifo.h"

#if 0
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#endif

#include "components/devices/bgq/mu2/Factory.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/bgq/mu2/model/PacketModel.h"
#include "components/devices/bgq/mu2/model/DmaModel.h"
#include "components/devices/bgq/mu2/model/DmaModelMemoryFifoCompletion.h"

#include "components/devices/bgq/mu2/model/AxialMulticast.h"
#include "components/devices/bgq/mu2/model/MulticastDmaModel.h"
#include "components/devices/bgq/mu2/model/Multicast.h"
#include "components/devices/bgq/mu2/model/Multicombine.h"
#include "components/devices/bgq/mu2/model/Multisync.h"
#include "components/devices/bgq/mu2/model/AllreducePacketModel.h"

//  #include "components/devices/bgq/mu/MUCollDevice.h"
//  #include "components/devices/bgq/mu/MUMulticastModel.h"
//  #include "components/devices/bgq/mu/MUMultisyncModel.h"
//  #include "components/devices/bgq/mu/MUMulticombineModel.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"
#include "p2p/protocols/rget/GetRdma.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/SendPWQ.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/bgq/L2CounterBounded.h"
#include "components/atomic/native/NativeCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/atomic/indirect/IndirectBarrier.h"
#include "components/atomic/indirect/IndirectCounter.h"
#include "components/devices/misc/AtomicBarrierMsg.h"

#include "common/NativeInterface.h"
#include "common/bgq/NativeInterface.h"

#include "algorithms/geometry/Geometry.h"

#define PAMI_GEOMETRY_CLASS    PAMI::Geometry::Common

#include "algorithms/geometry/PGASCollRegistration.h"

#include "components/devices/shmem/mdls/ShmemMcombModelWorld.h"
#include "components/devices/shmem/mdls/ShmemMcstModelWorld.h"
//#include "components/devices/shmem/ShmemCollDesc.h"
#include "components/devices/shmem/mdls/ShmemColorMcstModel.h"

#include "components/devices/bgq/mu2/model/CollectiveMulticastDmaModel.h"
#include "components/devices/bgq/mu2/model/CollectiveMulticombineDmaModel.h"
#include "components/devices/bgq/mu2/model/CollectiveMcomb2Device.h"
#include "components/devices/bgq/mu2/model/AMMulticastModel.h"
#include "components/devices/bgq/mu2/model/MUMultisync.h"


namespace PAMI
{
  typedef Geometry::Common                     BGQGeometry;

  typedef MemoryAllocator<2048, 64, 16> ProtocolAllocator; /// \todo How much do we really need?  Is there a better way?

  typedef Device::MU::Context MUDevice;

  typedef BGQNativeInterface < MUDevice,
  Device::MU::MulticastModel<true>, // all-sided
  Device::MU::MultisyncModel<false, false>,
  Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUGlobalNI;

  typedef BGQNativeInterface < MUDevice,
  Device::MU::AxialMulticastModel<false, false>,
  Device::MU::MultisyncModel<false, false>,
  Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialNI;

  /*  typedef BGQNativeInterface < MUDevice,
                                 Device::MU::AxialMulticastModel<false, false>,
                                 Device::MU::MultisyncModel<false, false>,
                                 Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialDputNI;
  */
  typedef BGQNativeInterfaceAS < MUDevice,
  Device::MU::MulticastDmaModel,
  Device::MU::MultisyncModel<false, false>,
  Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false> > MUAxialDputNI;

  typedef BGQNativeInterfaceAS < MUDevice,
  Device::MU::CollectiveMulticastDmaModel,
  Device::MU::MUMultisyncModel,
  Device::MU::CollectiveMulticombineDmaModel
  //Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false>
  > MUGlobalDputNI;

/*  typedef BGQNativeInterfaceAS < MUDevice,
  Device::MU::CollectiveMulticastDmaModel,
  Device::MU::MUMultisyncModel,
  Device::MU::CollectiveMcomb2Device
  > MUGlobalDputNI;*/

  typedef BGQNativeInterfaceAS < MUDevice,
  Device::MU::CollectiveMulticastDmaModel,
  Device::MU::MUMultisyncModel,
  Device::MU::CollectiveMcomb2Device
  > MUShmemGlobalDputNI;
  
  //typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Counter::Native> ShmemCollDesc;
  //typedef PAMI::Device::Shmem::ShmemCollDesc <PAMI::Counter::BGQ::IndirectL2> ShmemCollDesc;

  typedef BGQNativeInterface < MUDevice,
    Device::MU::ShortAMMulticastModel,
    Device::MU::MultisyncModel<false, false>,
    Device::MU::MulticombineModel<Device::MU::AllreducePacketModel, false, false>
    > MUAMMulticastNI;

  typedef Fifo::FifoPacket <32, 160> ShmemPacket;
  //typedef Fifo::LinearFifo<ShmemPacket, PAMI::Counter::BGQ::IndirectL2> ShmemFifo;
  //typedef Fifo::LinearFifo<ShmemPacket, PAMI::Counter::BGQ::IndirectL2, 128, Wakeup::BGQ> ShmemFifo;
  typedef Fifo::WrapFifo<ShmemPacket, PAMI::Counter::BGQ::IndirectL2Bounded, 128, Wakeup::BGQ> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo, Device::Shmem::BgqShaddrReadOnly> ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice> ShmemPacketModel;
  //typedef Device::Shmem::BgqShaddrPacketModel<ShmemDevice> ShmemPacketModel;
  typedef Device::Shmem::DmaModel<ShmemDevice> ShmemDmaModel;

  typedef Protocol::Send::Eager <ShmemPacketModel, ShmemDevice> ShmemEagerBase;
  typedef PAMI::Protocol::Send::SendPWQ < ShmemEagerBase > ShmemEager;

  typedef PAMI::Device::Shmem::ShmemColorMcstModel<ShmemDevice> ShaddrMcstModel;
  typedef PAMI::Device::Shmem::ShmemCollDescFifo  <PAMI::Counter::BGQ::IndirectL2> ShmemColorDesc;

  typedef  PAMI::BGQNativeInterfaceASMultiDevice < MUDevice,
  ShmemDevice,
  Device::MU::MulticastDmaModel,
  ShaddrMcstModel,
  Device::MU::MultisyncModel<false, false>,
  Device::MU::MulticombineModel<PAMI::Device::MU::AllreducePacketModel, false, false> > MUShmemAxialDputNI;


  // shmem active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage<ShmemEager> ShmemNI_AM;
  // shmem allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided<ShmemEager> ShmemNI_AS;

  typedef Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, true> MUEagerBase;
  typedef PAMI::Protocol::Send::SendPWQ < MUEagerBase > MUEager;

  // MU active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage<MUEager> MUNI_AM;
  // MU allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided<MUEager> MUNI_AS;

  // shmem + MU composite active message over p2p eager
  typedef PAMI::NativeInterfaceActiveMessage< PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send> > CompositeNI_AM;
  // shmem + MU composite allsided over p2p eager
  typedef PAMI::NativeInterfaceAllsided< PAMI::Protocol::Send::SendPWQ< Protocol::Send::Send> > CompositeNI_AS;

  // PGAS over MU
  typedef TSPColl::NBCollManager<CompositeNI_AM> MU_NBCollManager;
  typedef CollRegistration::PGASRegistration < BGQGeometry,
  CompositeNI_AM,
  ProtocolAllocator,
  MUEager,
  ShmemEager,
  MUDevice,
  ShmemDevice,
  MU_NBCollManager > MU_PGASCollreg;
  // PGAS over Shmem
  typedef TSPColl::NBCollManager<CompositeNI_AM> Shmem_NBCollManager;
  typedef CollRegistration::PGASRegistration < BGQGeometry,
  CompositeNI_AM,
  ProtocolAllocator,
  ShmemEager,
  ShmemEager,
  ShmemDevice,
  ShmemDevice,
  Shmem_NBCollManager > Shmem_PGASCollreg;


  typedef PAMI::Barrier::IndirectCounter<PAMI::Counter::BGQ::IndirectL2> Barrier_Type;
  //typedef PAMI::Barrier::Indirect<PAMI::Barrier::Counter<PAMI::Counter::BGQ::L2> > Barrier_Type;

  typedef PAMI::Device::AtomicBarrierMdl<Barrier_Type>                           ShmemMsyncModel;

  //typedef PAMI::Device::Shmem::ShmemCollDesc <Counter::Indirect<Counter::Native> > ShmemCollDesc;
  //typedef PAMI::Device::ShmemCollDevice<Counter::Indirect<Counter::Native> > ShmemCollDevice;
//  typedef PAMI::Device::ShmemCollDevice<PAMI::Counter::BGQ::IndirectL2> ShmemCollDevice;
  typedef PAMI::Device::Shmem::ShmemMcombModelWorld <ShmemDevice> ShmemMcombModel;
  typedef PAMI::Device::Shmem::ShmemMcstModelWorld <ShmemDevice> ShmemMcstModel;
  typedef PAMI::Device::Shmem::ShortMcombMessage <ShmemDevice> ShmemMcombMessage;
#if 0
  typedef Device::LocalAllreduceWQModel ShmemMcombModel;
  typedef Device::LocalBcastWQModel ShmemMcstModel;
  typedef ShmemDevice ShmemCollDevice;
#endif

  typedef BGQNativeInterfaceAS <ShmemDevice, ShmemMcstModel, ShmemMsyncModel, ShmemMcombModel> AllSidedShmemNI;

}

//#define PAMI_COLL_MCAST_CLASS
//#define PAMI_COLL_M2M_CLASS
//#define PAMI_NATIVEINTERFACE
//#define PAMI_GEOMETRY_CLASS    PAMI::BGQGeometry

#endif
