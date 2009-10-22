#include <stdio.h>
#include "sys/xmi.h"

#include "PipeWorkQueue.h"
#include "Topology.h"

#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"


#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

void fail_reg(const char *s) {
	fprintf(stderr, "failed to register \"%s\"\n", s);
	exit(1);
}

//#define LOCAL_BCAST_NAME	"XMI::Device::WQRingBcastMdl"
//#define LOCAL_BCAST_MODEL	XMI::Device::WQRingBcastMdl
#define LOCAL_BCAST_NAME	"XMI::Device::LocalBcastWQModel"
#define LOCAL_BCAST_MODEL	XMI::Device::LocalBcastWQModel

char lr[sizeof(LOCAL_BCAST_MODEL)];
char lrm[LOCAL_BCAST_MODEL::sizeof_msg];

char source[TEST_BUF_SIZE];
char result[TEST_BUF_SIZE];

XMI_PIPEWORKQUEUE_CLASS ipwq;
XMI_PIPEWORKQUEUE_CLASS opwq;
XMI_TOPOLOGY_CLASS itopo;
XMI_TOPOLOGY_CLASS otopo;

int done = 0;

void done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
	fprintf(stderr, "... completion callback for %s, done %d ++\n", (char *)cookie, done);
	++done;
}

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multicast test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	status = XMI_Context_create(client, NULL, 0, &context);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
		return 1;
	}

	xmi_configuration_t configuration;

	configuration.name = XMI_TASK_ID;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t task_id = configuration.value.intval;
	fprintf(stderr, "My task id = %zd\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	fprintf(stderr, "Number of tasks = %zd\n", num_tasks);
	if (__global.topology_local.size() < 2) {
		fprintf(stderr, "requires at least 2 ranks to be local\n");
		exit(1);
	}



// END standard setup
// ------------------------------------------------------------------------

	// Register some multicasts, C++ style

	size_t root = __global.topology_local.index2Rank(0);
	const char *test = LOCAL_BCAST_NAME;
	LOCAL_BCAST_MODEL *model = new (&lr) LOCAL_BCAST_MODEL(status);
	if (status != XMI_SUCCESS) fail_reg(test);

	new (&itopo) XMI_TOPOLOGY_CLASS(root);
	if (task_id != root) {
		new (&otopo) XMI_TOPOLOGY_CLASS(task_id);
	}

	ipwq.configure(NULL, source, sizeof(source), sizeof(source));
	ipwq.reset();
	opwq.configure(NULL, result, sizeof(result), 0);
	opwq.reset();
	xmi_multicast_t mcast;

	// simple allreduce on the local ranks...
	mcast.request = &lrm;
	mcast.cb_done = (xmi_callback_t){done_cb, (void *)test};
	mcast.roles = (unsigned)-1;
	mcast.src = (xmi_pipeworkqueue_t *)&ipwq;
	mcast.src_participants = (xmi_topology_t *)&itopo;
	mcast.dst = (xmi_pipeworkqueue_t *)&opwq;
	mcast.dst_participants = (xmi_topology_t *)&otopo;
	mcast.bytes = TEST_BUF_SIZE;
	size_t count = mcast.bytes / sizeof(unsigned);
	for (x = 0; x < count; ++x) {
		((unsigned *)source)[x] = 1;
		((unsigned *)result)[x] = -1;
	}
	done = 0;
	fprintf(stderr, "... before %s.postMulticast\n", test);
	bool rc = model->postMulticast(&mcast);
	if (!rc) {
		fprintf(stderr, "Failed to post multicast \"%s\"\n", test);
		// need to skip rest...
	}

	fprintf(stderr, "... before advance loop for %s.postMulticast\n", test);
	while (!done) {
		status = XMI_Context_advance(context, 100);
		if (status != XMI_SUCCESS) {
			fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", status);
			return 1;
		}
	}
	if (task_id == root) {
		for (x = 0; x < count; ++x) {
			if (((unsigned *)source)[x] != 1) {
				fprintf(stderr, "Corrupted source buffer at index %d. stop.\n", x);
				break;
			}
			if (((unsigned *)result)[x] != -1) {
				fprintf(stderr, "Corrupted result buffer at index %d. stop.\n", x);
				break;
			}
		}
	} else {
		for (x = 0; x < count; ++x) {
			if (((unsigned *)source)[x] != 1) {
				fprintf(stderr, "Corrupted source buffer at index %d. stop.\n", x);
				break;
			}
			if (((unsigned *)result)[x] != 1) {
				fprintf(stderr, "Incorrect result at index %d. stop.\n", x);
				break;
			}
		}
	}
	if (x < count) {
		// failed
	} else {
		fprintf(stderr, "PASS\n");
	}

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
