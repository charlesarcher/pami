/**
 * \file api/extension/c/async_progress/Extension.h
 * \brief PAMI extension "asynchronous progress" interface template specialization
 */
#ifndef __api_extension_c_async_progress_Extension_h__
#define __api_extension_c_async_progress_Extension_h__

// if configure --with-pami-extension=async_progress
#ifdef __pami_extension_async_progress__

#include "api/extension/Extension.h"
#include "ProgressExtension.h"
#include <pami.h>

namespace PAMI
{

  template <>
  void * Extension::openExtension<4000> (pami_client_t   client,
                                         const char    * name,
                                         pami_result_t & result)
  {
    result = PAMI_SUCCESS;
    return NULL;
  }

  template <>
  void Extension::closeExtension<4000> (void          * cookie,
                                        pami_result_t & result)
  {
    result = PAMI_SUCCESS;
    return;
  }

  template <>
  void * Extension::queryExtension<4000> (const char * name,
                                          void       * cookie)
  {
    if (strcasecmp(name, "register") == 0)
      {
        return (void *)PAMI::ProgressExtension::context_async_progress_register;
      }

    if (strcasecmp(name, "enable") == 0)
      {
        return (void *)PAMI::ProgressExtension::context_async_progress_enable;
      }

    if (strcasecmp(name, "disable") == 0)
      {
        return (void *)PAMI::ProgressExtension::context_async_progress_disable;
      }

    return NULL;
  }

}; // namespace PAMI


#endif // __pami_extension_async_progress__
#endif // __api_extension_c_async_progress_Extension_h__

