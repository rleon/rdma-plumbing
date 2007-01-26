/*
 * Copyright (c) 2005 Voltaire Inc.  All rights reserved.
 * Copyright (c) 2005-2006 Intel Corporation.  All rights reserved.
 *
 * This Software is licensed under one of the following licenses:
 *
 * 1) under the terms of the "Common Public License 1.0" a copy of which is
 *    available from the Open Source Initiative, see
 *    http://www.opensource.org/licenses/cpl.php.
 *
 * 2) under the terms of the "The BSD License" a copy of which is
 *    available from the Open Source Initiative, see
 *    http://www.opensource.org/licenses/bsd-license.php.
 *
 * 3) under the terms of the "GNU General Public License (GPL) Version 2" a
 *    copy of which is available from the Open Source Initiative, see
 *    http://www.opensource.org/licenses/gpl-license.php.
 *
 * Licensee has the right to choose one of the above licenses.
 *
 * Redistributions of source code must retain the above copyright
 * notice and one of the license notices.
 *
 * Redistributions in binary form must reproduce both the above copyright
 * notice, one of the license notices in the documentation
 * and/or other materials provided with the distribution.
 *
 */

#if !defined(RDMA_CMA_H)
#define RDMA_CMA_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <infiniband/verbs.h>
#include <infiniband/sa.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Upon receiving a device removal event, users must destroy the associated
 * RDMA identifier and release all resources allocated with the device.
 */
enum rdma_cm_event_type {
	RDMA_CM_EVENT_ADDR_RESOLVED,
	RDMA_CM_EVENT_ADDR_ERROR,
	RDMA_CM_EVENT_ROUTE_RESOLVED,
	RDMA_CM_EVENT_ROUTE_ERROR,
	RDMA_CM_EVENT_CONNECT_REQUEST,
	RDMA_CM_EVENT_CONNECT_RESPONSE,
	RDMA_CM_EVENT_CONNECT_ERROR,
	RDMA_CM_EVENT_UNREACHABLE,
	RDMA_CM_EVENT_REJECTED,
	RDMA_CM_EVENT_ESTABLISHED,
	RDMA_CM_EVENT_DISCONNECTED,
	RDMA_CM_EVENT_DEVICE_REMOVAL,
	RDMA_CM_EVENT_MULTICAST_JOIN,
	RDMA_CM_EVENT_MULTICAST_ERROR
};

enum rdma_port_space {
	RDMA_PS_IPOIB= 0x0002,
	RDMA_PS_TCP  = 0x0106,
	RDMA_PS_UDP  = 0x0111,
};

/*
 * Global qkey value for UDP QPs and multicast groups created via the 
 * RDMA CM.
 */
#define RDMA_UDP_QKEY 0x01234567

struct ib_addr {
	union ibv_gid	sgid;
	union ibv_gid	dgid;
	uint16_t	pkey;
};

struct rdma_addr {
	struct sockaddr		src_addr;
	uint8_t			src_pad[sizeof(struct sockaddr_storage) -
					sizeof(struct sockaddr)];
	struct sockaddr		dst_addr;
	uint8_t			dst_pad[sizeof(struct sockaddr_storage) -
					sizeof(struct sockaddr)];
	union {
		struct ib_addr	ibaddr;
	} addr;
};

struct rdma_route {
	struct rdma_addr	 addr;
	struct ibv_sa_path_rec	*path_rec;
	int			 num_paths;
};

struct rdma_event_channel {
	int			fd;
};

struct rdma_cm_id {
	struct ibv_context	*verbs;
	struct rdma_event_channel *channel;
	void			*context;
	struct ibv_qp		*qp;
	struct rdma_route	 route;
	enum rdma_port_space	 ps;
	uint8_t			 port_num;
};

struct rdma_conn_param {
	const void *private_data;
	uint8_t private_data_len;
	uint8_t responder_resources;
	uint8_t initiator_depth;
	uint8_t flow_control;
	uint8_t retry_count;		/* ignored when accepting */
	uint8_t rnr_retry_count;
	/* Fields below ignored if a QP is created on the rdma_cm_id. */
	uint8_t srq;
	uint32_t qp_num;
};

struct rdma_ud_param {
	const void *private_data;
	uint8_t private_data_len;
	struct ibv_ah_attr ah_attr;
	uint32_t qp_num;
	uint32_t qkey;
};

struct rdma_cm_event {
	struct rdma_cm_id	*id;
	struct rdma_cm_id	*listen_id;
	enum rdma_cm_event_type	 event;
	int			 status;
	union {
		struct rdma_conn_param conn;
		struct rdma_ud_param   ud;
	} param;
};

/**
 * rdma_create_event_channel - Open a channel used to report communication
 *   events.
 */
struct rdma_event_channel *rdma_create_event_channel(void);

/**
 * rdma_destroy_event_channel - Close the event communication channel.
 * @channel: The communication channel to destroy.
 */
void rdma_destroy_event_channel(struct rdma_event_channel *channel);

/**
 * rdma_create_id - Allocate a communication identifier.
 * @channel: The communication channel that events associated with the
 *   allocated rdma_cm_id will be reported on.
 * @id: A reference where the allocated communication identifier will be
 *   returned.
 * @context: User specified context associated with the rdma_cm_id.
 * @ps: RDMA port space.
 */
int rdma_create_id(struct rdma_event_channel *channel,
		   struct rdma_cm_id **id, void *context,
		   enum rdma_port_space ps);

/**
 * rdma_destroy_id - Release a communication identifier.
 * @id: The communication identifier to destroy.
 * 
 * Calling this routine has the effect of canceling any outstanding
 * asynchronous operation on the associated id.
 */
int rdma_destroy_id(struct rdma_cm_id *id);

/**
 * rdma_bind_addr - Bind an RDMA identifier to a source address and
 *   associated RDMA device, if needed.
 *
 * @id: RDMA identifier.
 * @addr: Local address information.  Wildcard values are permitted.
 *
 * This associates a source address with the RDMA identifier before calling
 * rdma_listen.  If a specific local address is given, the RDMA identifier will
 * be bound to a local RDMA device.
 */
int rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr);

/**
 * rdma_resolve_addr - Resolve destination and optional source addresses
 *   from IP addresses to an RDMA address.  If successful, the specified
 *   rdma_cm_id will be bound to a local device.
 *
 * @id: RDMA identifier.
 * @src_addr: Source address information.  This parameter may be NULL.
 * @dst_addr: Destination address information.
 * @timeout_ms: Time to wait for resolution to complete.
 */
int rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		      struct sockaddr *dst_addr, int timeout_ms);

/**
 * rdma_resolve_route - Resolve the RDMA address bound to the RDMA identifier
 *   into route information needed to establish a connection.
 *
 * This is called on the client side of a connection.
 * Users must have first called rdma_resolve_addr to resolve a dst_addr
 * into an RDMA address before calling this routine.
 */
int rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms);

/**
 * rdma_create_qp - Allocate a QP and associate it with the specified RDMA
 * identifier.
 *
 * QPs allocated to an rdma_cm_id will automatically be transitioned by the CMA
 * through their states.
 */
int rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		   struct ibv_qp_init_attr *qp_init_attr);

/**
 * rdma_destroy_qp - Deallocate the QP associated with the specified RDMA
 * identifier.
 *
 * Users must destroy any QP associated with an RDMA identifier before
 * destroying the RDMA ID.
 */
void rdma_destroy_qp(struct rdma_cm_id *id);

/**
 * rdma_connect - Initiate an active connection request.
 *
 * Users must have resolved a route for the rdma_cm_id to connect with
 * by having called rdma_resolve_route before calling this routine.
 */
int rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);

/**
 * rdma_listen - This function is called by the passive side to
 *   listen for incoming connection requests.
 *
 * Users must have bound the rdma_cm_id to a local address by calling
 * rdma_bind_addr before calling this routine.
 */
int rdma_listen(struct rdma_cm_id *id, int backlog);

/**
 * rdma_accept - Called to accept a connection request.
 * @id: Connection identifier associated with the request.
 * @conn_param: Information needed to establish the connection.
 */
int rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);

/**
 * rdma_reject - Called on the passive side to reject a connection request.
 */
int rdma_reject(struct rdma_cm_id *id, const void *private_data,
		uint8_t private_data_len);

/**
 * rdma_notify - Notifies the RDMA CM of an asynchronous event that has
 * occurred on the connection.
 * @id: Connection identifier to transition to established.
 * @event: Asynchronous event.
 *
 * This routine should be invoked by users to notify the CM of relevant
 * communication events.  Events that should be reported to the CM and
 * when to report them are:
 *
 * IB_EVENT_COMM_EST - Used when a message is received on a connected
 *    QP before an RTU has been received.
 */
int rdma_notify(struct rdma_cm_id *id, enum ibv_event_type event);

/**
 * rdma_disconnect - This function disconnects the associated QP and
 *   transitions it into the error state.
 */
int rdma_disconnect(struct rdma_cm_id *id);

/**
 * rdma_join_multicast - Join the multicast group specified by the given
 *   address.
 * @id: Communication identifier associated with the request.
 * @addr: Multicast address identifying the group to join.
 * @context: User-defined context associated with the join request.  The
 *   context is returned to the user through the private_data field in
 *   the rdma_cm_event.
 */
int rdma_join_multicast(struct rdma_cm_id *id, struct sockaddr *addr,
			void *context);

/**
 * rdma_leave_multicast - Leave the multicast group specified by the given
 *   address.
 */
int rdma_leave_multicast(struct rdma_cm_id *id, struct sockaddr *addr);

/**
 * rdma_get_cm_event - Retrieves the next pending communications event,
 *   if no event is pending waits for an event.
 * @channel: Event channel to check for events.
 * @event: Allocated information about the next communication event.
 *    Event should be freed using rdma_ack_cm_event()
 *
 * A RDMA_CM_EVENT_CONNECT_REQUEST communication events result 
 * in the allocation of a new @rdma_cm_id. 
 * Clients are responsible for destroying the new @rdma_cm_id.
 */
int rdma_get_cm_event(struct rdma_event_channel *channel,
		      struct rdma_cm_event **event);

/**
 * rdma_ack_cm_event - Free a communications event.
 * @event: Event to be released.
 *
 * All events which are allocated by rdma_get_cm_event() must be released,
 * there should be a one-to-one correspondence between successful gets
 * and acks.
 */
int rdma_ack_cm_event(struct rdma_cm_event *event);

static inline uint16_t rdma_get_src_port(struct rdma_cm_id *id)
{
	return	id->route.addr.src_addr.sa_family == PF_INET6 ?
		((struct sockaddr_in6 *) &id->route.addr.src_addr)->sin6_port :
		((struct sockaddr_in *) &id->route.addr.src_addr)->sin_port;
}

static inline uint16_t rdma_get_dst_port(struct rdma_cm_id *id)
{
	return	id->route.addr.dst_addr.sa_family == PF_INET6 ?
		((struct sockaddr_in6 *) &id->route.addr.dst_addr)->sin6_port :
		((struct sockaddr_in *) &id->route.addr.dst_addr)->sin_port;
}

/**
 * rdma_get_devices - Get list of RDMA devices currently available.
 * @num_devices: If non-NULL, set to the number of devices returned.
 *
 * Return a NULL-terminated array of opened RDMA devices.  Callers can use this
 * routine to allocate resources on specific RDMA devices that will be shared
 * across multiple rdma_cm_id's.
 * The array must be released by calling rdma_free_devices().
 */
struct ibv_context **rdma_get_devices(int *num_devices);

/**
 * rdma_free_devices - Frees the list of devices returned by rdma_get_devices().
 */
void rdma_free_devices(struct ibv_context **list);

#ifdef __cplusplus
}
#endif

#endif /* RDMA_CMA_H */