#include "nfs-utils.h"

static void
nfs_connect_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	if (status != RPC_STATUS_SUCCESS) {
		printf("connection to RPC.MOUNTD on server %s failed\n", client->server);
		exit(10);
	}

	printf("Connected to RPC.NFSD on %s:%d\n", client->server, client->mount_port);

	if (rpc_nfs3_null_async(rpc, client->au_rpc_cb, client) != 0) {
		printf("Failed to sent a NULL RPC\n");
		exit(10);
	}
}

static void
mount_mnt_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;
	mountres3 *mnt = data;

	if (status == RPC_STATUS_ERROR) {
		printf("mount/mnt call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
	if (status != RPC_STATUS_SUCCESS) {
		printf("mount/mnt call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	printf("Got reply from server for MOUNT/MNT procedure.\n");
	client->rootfh.data.data_len = mnt->mountres3_u.mountinfo.fhandle.fhandle3_len;
        client->rootfh.data.data_val = malloc(client->rootfh.data.data_len);
	memcpy(client->rootfh.data.data_val, mnt->mountres3_u.mountinfo.fhandle.fhandle3_val, client->rootfh.data.data_len);

	printf("Disconnect socket from mountd server\n");
	if (rpc_disconnect(rpc, "normal disconnect") != 0) {
		printf("Failed to disconnect socket to mountd\n");
		exit(10);
	}

	printf("Connect to RPC.NFSD on %s:%d\n", client->server, 2049);
	if (rpc_connect_async(rpc, client->server, 2049, nfs_connect_cb, client) != 0) {
		printf("Failed to start connection\n");
		exit(10);
	}
}

static void
mount_export_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;
	exports export = *(exports *)data;

	if (status == RPC_STATUS_ERROR) {
		printf("mount null call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
	if (status != RPC_STATUS_SUCCESS) {
		printf("mount null call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	printf("Got reply from server for MOUNT/EXPORT procedure.\n");
	while (export != NULL) {
		printf("Export: %s\n", export->ex_dir);
		export = export->ex_next;
	}
	printf("Send MOUNT/MNT command for %s\n", client->export);
	if (rpc_mount_mnt_async(rpc, mount_mnt_cb, client->export, client) != 0) {
		printf("Failed to send mnt request\n");
		exit(10);
	}
}

static void
mount_null_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	if (status == RPC_STATUS_ERROR) {
		printf("mount null call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
	if (status != RPC_STATUS_SUCCESS) {
		printf("mount null call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	printf("Got reply from server for MOUNT/NULL procedure.\n");
	printf("Send MOUNT/EXPORT command\n");
	if (rpc_mount_export_async(rpc, mount_export_cb, client) != 0) {
		printf("Failed to send export request\n");
		exit(10);
	}
}

static void
mount_connect_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	if (status != RPC_STATUS_SUCCESS) {
		printf("connection to RPC.MOUNTD on server %s failed\n", client->server);
		exit(10);
	}

	printf("Connected to RPC.MOUNTD on %s:%d\n", client->server, client->mount_port);
	printf("Send NULL request to check if RPC.MOUNTD is actually running\n");
	if (rpc_mount_null_async(rpc, mount_null_cb, client) != 0) {
		printf("Failed to send null request\n");
		exit(10);
	}
}

static void
pmap_getport2_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	if (status == RPC_STATUS_ERROR) {
		printf("portmapper getport call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
       	if (status != RPC_STATUS_SUCCESS) {
		printf("portmapper getport call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	client->mount_port = *(uint32_t *)data;
	printf("GETPORT returned RPC.MOUNTD is on port:%d\n", client->mount_port);
	if (client->mount_port == 0) {
		printf("RPC.MOUNTD is not available on server : %s:%d\n", client->server, client->mount_port);
		exit(10);
	}		

	printf("Disconnect socket from portmap server\n");
	if (rpc_disconnect(rpc, "normal disconnect") != 0) {
		printf("Failed to disconnect socket to portmapper\n");
		exit(10);
	}

	printf("Connect to RPC.MOUNTD on %s:%d\n", client->server, client->mount_port);
	if (rpc_connect_async(rpc, client->server, client->mount_port, mount_connect_cb, client) != 0) {
		printf("Failed to start connection\n");
		exit(10);
	}
}

static void
pmap_dump_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;
	struct pmap2_dump_result *dr = data;
	struct pmap2_mapping_list *list = dr->list;

	if (status == RPC_STATUS_ERROR) {
		printf("portmapper null call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
	if (status != RPC_STATUS_SUCCESS) {
		printf("portmapper null call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	printf("Got reply from server for PORTMAP/DUMP procedure.\n");
	while (list) {
		printf("Prog:%d Vers:%d Protocol:%d Port:%d\n",
			list->map.prog,
			list->map.vers,
			list->map.prot,
			list->map.port);
		list = list->next;
	}
	printf("Send getport request asking for MOUNT port\n");
	if (rpc_pmap2_getport_async(rpc, MOUNT_PROGRAM, MOUNT_V3, IPPROTO_TCP, pmap_getport2_cb, client) != 0) {
		printf("Failed to send getport request\n");
		exit(10);
	}
}

static void
pmap_null_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	if (status == RPC_STATUS_ERROR) {
		printf("portmapper null call failed with \"%s\"\n", (char *)data);
		exit(10);
	}
	if (status != RPC_STATUS_SUCCESS) {
		printf("portmapper null call to server %s failed, status:%d\n", client->server, status);
		exit(10);
	}

	printf("Got reply from server for PORTMAP/NULL procedure.\n");
	printf("Send PMAP/DUMP command\n");
	if (rpc_pmap2_dump_async(rpc, pmap_dump_cb, client) != 0) {
		printf("Failed to send getport request\n");
		exit(10);
	}
}

static void
pmap_connect_cb(struct rpc_context *rpc, int status, void *data, void *private_data)
{
	struct client *client = private_data;

	printf("pmap_connect_cb    status:%d.\n", status);
	if (status != RPC_STATUS_SUCCESS) {
		printf("connection to portmapper on server %s failed\n", client->server);
		exit(10);
	}

	printf("Send NULL request to check if portmapper is actually running\n");
	if (rpc_pmap2_null_async(rpc, pmap_null_cb, client) != 0) {
		printf("Failed to send null request\n");
		exit(10);
	}
}

void
nfs_setup(struct rpc_context *rpc, void *private_data)
{
	struct client *client = private_data;

	if (rpc == NULL) {
		printf("failed to init context\n");
		exit(10);
	}

	if (rpc_connect_async(rpc, client->server, 111, pmap_connect_cb, client) != 0) {
		printf("Failed to start connection\n");
		exit(10);
	}
}

void
nfs_destroy(struct rpc_context *rpc)
{

	rpc_destroy_context(rpc);
	rpc = NULL;
}
