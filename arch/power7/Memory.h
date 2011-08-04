/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/power7/Memory.h
 * \brief Defines power7 memory specializations
 */

#ifndef __arch_power7_Memory_h__
#define __arch_power7_Memory_h__

#define mem_barrier() { asm volatile ("lwsync"); }

#include "arch/MemoryInterface.h"

#undef  mem_barrier

namespace PAMI
{
  namespace Memory
  {
    template <> const bool supports <instruction> () { return true; };

    template <> void sync <instruction> () { asm volatile ("isync"); };
  };
};

#endif // __arch_power7_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//