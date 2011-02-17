/**
 * \file api/extension/c/multisend/Extension.h
 * \brief PAMI multisend extension interface template specialization
 */
#ifndef __api_extension_c_multisend_Extension_h__
#define __api_extension_c_multisend_Extension_h__

#ifdef __pami_extension_multisend__ // configure --with-pami-extension=multisend

#include "api/extension/Extension.h"

namespace PAMI
{
  class ExtensionTest
  {
    public:

      typedef struct
      {
        size_t line;
        char   func[128];
      } info_t;

      static void foo ()
      {
        fprintf (stderr, "This is function 'foo' of extension 'test'. %s() [%s:%d]\n",
                 __FUNCTION__, __FILE__, __LINE__);
      };

      static void bar (info_t * info)
      {
        fprintf (stderr, "This is function 'bar' of extension 'test'.\n");
        info->line = __LINE__;
        snprintf (info->func, 127, "%s", __FUNCTION__);
      };
  };

  template <>
  void * Extension::openExtension<1> (pami_client_t   client,
                                      const char    * name,
                                      pami_result_t & result)
  {
    result = PAMI_SUCCESS;
    return NULL;
  }

  template <>
  void Extension::closeExtension<1> (void * cookie, pami_result_t & result)
  {
    result = PAMI_SUCCESS;
    return;
  }

  template <>
  void * Extension::queryExtension<1> (const char * name, void * cookie)
  {
    if (strcasecmp (name, "foo") == 0)
      return (void *) PAMI::ExtensionTest::foo;

    if (strcasecmp (name, "bar") == 0)
      return (void *) PAMI::ExtensionTest::bar;

    return NULL;
  };
};

#endif // __pami_extension_multisend__
#endif // __api_extension_c_multisend_Extension_h__
