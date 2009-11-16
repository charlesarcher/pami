///
/// \file common/lapi/Client.h
/// \brief XMI client interface specific for the LAPI platform.
///
#ifndef __common_lapi_Client_h__
#define __common_lapi_Client_h__

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/ClientInterface.h"
#include "Context.h"
#include "Geometry.h"

namespace XMI
{
    class Client : public Interface::Client<XMI::Client,XMI::Context>
    {
    public:

      static void shutdownfunc()
        {

        }
      inline Client (const char * name, xmi_result_t & result) :
        Interface::Client<XMI::Client,XMI::Context>(name, result),
        _client ((xmi_client_t) this),
        _references (1),
        _ncontexts (0),
        _mm ()
        {
          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);

          // Get some shared memory for this client
          initializeMemoryManager ();
          result = XMI_SUCCESS;
        }

      inline ~Client ()
        {
        }

      static xmi_result_t generate_impl (const char * name, xmi_client_t * in_client)
        {
          int rc = 0;
          XMI::Client * client;
          client = (XMI::Client *)malloc(sizeof (XMI::Client));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client));
          xmi_result_t res;
          new (client) XMI::Client (name, res);
          *in_client = (xmi_client_t*) client;
          return XMI_SUCCESS;
        }

      static void destroy_impl (xmi_client_t client)
        {
          free (client);
        }

      inline char * getName_impl ()
        {
          return _name;
        }

      inline xmi_result_t createContext_impl (xmi_configuration_t configuration[],
                                               size_t              count,
						int ncontexts)
        {
		//_context_list->lock ();
		int n = ncontexts;
		if (_ncontexts != 0) {
			return XMI_ERROR;
		}
		if (_ncontexts + n > 4) {
			n = 4 - _ncontexts;
		}
		if (n <= 0) { // impossible?
			return XMI_ERROR;
		}

#ifdef USE_MEMALIGN                
		int rc = posix_memalign((void **)&_generics, 16, sizeof(*_generics) * n);
		XMI_assertf(rc==0, "posix_memalign failed for _generics[%d], errno=%d\n", n, errno);

		rc = posix_memalign((void **)&_contexts, 16, sizeof(XMI::Context) * n);
		XMI_assertf(rc==0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
#else
		_generics = (XMI::Device::Generic::Device *)malloc(sizeof(*_generics) * n);
		XMI_assertf(_generics!=NULL, "malloc failed for _generics[%d], errno=%d\n", n, errno);
                _contexts = (XMI::Context*)malloc(sizeof(XMI::Context)*n);
		XMI_assertf(_contexts!=NULL, "malloc failed for _contexts[%d], errno=%d\n", n, errno);
#endif
		int x;
		for (x = 0; x < n; ++x) {
			new (&_generics[x]) XMI::Device::Generic::Device();
		}
		memset((void *)_contexts, 0, sizeof(XMI::Context) * n);
		size_t bytes = _mm.size() / n;
		for (x = 0; x < n; ++x) {
			void *base = NULL;
			_mm.memalign((void **)&base, 16, bytes);
			XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
			new (&_contexts[x]) XMI::Context(this->getClient(), x, n,
							_generics, base, bytes);
			//_context_list->pushHead((QueueElem *)&context[x]);
			//_context_list->unlock();
		}
		return XMI_SUCCESS;
        }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
        }

	inline size_t getNumContexts()
	{
		return _ncontexts;
	}

	inline XMI::Context *getContext(size_t ctx)
	{
		return _contexts + ctx;
	}
	inline XMI::Context *getContexts()
	{
		return _contexts;
	}

    protected:

      inline xmi_client_t getClient () const
        {
          return _client;
        }

    private:
      xmi_client_t _client;
      size_t       _references;
      size_t       _ncontexts;
	XMI::Context *_contexts;
	XMI::Device::Generic::Device *_generics;

        char         _name[256];

        Memory::MemoryManager _mm;

        inline void initializeMemoryManager ()
        {
          char   shmemfile[1024];
          size_t bytes     = 1024*1024;
          size_t pagesize  = 4096;
          
          snprintf (shmemfile, 1023, "/xmi-client-%s", _name);
          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);
          int fd, rc;
          size_t n = bytes;

          // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
          rc = shm_open (shmemfile, O_CREAT|O_RDWR,0600);
          if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );
            void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
            if ( ptr != MAP_FAILED )
                {
                  _mm.init (ptr, n);
                  return;
                }
          }
          // Failed to create shared memory .. fake it using the heap ??
          _mm.init (malloc (n), n);

          return;
        }
    }; // end class XMI::Client
}; // end namespace XMI


#endif
// __xmi_lapi_lapiclient_h__
