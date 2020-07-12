#ifndef _NFS_UTIL_H_
#define _NFS_UTIL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_POLL_H
#include <poll.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libnfs.h"
#include "libnfs-raw.h"
#include "libnfs-raw-mount.h"
#include "libnfs-raw-nfs.h"
#include "libnfs-raw-portmap.h"

struct client {
	char	*server;
	char	*export;
	uint32_t	mount_port;
	rpc_cb	au_rpc_cb;
	int	au_rpc_status;
	int	is_finished;
	struct	nfs_fh3 rootfh;
};

void nfs_setup(struct rpc_context *rpc, void *private_data);
void nfs_destroy(struct rpc_context *rpc);

#endif
