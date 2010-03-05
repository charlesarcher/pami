/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/progressfunc/progressfunc.cc
 * \brief Test the generic progress function feature
 */

#include <stdio.h>
#include "sys/xmi.h"
//#include "Client.h"
#include "Global.h"
#include "components/devices/misc/ProgressFunctionMsg.h"

XMI::Device::ProgressFunctionMdl _model;

char msgbuf[XMI::Device::ProgressFunctionMdl::sizeof_msg];

struct my_work {
	unsigned long long count;
	unsigned long long t0;
	unsigned long v;
};

unsigned done = 0;

static xmi_result_t my_func(xmi_context_t ctx, void *cd) {
	struct my_work *w = (struct my_work *)cd;
	if (w->t0 == 0) {
		w->t0 = __global.time.timebase();
		fprintf(stderr, "Starting work at tick %llu, waiting until %llu\n",
			w->t0, w->t0 + w->count);
		w->v = 0;
	}
	unsigned long long t1 = __global.time.timebase();
	if (t1 - w->t0 >= w->count) {
		fprintf(stderr, "Finished at tick %llu (%ld calls)\n", t1, w->v);
		done = 1;
		return XMI_SUCCESS;
	}
	++w->v;
	return XMI_EAGAIN;
}

int main(int argc, char **argv) {
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("progressfunc test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	{ size_t _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, _n); }
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
		return 1;
	}

	xmi_configuration_t configuration;

	configuration.name = XMI_TASK_ID;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zu\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(client, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
	struct my_work work;
	XMI_ProgressFunc_t pf;

	work.t0 = 0;
	if (argc > 1) {
		work.count = strtoull(argv[1], NULL, 0);
	} else {
		work.count = 300000ULL;	// the printf takes time, ensure we don't
	}				// finish too soon.

	pf.client = 0;
	pf.context = 0;
	pf.request = &msgbuf[0];
	pf.func = my_func;
	pf.clientdata = &work;
	bool rc = _model.postWork(&pf);
	if (!rc) {
		fprintf(stderr, "Failed to generateMessage on progress function\n");
		exit(1);
	}
	while (!done) {
		XMI_Context_advance(context, 100);
	}
	fprintf(stderr, "Test completed\n");

// ------------------------------------------------------------------------

	status = XMI_Context_destroy(context);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
		return 1;
	}

	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
