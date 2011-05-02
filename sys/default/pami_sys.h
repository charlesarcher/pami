/**
 * \file sys/default/pami_sys.h
 * \brief "Default" platform specifc type definitions for IBM's PAMI message layer.
 */

#ifndef __default_pami_sys_h__
#define __default_pami_sys_h__

#define PAMI_CLIENT_MEMREGION_SIZE_STATIC (8*sizeof(uintptr_t))
#define PAMI_WORK_SIZE_STATIC (8*sizeof(uintptr_t))
#define PAMI_REQUEST_NQUADS 512

#endif /* __default_pami_sys_h__ */
