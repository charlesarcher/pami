/**
 * \file test/api/p2p/rget/simple_rget_func.c
 * \brief Simple point-to-point PAMI_Rget() test
 *
 * This test implements a very simple "rendezvous" communication and
 * depends on a functional PAMI_Send_immediate() function.
 */

#include <pami.h>
#include <stdio.h>
#include <stdint.h>

/*#define TEST_CROSSTALK */

/*#define USE_SHMEM_OPTION */
/*#define NO_SHMEM_OPTION */

#define DISPATCH_ID_RTS 0
#define DISPATCH_ID_ACK 1

#define BUFFERSIZE 16

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  /*fprintf x */
#endif

typedef struct
{
  pami_endpoint_t   origin;
  size_t            bytes;
  pami_memregion_t  memregion;
} rts_info_t;

typedef struct
{
  pami_endpoint_t   origin;
  size_t            bytes;
  size_t            pad;
  uint32_t          buffer[BUFFERSIZE<<1];
  pami_memregion_t  memregion;
  volatile size_t * value;
} get_info_t;

size_t  _ack_status;

void print_data (void * addr, size_t bytes)
{
  uint8_t * p = (uint8_t *) addr;
  size_t i;
  for (i=0; i<bytes; i+=4)
    {
      fprintf (stderr, "  addr[%04zu] (%p): %02x %02x %02x %02x\n", i, &p[i], p[i], p[i+1], p[i+2], p[i+3]);
    }
}


void initialize_data (uint32_t * addr, size_t bytes, size_t pad)
{
  size_t i = 0;
  uint8_t * p    = (uint8_t *) addr;
  uint8_t * data = (uint8_t *) &p[pad];

  uint8_t deadbeef[4];
  deadbeef[0] = 0xde;
  deadbeef[1] = 0xad;
  deadbeef[2] = 0xbe;
  deadbeef[3] = 0xef;

  /* fill the entire buffer with "deadbeef" */
  for (i=0; i<(pad * 2 + bytes); i++)
    p[i] = deadbeef[i%4];

  /* fill the data pattern */
  for (i=0; i<bytes; i++)
    data[i] = (uint8_t) (~(bytes-i));
}

unsigned validate_data (uint32_t * addr, size_t bytes, size_t pad)
{
  unsigned success = 1;
  size_t i = 0;

  TRACE_ERR((stderr, "validate_data(%p,%zu,%zu)\n", addr, bytes, pad));

  uint8_t deadbeef[4];
  deadbeef[0] = 0xde;
  deadbeef[1] = 0xad;
  deadbeef[2] = 0xbe;
  deadbeef[3] = 0xef;

  /* check the "pad" on either side of the buffer */
  uint8_t * p = (uint8_t *) addr;
  for (i=0; i<pad; i++)
  {
    if (p[i] != deadbeef[i%4])
    {
      success = 0;
      fprintf (stderr, "  Error. (address: %p) p[%zu] != 0x%02x (value is 0x%02x)\n", &p[i], i, deadbeef[i%4], p[i]);
    }
  }
  p += pad + bytes;
  for (i=0; i<pad; i++)
  {
    if (p[i] != deadbeef[i%4])
    {
      success = 0;
      fprintf (stderr, "  Error. (address: %p) p[%zu] != 0x%02x (value is 0x%02x)\n", &p[i], i+pad+bytes, deadbeef[i%4], p[i]);
    }
  }

  /* check the data in the middle */
  p = (uint8_t *) addr;
  p += pad;
  for (i=0; i<bytes; i++)
  {
    if (p[i] != (uint8_t)(~(bytes-i)))
    {
      success = 0;
      fprintf (stderr, "  Error. (address: %p) p[%zu] != 0x%02x (value is 0x%02x)\n", &p[i], i+pad, (uint8_t)(~(bytes-i)), p[i]);
    }
  }

  return success;
}

static void dispatch_ack (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t origin,
pami_recv_t        * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, ">> 'ack' dispatch function.  cookie = %p (active: %zu), header_size = %zu, pipe_size = %zu, recv = %p\n", cookie, *active, header_size, pipe_size, recv);

  _ack_status = *((size_t *) header_addr);

  (*active)--;

  fprintf (stderr, "<< 'ack' dispatch function.  cookie = %p (active: %zu)\n", cookie, *active);

  return;
}

static void get_done (pami_context_t   context,
                      void           * cookie,
                      pami_result_t    result)
{
  get_info_t * info = (get_info_t *) cookie;

  fprintf (stderr, ">> 'get_done' callback, cookie = %p (info->value = %zu => %zu), result = %d\n", cookie, *(info->value), *(info->value)-1, result);

  size_t status = 0; /* success */
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "   'get_done' callback, PAMI_Get failed\n");
    status = 1; /* get failed */
  }
  else
  {
    /* validate the data! */
    print_data ((void *)info->buffer, info->bytes + info->pad * 2);
    if (!validate_data(info->buffer, info->bytes, info->pad))
    {
      fprintf (stderr, "   'get_done' callback,) PAMI_Get data validation error.\n");
      status = 2; /* get data validation failure */
    }
  }

  /* Send an 'ack' to the origin */
  pami_send_immediate_t parameters;
  parameters.dispatch        = DISPATCH_ID_ACK;
  parameters.dest            = info->origin;
  parameters.header.iov_base = &status;
  parameters.header.iov_len  = sizeof(status);
  parameters.data.iov_base   = NULL;
  parameters.data.iov_len    = 0;
  PAMI_Send_immediate (context, &parameters);

  /* Destroy the local memory region */
  PAMI_Memregion_destroy (context, &(info->memregion));

  free (cookie);

  --*(info->value);
  fprintf (stderr, "<< 'get_done' callback\n");
}

static void dispatch_rts (
    pami_context_t       context,      /**< IN: PAMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    const void         * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    const void         * pipe_addr,    /**< IN: address of PAMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of PAMI pipe buffer */
    pami_endpoint_t origin,
pami_recv_t        * recv)        /**< OUT: receive message structure */
{
  volatile size_t * active = (volatile size_t *) cookie;
  fprintf (stderr, ">> 'rts' dispatch function.  cookie = %p (active: %zu), header_size = %zu, pipe_size = %zu, recv = %p\n", cookie, *active, header_size, pipe_size, recv);

  rts_info_t * rts = (rts_info_t *) header_addr;
  fprintf (stderr, "   'rts' dispatch function.  rts->origin = 0x%08x, rts->bytes = %zu\n", rts->origin, rts->bytes);

  /*assert(pipe_addr!=NULL); */
  /*pami_memregion_t * origin_memregion = (pami_memregion_t *) pipe_addr; */

  get_info_t * get = (get_info_t *) malloc (sizeof(get_info_t));
  get->value  = active;
  get->origin = rts->origin;
  get->bytes  = rts->bytes;
  get->pad    = 16;

  initialize_data (get->buffer, 0, BUFFERSIZE<<1);
  print_data (get->buffer, BUFFERSIZE<<2);

  /* Create a memregion for the data buffer. */
  size_t bytes = 0;
  PAMI_Memregion_create (context, get->buffer, BUFFERSIZE>>2, &bytes, &(get->memregion));

  /* Perform the rdma get operation */
  pami_rget_simple_t parameters;
  parameters.rma.dest    = rts->origin;
  parameters.rma.bytes   = rts->bytes;
  parameters.rma.cookie  = get;
  parameters.rma.done_fn = get_done;
  parameters.rdma.local.mr      = &(get->memregion);
  parameters.rdma.local.offset  = get->pad;
  parameters.rdma.remote.mr     = &(rts->memregion);
  parameters.rdma.remote.offset = 0;

  fprintf (stderr, "   'rts' dispatch function.  Before PAMI_Rget()\n");
  pami_result_t status = PAMI_Rget (context, &parameters);
  fprintf (stderr, "   'rts' dispatch function.   After PAMI_Rget(), status = %d\n", status);
  if (status != PAMI_SUCCESS)
    get_done (context, (void *) get, status);

  fprintf (stderr, "<< 'rts' dispatch function.\n");

  return;
}


int main (int argc, char ** argv)
{
  volatile size_t _rts_active = 1;
  volatile size_t _ack_active = 1;


  pami_client_t client;
  pami_context_t context[2];

  char                  cl_string[] = "TEST";
  pami_result_t result = PAMI_ERROR;

  result = PAMI_Client_create (cl_string, &client, NULL, 0);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami client. result = %d\n", result);
    return 1;
  }

#ifdef TEST_CROSSTALK
  size_t num = 2;
#else
  size_t num = 1;
#endif
  result = PAMI_Context_createv(client, NULL, 0, context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create pami context(s). result = %d\n", result);
    return 1;
  }

  pami_configuration_t configuration;

  configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  pami_task_t task_id = configuration.value.intval;
  fprintf (stderr, "My task id = %d\n", task_id);

  configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(client, &configuration,1);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  fprintf (stderr, "Number of tasks = %zu\n", num_tasks);
  if (num_tasks < 2)
  {
    fprintf (stderr, "Error. This test requires at least 2 tasks. Number of tasks in this job: %zu\n", num_tasks);
    return 1;
  }

  pami_send_hint_t options={};

#ifdef USE_SHMEM_OPTION
  options.use_shmem = PAMI_HINT3_FORCE_ON;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced ON\n");
  fprintf (stderr, "##########################################\n");
#elif defined(NO_SHMEM_OPTION)
  options.use_shmem = PAMI_HINT3_FORCE_OFF;
  fprintf (stderr, "##########################################\n");
  fprintf (stderr, "shared memory optimizations forced OFF\n");
  fprintf (stderr, "##########################################\n");
#endif

  size_t i = 0;
#ifdef TEST_CROSSTALK
  for (i=0; i<2; i++)
#endif
  {
    pami_dispatch_callback_fn fn;

    fprintf (stderr, "Before PAMI_Dispatch_set(%d) .. &_rts_active = %p, _rts_active = %zu\n", DISPATCH_ID_RTS, &_rts_active, _rts_active);
    fn.p2p = dispatch_rts;
    result = PAMI_Dispatch_set (context[i],
                                DISPATCH_ID_RTS,
                                fn,
                                (void *)&_rts_active,
                                options);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }

    fprintf (stderr, "Before PAMI_Dispatch_set(%d) .. &_ack_active = %p, _ack_active = %zu\n", DISPATCH_ID_ACK, &_ack_active, _ack_active);
    fn.p2p = dispatch_ack;
    result = PAMI_Dispatch_set (context[i],
                                DISPATCH_ID_ACK,
                                fn,
                                (void *)&_ack_active,
                                options);
    if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
      return 1;
    }
  }

  if (task_id == 0)
  {
    pami_send_immediate_t parameters;
#ifdef TEST_CROSSTALK
    fprintf (stdout, "PAMI_Rget('simple') functional test [crosstalk]\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, num_tasks-1, 1, &parameters.dest);
#else
    fprintf (stdout, "PAMI_Rget('simple') functional test\n");
    fprintf (stdout, "\n");
    PAMI_Endpoint_create (client, num_tasks-1, 0, &parameters.dest);
#endif


    /* Allocate some memory from the heap. */
    void * send_buffer = malloc (BUFFERSIZE);

    /* Initialize the memory for validation. */
    initialize_data ((uint32_t *)send_buffer, BUFFERSIZE, 0);
    print_data (send_buffer, BUFFERSIZE);

    /* Send an 'rts' message to the target task and provide the memory region */
    rts_info_t rts_info;
    PAMI_Endpoint_create (client, 0, 0, &rts_info.origin);
    rts_info.bytes  = BUFFERSIZE;

    /* Create a memory region for this memoru buffer */
    size_t bytes = 0;
    PAMI_Memregion_create (context[0], send_buffer, BUFFERSIZE, &bytes, &(rts_info.memregion));

    parameters.dispatch        = DISPATCH_ID_RTS;
    parameters.header.iov_base = &rts_info;
    parameters.header.iov_len  = sizeof(rts_info_t);
    parameters.data.iov_base   = NULL;
    parameters.data.iov_len    = 0;
fprintf (stderr, "Before PAMI_Send_immediate()\n");
    PAMI_Send_immediate (context[0], &parameters);

    /* wait for the 'ack' */
fprintf (stderr, "Wait for 'ack', _ack_active = %zu\n", _ack_active);
    while (_ack_active != 0)
    {
      result = PAMI_Context_advance (context[0], 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }

    /* Destroy the local memory region */
    PAMI_Memregion_destroy (context[0], &(rts_info.memregion));

    free (send_buffer);

    switch (_ack_status)
    {
      case 0:
        fprintf (stdout, "Test PASSED\n");
        break;
      case 1:
        fprintf (stdout, "Test FAILED (rget error)\n");
        break;
      case 2:
        fprintf (stdout, "Test FAILED (data error)\n");
        break;
      default:
        fprintf (stdout, "Test FAILED (unknown error)\n");
        break;
    }

  }
  else if (task_id == num_tasks-1)
  {
#ifdef TEST_CROSSTALK
      size_t contextid = 1;
#else
      size_t contextid = 0;
#endif

    /* wait for the 'rts' */
fprintf (stderr, "Wait for 'rts', _rts_active = %zu, contextid = %zu\n", _rts_active, contextid);
    while (_rts_active != 0)
    {
      result = PAMI_Context_advance (context[contextid], 100);
      if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
        return 1;
      }
    }
  }
fprintf (stderr, "Test completed .. cleanup\n");

 result = PAMI_Context_destroyv (context, num);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
    return 1;
  }

  result = PAMI_Client_destroy(&client);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to destroy pami client. result = %d\n", result);
    return 1;
  }

  /*fprintf (stdout, "Success (%d)\n", task_id); */

  return 0;
};
