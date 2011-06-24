/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/MemoryInterface.h
 * \brief Defines arch memory interface
 */

#ifndef __arch_MemoryInterface_h__
#define __arch_MemoryInterface_h__

#ifndef mem_barrier
#error "mem_barrier() was not defined by the architecture"
#endif

#include "Arch.h"

namespace PAMI
{
  namespace Memory
  {
    typedef enum
    {
      full_sync = 0,
      remote_msync,
      l1p_flush
    } attribute_t;

    template <unsigned T_Attribute>
    static const bool supports ()
    {
      return false;
    };

    template <unsigned T_Attribute>
    static void sync ()
    {
      mem_barrier();
    };

    static void sync ()
    {
      sync<0> ();
    };

    template <> const bool supports <full_sync> () { return true; };

  };
};

#endif // __arch_MemoryInterface_h__


//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
