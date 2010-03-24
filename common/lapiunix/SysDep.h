/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/SysDep.h
 * \brief ???
 */

#ifndef __common_lapiunix_SysDep_h__
#define __common_lapiunix_SysDep_h__

#undef __bgp__
#undef __bgq__
#define PAMI_SYSDEP_CLASS PAMI::SysDep


#include "common/SysDepInterface.h"
#include "Platform.h"
#include "Mapping.h"
#include "Wtime.h"
#include "Topology.h"

namespace PAMI
{
  typedef Interface::SysDep SysDep;
};
#endif // __components_sysdep_lapi_lapisysdep_h__
