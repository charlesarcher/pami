/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/ambcast.c
 * \brief Simple AMBroadcast test on world geometry
 */

#define COUNT      524288
#define NITERLAT   100
/*
#define OFFSET     0
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     65536
*/

#include "../pami_util.h"

#define AMDEBUG 0

#if AMDEBUG == 1
#define DEBUG(x) fprintf x
#else
#define DEBUG(x) 
#endif

typedef struct
{
  void *rbuf;
  void *cookie;
  int bytes;
  int root;
} validation_t;

typedef struct
{
  unsigned cookie;
} user_header_t;

pami_context_t      lgContext = NULL;
pami_task_t         my_task_id;
size_t              num_tasks;
int                 _gRc = PAMI_SUCCESS;

char                   *_g_buffer;
validation_t           *_g_val_buffer;

void initialize_sndbuf (void *sbuf, int bytes, int root)
{
  unsigned char c = root;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  sbuf;

  for (; i; i--)
  {
    cbuf[i-1] = (c++);
  }
}

int check_rcvbuf (void *rbuf, int bytes, int root)
{
  unsigned char c = root;
  int i = bytes;
  unsigned char *cbuf = (unsigned char *)  rbuf;

  for (; i; i--)
  {
    if (cbuf[i-1] != c)
    {
      fprintf(stderr, "%s:Check(%d) failed <%p>rbuf[%d]=%.2u != %.2u \n", gProtocolName, bytes, rbuf, i - 1, cbuf[i-1], c);
      return 1;
    }

    c++;
  }

  return 0;
}

/**
 *  Check if we have a valid context
 */
void check_context(pami_context_t context)
{
  if(!context)
    fprintf(stderr, "%s: Error. Null context received in callback.\n",gProtocolName);
  if(lgContext != context)
    fprintf(stderr, "%s: Error. Unexpected context received in callback %p != %p.\n",gProtocolName,lgContext,context);
#ifdef PAMI_TEST_STRICT
  assert(context);
  assert(lgContext==context);
#endif

  pami_configuration_t configs;
  configs.name         = PAMI_CONTEXT_DISPATCH_ID_MAX;
  configs.value.intval = -1;

  pami_result_t rc;
  rc = PAMI_Context_query (context,&configs,1);

  if(rc != PAMI_SUCCESS && rc != PAMI_INVAL)
    fprintf(stderr,"%s: Error. Could not query the context(%u).\n",gProtocolName,rc);
#ifdef PAMI_TEST_STRICT
  assert(rc == PAMI_SUCCESS || rc == PAMI_INVAL);
#endif
}

/**
 *  Completion callback
 */
void cb_ambcast_done (void *context, void * clientdata, pami_result_t err)
{
  validation_t *v = (validation_t*)clientdata;
  volatile unsigned *active = (volatile unsigned *) v->cookie;
  DEBUG((stderr, "cb_ambcast_done(): cookie= %p value=%u\n", active, *active));
  if(gVerbose)
  {
    check_context((pami_context_t) context);
  }

  if(my_task_id != v->root)
  {
    int rc_check;
    _gRc |= rc_check = check_rcvbuf (_g_buffer, v->bytes, v->root);
    if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);
  }

  (*active)++;
}
/**
 *  User dispatch function
 */
void cb_ambcast_recv(pami_context_t        context,      /**< IN:  communication context which invoked the dispatch function */
                      void                 * cookie,       /**< IN:  dispatch cookie */
                      const void           * header_addr,  /**< IN:  header address  */
                      size_t                 header_size,  /**< IN:  header size     */
                      const void           * pipe_addr,    /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                      size_t                 data_size,    /**< IN:  data size       */
                      pami_endpoint_t        origin,       /**< IN:  root initiating endpoint */
                      pami_geometry_t        geometry,     /**< IN:  Geometry */
                      pami_recv_t          * recv)         /**< OUT: receive message structure */
{
  DEBUG((stderr,"cb_ambcast_recv(): header_addr=%p  header_size=%zu header_val=%u cookie=%p\n",
         header_addr, header_size,((user_header_t *)header_addr)->cookie, cookie));
  if(gVerbose)
  {
    check_context(context);
  }

  pami_task_t     task;
  size_t          offset;
  _gRc |= PAMI_Endpoint_query (origin,
                              &task,
                              &offset);

  validation_t *v = _g_val_buffer + task;
  v->rbuf   = _g_buffer;
  v->cookie = cookie;
  v->bytes  = data_size;
  v->root   = task;

  recv->cookie      = (void*)v;
  recv->local_fn    = cb_ambcast_done;
  recv->addr        = v->rbuf;
  recv->type        = PAMI_TYPE_BYTE;
  recv->offset      = 0;
  recv->data_fn     = PAMI_DATA_COPY;
  recv->data_cookie = NULL;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_geometry_t      world_geometry;
  pami_task_t root_task = 0;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  pami_xfer_t          barrier;
  volatile unsigned    bar_poll_flag = 0;

  /* Ambcast variables */
  size_t               ambcast_num_algorithm[2];
  pami_algorithm_t    *ambcast_always_works_algo = NULL;
  pami_metadata_t     *ambcast_always_works_md = NULL;
  pami_algorithm_t    *ambcast_must_query_algo = NULL;
  pami_metadata_t     *ambcast_must_query_md = NULL;
  pami_xfer_type_t     ambcast_xfer = PAMI_XFER_AMBROADCAST;
  pami_xfer_t          ambcast;
  volatile unsigned    ambcast_total_count = 0;

  int                  nalg = 0, i;
  double               ti, tf, usec;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);

  /* \note Test environment variable" TEST_ROOT=N, defaults to 0.*/
  char* sRoot = getenv("TEST_ROOT");
  /* Override ROOT */
  if (sRoot) root_task = (pami_task_t) atoi(sRoot);

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts      */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &my_task_id,    /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;

  if (gNumRoots > num_tasks) gNumRoots = num_tasks;

  /*  Allocate buffer(s) */
  int err = 0;
  void *buf = NULL;
  err = posix_memalign(&buf, 128, gMax_count + gBuffer_offset);
  assert(err == 0);
  buf = (char*)buf + gBuffer_offset;

  void *validation = NULL;
  err = posix_memalign((void **)&validation, 128, (num_tasks * sizeof(validation_t)) + gBuffer_offset);
  validation = (char*)validation + gBuffer_offset;

  user_header_t header;
  header.cookie = 100;

  unsigned iContext = 0;
  for (; iContext < gNum_contexts; ++iContext)
  {
    if (my_task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for barrier algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               barrier_xfer,
                               barrier_num_algorithm,
                               &bar_always_works_algo,
                               &bar_always_works_md,
                               &bar_must_query_algo,
                               &bar_must_query_md);

    if (rc == 1)
      return 1;

    /*  Query the world geometry for ambcast algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               ambcast_xfer,
                               ambcast_num_algorithm,
                               &ambcast_always_works_algo,
                               &ambcast_always_works_md,
                               &ambcast_must_query_algo,
                               &ambcast_must_query_md);

    if (rc == 1)
      return 1;

    _g_buffer = (char *)buf;
    _g_val_buffer  = (validation_t *)validation;


    barrier.cb_done     = cb_done;
    barrier.cookie      = (void*) & bar_poll_flag;
    barrier.algorithm   = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);


    validation_t v;
    v.root   = my_task_id;
    v.cookie = (void *)&ambcast_total_count;
    ambcast.cb_done = cb_ambcast_done;
    ambcast.cookie  = &v;
    ambcast.algorithm = ambcast_always_works_algo[0];
    ambcast.cmd.xfer_ambroadcast.user_header  = &header;
    ambcast.cmd.xfer_ambroadcast.headerlen    = sizeof(user_header_t);
    ambcast.cmd.xfer_ambroadcast.sndbuf       = buf;
    ambcast.cmd.xfer_ambroadcast.stype        = PAMI_TYPE_BYTE;
    ambcast.cmd.xfer_ambroadcast.stypecount   = 0;

    for (nalg = 0; nalg < ambcast_num_algorithm[0]; nalg++)
    {
      gProtocolName = ambcast_always_works_md[nalg].name;

      if (my_task_id == root_task)
      {
        printf("# AMBroadcast Bandwidth Test(size:%zu) -- context = %d, root = %d, protocol: %s\n",num_tasks,
               iContext, root_task, ambcast_always_works_md[nalg].name);
        printf("# Size(bytes)      iterations     bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
        fflush(stdout);
      }

      if (((strstr(ambcast_always_works_md[nalg].name,gSelected) == NULL) && gSelector) ||
          ((strstr(ambcast_always_works_md[nalg].name,gSelected) != NULL) && !gSelector))  continue;

      int j;
      pami_collective_hint_t h = {0};
      pami_dispatch_callback_function fn;
      lgContext = context[iContext];
      fn.ambroadcast = cb_ambcast_recv;
      PAMI_AMCollective_dispatch_set(context[iContext],
                                     ambcast_always_works_algo[nalg],
                                     root_task,/* Set the dispatch id, can be any arbitrary value */
                                     fn,
                                     (void*) &ambcast_total_count,
                                     h);
      ambcast.cmd.xfer_ambroadcast.dispatch = root_task;
      ambcast.algorithm = ambcast_always_works_algo[nalg];

      volatile unsigned *nbcast = &ambcast_total_count;
      for (i = gMin_count; i <= gMax_count; i *= 2)
      {
        size_t  dataSent = i;
        int     niter;
        pami_result_t result;

        if (dataSent < CUTOFF)
          niter = gNiterlat;
        else
          niter = NITERBW;

        *nbcast = 0;
        memset(buf, 0xFF, i);
        root_task = (root_task + num_tasks - 1) % num_tasks;
        if (my_task_id == root_task)
          initialize_sndbuf (buf, i, root_task);

        blocking_coll(context[iContext], &barrier, &bar_poll_flag);
        ti = timer();

        for (j = 0; j < niter; j++)
        {
          /*root_task = (root_task + num_tasks - 1) % num_tasks;*/
          if (my_task_id == root_task)
          {
            ambcast.cmd.xfer_ambroadcast.stypecount = i;
            result = PAMI_Collective(context[iContext], &ambcast);
            if (result != PAMI_SUCCESS)
              {
                fprintf (stderr, "Error. Unable to issue collective. result = %d\n", result);
                return 1;
              }
          }
          while (*nbcast <= j)
            result = PAMI_Context_advance (context[iContext], 1);

          rc |= _gRc; /* validation return code done in cb_ambcast_done */
        }

        assert(*nbcast == niter);
        tf = timer();
        blocking_coll(context[iContext], &barrier, &bar_poll_flag);

        usec = (tf - ti) / (double)niter;
        if(my_task_id == root_task)
        {
          printf("  %11lld %16d %14.1f %12.2f\n",
                 (long long)dataSent,
                 niter,
                 (double)1e6*(double)dataSent / (double)usec,
                 usec);
          fflush(stdout);
        }
      }
      lgContext = NULL;
    }
    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);
    free(ambcast_always_works_algo);
    free(ambcast_always_works_md);
    free(ambcast_must_query_algo);
    free(ambcast_must_query_md);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  buf = (char*)buf - gBuffer_offset;
  free(buf);

  validation = (char*)validation - gBuffer_offset;
  free(validation);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
