/*
A very thin wrapper around BSD sockets for Duktape in order to play
around with socket programming in a higher-level language.

Based on the following tutorial:
http://www.binarytides.com/winsock-socket-programming-tutorial/
*/

/* Vista or higher compatibility */
#define WINVER _WIN32_WINNT_VISTA 
#define _WIN32_WINNT _WIN32_WINNT_VISTA 

#define DUK_SOCK_BUFLEN 512

#include <stdio.h>

#include <duktape.h>

#include <winsock2.h>
#include <ws2tcpip.h>


// ----------------------------------------------------------------------------

static void register_constant(duk_context *ctx, duk_idx_t idx, const char *name, duk_int_t value) {
	duk_push_int(ctx, value);
	(void) duk_put_prop_string(ctx, idx, name);
}

#define REGISTER_CONST(ctx, i, c)  register_constant(ctx, i, #c, c)


// ----------------------------------------------------------------------------

static duk_ret_t duk_socket(duk_context *ctx) {
	SOCKET s;

	duk_int_t family = duk_require_int(ctx, 0);
	duk_int_t type = duk_require_int(ctx, 1);
	duk_int_t protocol = duk_require_int(ctx, 2);

	s = socket(family, type, protocol);
	if (s == INVALID_SOCKET) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not create socket (error code %d)", WSAGetLastError());
        return -1;
	}

	duk_push_pointer(ctx, (void*) s);
	return 1;
}


static void duk_get_sockaddr(duk_context *ctx, struct sockaddr_in *server) {
	const char *addr = NULL;
	unsigned long addr_int = INADDR_NONE;

	duk_int_t family;
	duk_int_t port;

	if (duk_is_object(ctx, 1)) {

		duk_get_prop_string(ctx, 1, "address");
		if (duk_is_string(ctx, -1)) {
			addr = duk_require_string(ctx, -1);
		} else {
			addr_int = duk_require_int(ctx, -1);
		}
		duk_pop(ctx);

		duk_get_prop_string(ctx, 1, "family");
		family = duk_require_int(ctx, -1);
		duk_pop(ctx);

		duk_get_prop_string(ctx, 1, "port");
		port = duk_require_int(ctx, -1);
		duk_pop(ctx);

	} else {

		if (duk_is_string(ctx, 1)) {
			addr = duk_require_string(ctx, 1);
		} else {
			addr_int = duk_require_int(ctx, 1);
		}

		family = duk_require_int(ctx, 2);
		port = duk_require_int(ctx, 3);
	}

	server->sin_addr.s_addr = addr != NULL ? inet_addr(addr) : addr_int;
    server->sin_family = family;
    server->sin_port = htons( port );
}

static void duk_put_sockaddr(duk_context *ctx, duk_idx_t obj_index, struct sockaddr_in *client) {
	if (duk_is_object(ctx, obj_index)) {
		char *address = inet_ntoa(client->sin_addr);
		duk_int_t port = ntohs(client->sin_port);
		duk_int_t family = client->sin_family;

		duk_push_string(ctx, address);
		duk_put_prop_string(ctx, obj_index, "address");

		duk_push_int(ctx, family);
		duk_put_prop_string(ctx, obj_index, "family");

		duk_push_int(ctx, port);
		duk_put_prop_string(ctx, obj_index, "port");
	}
}


static duk_ret_t duk_connect(duk_context *ctx) {
	SOCKET s;
	struct sockaddr_in server;

	s = (SOCKET) duk_require_pointer(ctx, 0);
	duk_get_sockaddr(ctx, &server);

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
    	duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not connect");
        return -1;
    }

    return 0;
}

static duk_ret_t duk_inetaddr(duk_context *ctx) {
	const char *addr = duk_require_string(ctx, 0);
	unsigned long result = INADDR_NONE;

	result = inet_addr(addr);

	duk_push_int(ctx, result);
	return 1;
}

static duk_ret_t duk_send(duk_context *ctx) {
	SOCKET s;
	const char *message;
	int messagelen;

	s = (SOCKET) duk_require_pointer(ctx, 0);
	message = duk_require_lstring(ctx, 1, &messagelen);

	if( send(s, message, messagelen, 0) < 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not send message");
        return -1;
	}

	return 0;
}

static duk_ret_t duk_recv(duk_context *ctx) {
	SOCKET s;
	char buff[DUK_SOCK_BUFLEN];
	int recv_size;

	s = (SOCKET) duk_require_pointer(ctx, 0);

	if((recv_size = recv(s, buff, DUK_SOCK_BUFLEN, 0)) == SOCKET_ERROR) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not receive reply");
        return -1;
	}

	duk_push_lstring(ctx, buff, recv_size);
	return 1;
}

static duk_ret_t duk_closesocket(duk_context *ctx) {
	SOCKET s;
	s = (SOCKET) duk_require_pointer(ctx, 0);

	closesocket(s);

	return 0;
}

static duk_ret_t duk_get_addrinfo(duk_context *ctx, struct addrinfo *hints, duk_idx_t idx) {
	int flags = 0, family = AF_UNSPEC, socktype = SOCK_STREAM, protocol = IPPROTO_TCP;

	if (duk_is_object(ctx, idx)) {
		duk_get_prop_string(ctx, idx, "flags");
		if (duk_is_number(ctx, -1)) {
			flags = duk_require_int(ctx, -1);
		}
		duk_pop(ctx);

		duk_get_prop_string(ctx, idx, "family");
		if (duk_is_number(ctx, -1)) {
			family = duk_require_int(ctx, -1);
		}
		duk_pop(ctx);

		duk_get_prop_string(ctx, idx, "socktype");
		if (duk_is_number(ctx, -1)) {
			socktype = duk_require_int(ctx, -1);
		}
		duk_pop(ctx);

		duk_get_prop_string(ctx, idx, "protocol");
		if (duk_is_number(ctx, -1)) {
			protocol = duk_require_int(ctx, -1);
		}
		duk_pop(ctx);
	}

	hints->ai_family = family;
    hints->ai_socktype = socktype;
    hints->ai_flags = flags;
    hints->ai_protocol = protocol;
    hints->ai_canonname = NULL;
    hints->ai_addr = NULL;
    hints->ai_next = NULL;
}

static void duk_push_addrinfo(duk_context *ctx, struct addrinfo *first) {
	struct addrinfo *cursor;
	struct sockaddr_in  *sockaddr_ipv4;
	duk_idx_t array_idx = duk_push_array(ctx);
	int i = 0;

	for(cursor = first; cursor != NULL; cursor = cursor->ai_next) {
		duk_push_object(ctx);

		duk_push_int(ctx, cursor->ai_flags);
		duk_put_prop_string(ctx, -2, "flags");

		duk_push_int(ctx, cursor->ai_family);
		duk_put_prop_string(ctx, -2, "family");

		duk_push_int(ctx, cursor->ai_socktype);
		duk_put_prop_string(ctx, -2, "socktype");

		duk_push_int(ctx, cursor->ai_protocol);
		duk_put_prop_string(ctx, -2, "protocol");

		duk_push_string(ctx, cursor->ai_canonname);
		duk_put_prop_string(ctx, -2, "canonname");

		if (cursor->ai_family == AF_INET) {
			sockaddr_ipv4 = (struct sockaddr_in *) cursor->ai_addr;
			duk_push_string(ctx, inet_ntoa(sockaddr_ipv4->sin_addr));
			duk_put_prop_string(ctx, -2, "addr");
		}

		duk_put_prop_index(ctx, array_idx, i++);
	}

}

static duk_ret_t duk_getaddrinfo(duk_context *ctx) {
	const char *host = duk_require_string(ctx, 0);
	const char *port = duk_require_string(ctx, 1);

	struct addrinfo hints;
	struct addrinfo *result = NULL;
	int retcode;

	memset(&hints, 0, sizeof(struct addrinfo));
	duk_get_addrinfo(ctx, &hints, 2);

	retcode = getaddrinfo(host, port, &hints, &result);
	if (retcode != 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not get address info (error code %d)", retcode);
        return -1;
	}

	duk_push_addrinfo(ctx, result);
	return 1;
}

static duk_ret_t duk_bind(duk_context *ctx) {
	SOCKET s;
	struct sockaddr_in server;

	s = (SOCKET) duk_require_pointer(ctx, 0);
	duk_get_sockaddr(ctx, &server);

	if( bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not bind socket (error code %d)", WSAGetLastError());
        return -1;
	}

	return 0;
}

static duk_ret_t duk_listen(duk_context *ctx) {
	SOCKET s;
	int backlog, retcode;

	s = (SOCKET) duk_require_pointer(ctx, 0);
	backlog = duk_require_int(ctx, 1);

	retcode = listen(s, backlog);
	if (retcode != 0) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not listen on socket (error code %d)", WSAGetLastError());
        return -1;
	}

	return 0;
}

static duk_ret_t duk_accept(duk_context *ctx) {
	SOCKET s, new_socket;
	struct sockaddr_in client;
	int c;

	s = (SOCKET) duk_require_pointer(ctx, 0);

	c = sizeof(struct sockaddr_in);
	new_socket = accept(s , (struct sockaddr *)&client, &c);
    if (new_socket == INVALID_SOCKET) {
    	duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not accept client socket (error code %d)", WSAGetLastError());
        return -1;
    }

    if (duk_is_object(ctx, 1)) {
    	duk_put_sockaddr(ctx, 1, &client);
    }

    duk_push_pointer(ctx, (void*) new_socket);
	return 1;
}

const duk_function_list_entry sock_functions [] = {
    { "socket", duk_socket, 3 },
    { "connect", duk_connect, 4 },
    { "inet_addr", duk_inetaddr, 1 },
    { "send", duk_send, 2 },
    { "recv", duk_recv, 1 },
    { "closesocket", duk_closesocket, 1 },
    { "getaddrinfo", duk_getaddrinfo, 3 },
    { "bind", duk_bind, 4 },
    { "listen", duk_listen, 2 },
    { "accept", duk_accept, 2 },
    { NULL, NULL, 0 }
};

// ----------------------------------------------------------------------------

static duk_ret_t sock_finalizer(duk_context *ctx) {
	WSACleanup();

	return 0;
}

static void sock_core(duk_context *ctx, duk_idx_t idx) {
	WSADATA wsa; /* will we need this? */

#ifdef DEBUG
	printf("loading Winsock2\n");
#endif

	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not startup Winsock2 (error code %d)", WSAGetLastError());
    }

    /* push WSACleanup function */
    duk_push_heap_stash(ctx);
    duk_push_object(ctx);
    duk_push_c_function(ctx, sock_finalizer, 1);
    duk_set_finalizer(ctx, -2);
    duk_put_prop_string(ctx, -2, "armornick_sock_wsa");
    duk_pop(ctx);

#ifdef DEBUG
    printf("registering Winsock2 functions\n");
#endif

    duk_put_function_list(ctx, idx, sock_functions);

#ifdef DEBUG
    printf("registering Winsock2 constants\n");
#endif

    if (idx == -1) {
    	idx = -2;
    }

    /* register address families */
    REGISTER_CONST(ctx, idx, AF_UNSPEC);
    REGISTER_CONST(ctx, idx, AF_INET);
    REGISTER_CONST(ctx, idx, AF_IPX);
    REGISTER_CONST(ctx, idx, AF_APPLETALK);
    REGISTER_CONST(ctx, idx, AF_NETBIOS);
    REGISTER_CONST(ctx, idx, AF_INET6);
    REGISTER_CONST(ctx, idx, AF_IRDA);
    REGISTER_CONST(ctx, idx, AF_BTH);

    /* register socket type specifications */
    REGISTER_CONST(ctx, idx, SOCK_STREAM);
    REGISTER_CONST(ctx, idx, SOCK_DGRAM);
    REGISTER_CONST(ctx, idx, SOCK_RAW);
    REGISTER_CONST(ctx, idx, SOCK_RDM);
    REGISTER_CONST(ctx, idx, SOCK_SEQPACKET);

    /* register socket protocols */
    REGISTER_CONST(ctx, idx, IPPROTO_ICMP);
    REGISTER_CONST(ctx, idx, IPPROTO_IGMP);
    // REGISTER_CONST(ctx, idx, BTHPROTO_RFCOMM);
    REGISTER_CONST(ctx, idx, IPPROTO_TCP);
    REGISTER_CONST(ctx, idx, IPPROTO_UDP);
    REGISTER_CONST(ctx, idx, IPPROTO_ICMPV6);
    // REGISTER_CONST(ctx, idx, IPPROTO_RM);

    /* inet_addr address constants */
    REGISTER_CONST(ctx, idx, INADDR_NONE);
    REGISTER_CONST(ctx, idx, INADDR_ANY);

    /* addrinfo flags */
    REGISTER_CONST(ctx, idx, AI_PASSIVE);
    REGISTER_CONST(ctx, idx, AI_CANONNAME);
    REGISTER_CONST(ctx, idx, AI_NUMERICHOST);
    REGISTER_CONST(ctx, idx, AI_ALL);
    REGISTER_CONST(ctx, idx, AI_V4MAPPED);
    REGISTER_CONST(ctx, idx, AI_NON_AUTHORITATIVE);
    REGISTER_CONST(ctx, idx, AI_SECURE);
    REGISTER_CONST(ctx, idx, AI_RETURN_PREFERRED_NAMES);
    // REGISTER_CONST(ctx, idx, AI_FQDN);
    // REGISTER_CONST(ctx, idx, AI_FILESERVER);
}

// ----------------------------------------------------------------------------


#ifdef BUILD_AS_DLL

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT duk_ret_t dukopen_sock(duk_context *ctx) {
	duk_idx_t idx = duk_push_object(ctx);

	sock_core(ctx, idx);

	return 1;
}

#else

void register_sock(duk_context *ctx) {
	duk_push_global_object(ctx);
	sock_core(ctx, -1);
}

#endif