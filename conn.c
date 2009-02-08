#include <conn.h>
#include <debug.h>

int conn_errno = 0;


static void __free_sockaddr(conn_t * conn);
static conn_res_t __init_sockaddr4(conn_t * conn, int family, unsigned short port, const char * addr);
static void __close_sock(conn_t * conn);
static conn_res_t __open_socket(conn_t * conn, int domain, int type);
static void __translate_address(conn_t * conn);

/**
 * initialise socket, bind, listen, etc...
 */
conn_res_t
conn_server_init(conn_t * conn, unsigned short port, char * addr, int domain) {
	DBG(1,"entering, port %d\n", port);
	conn_res_t retval = CONN_ERR;
	if (conn != NULL) {
		memset(conn, 0, sizeof(conn_t));
		/* mark connection type */
		conn->type = CONN_T_SERVER;	

		/* try to bind now */
		switch (domain) {
			case PF_INET: {
				DBG(1,"PF_INET requested\n");
				conn->domain = domain;
				conn->port = port;
				__init_sockaddr4(conn, AF_INET, port, addr);
				break;
			}
			case PF_INET6:
			case PF_LOCAL:
				DBG(1, "IPv6 and Unix sockets not supported yet\n");
				errno = ESOCKTNOSUPPORT;
				goto at_return;
				break;
		}

		/* create socket */
		if (__open_socket(conn, domain, SOCK_STREAM) != CONN_OK) {
			goto at_return;
		} 
		/* huston we have a socket
		 * try to bind now
		 * all params are set in conn structure */
		if (bind(conn->socket_fd, (struct sockaddr *) conn->sock_addr, conn->sock_size) == -1) {
			DBG(1, "bind failed: %s\n", strerror(errno));
			conn_errno = errno;
			__close_sock(conn);
			__free_sockaddr(conn);
			goto at_return;
		} else {
			DBG(1,"bound to socket\n");
		}
		/* and set listening state */
		if (listen(conn->socket_fd, CONN_QUEUE_LEN) == -1) {
			conn_errno = errno;
			DBG(1, "listen failed: %s\n", strerror(errno));
			__close_sock(conn);
			__free_sockaddr(conn);
			goto at_return;
		} else {
			DBG(1,"listening on socket\n");
			retval = CONN_OK;
		}
	}
at_return:
	DBG(1,"returning %s\n", ((retval == CONN_OK) ? "OK" : "ERROR"));
	return retval;
}

/**
 * open socket, domain should be passed as given in CONN_init..
 * type should be SOCK_STREAM, DGRAM not supported yet,
 * returned value CONN_ERR if socket() failed
 */
static conn_res_t 
__open_socket(conn_t * conn, int domain, int type) {
	conn_res_t ret = CONN_ERR;
    int sock;
	DBG(1,"open socket, domain: %d, type: %d\n", domain, type);
	sock = socket(domain, type, 0);
	if (sock == -1) {
		DBG(1, "socket opening failed: %s\n", strerror(errno));
		conn_errno = errno;
	} else {
		DBG(1,"opened socket, fd = %d\n", sock);
		conn->socket_fd = sock;
		ret = CONN_OK;
	}
	DBG(1,"returning: %d\n", ret);
	return ret;
}


/**
 * clean up connection object
 * free sockaddr structure
 */
void
conn_finish(conn_t * conn) {
	DBG(1,"finish called\n");
	__close_sock(conn);
	__free_sockaddr(conn);
	DBG(1,"finish ended\n");
}

/**
 * free sockaddr structure in connection object
 */
static void
__free_sockaddr(conn_t *conn) {
	DBG(1, "enter\n");
	if (conn != NULL) {
		if (conn->sock_addr != NULL) {
			DBG(1,"freeing mem\n");
			free(conn->sock_addr);
			conn->sock_addr = NULL;
		}
	}
	DBG(1, "exiting\n");
}

/**
 * close socket
 */
static void
_c_lose_sock(conn_t *conn) {
	DBG(1, "enter\n");
	if ((conn != NULL) && (conn->socket_fd != 0)) {
		DBG(1,"closing socket\n");
		if (conn->type == CONN_T_SERVER) {
			shutdown(conn->socket_fd, SHUT_RDWR);
		} else {
			close(conn->socket_fd);
		}
	}
	DBG(1, "exiting\n");
}

/**
 * initialise IPv4 connection
 */
static conn_res_t
__init_sockaddr4(conn_t * conn, int family, unsigned short port, const char * addr) {
	DBG(1, "enter\n");
	
	// if family other than AF_INET
	if (family != AF_INET) 
		return CONN_ERR;

	conn->sock_addr = calloc(1, sizeof(struct sockaddr_in));
	if (conn->sock_addr == NULL) {
		DBG(1, "failed to allocate sockaddr_in");
		return CONN_ERR;
	}
	DBG(1,"allocated sockaddr_in\n");
	struct sockaddr_in * saddr = (struct sockaddr_in *)conn->sock_addr;
	saddr->sin_family = AF_INET;
	/* check if user specified any address */
	if (addr != NULL) {
		DBG(1,"address %s\n", addr);
		/* resolve the address */
		struct hostent * bind_host = gethostbyname(addr);
		if (bind_host != NULL) {
            int i;
			/*
			 * TODO: use the stuff received here
			 */

			/* just debugging stuff, nothing special */
			DBG(1,"host name: %s\n", bind_host->h_name);
			for (i = 0; bind_host->h_aliases[i] != NULL; i++) {
				DBG(1,"alias: %s\n", bind_host->h_aliases[i]);
			}
			DBG(1,"addr type: %s\n", (bind_host->h_addrtype == AF_INET6) ? "IPv6" : "IPv4");
			DBG(1,"address: %s\n", inet_ntoa(*((struct in_addr *)bind_host->h_addr)));
		} else { /* address cannot be resolved */
			DBG(1, "gethostbyname failed: %s\n", strerror(h_errno));
			/* since the address cannot be resolved, listen on all
			 * available addresses */
			saddr->sin_addr.s_addr = INADDR_ANY;
		}
	} else {
		DBG(1,"addr INADDR_ANY\n");
		saddr->sin_addr.s_addr = INADDR_ANY;
	}
	DBG(1,"setting port %d\n", port);
	saddr->sin_port = htons(port);
	conn->sock_size = sizeof(struct sockaddr_in);

	DBG(1, "exiting\n");
	return CONN_OK;
}

/**
 * check if there are any pending connection requests
 * client should call @see CONN_accept next
 */
int
conn_pending(conn_t * conn) {
	DBG(1, "enter\n");
	int cli_socket = 0;

	if (conn != NULL) {
		fd_set read_fds;
		struct timeval tv = { .tv_sec = 0, .tv_usec = 500000 };

		FD_ZERO(&read_fds);
		FD_SET(conn->socket_fd, &read_fds);
		int sret = select(conn->socket_fd + 1, &read_fds, NULL, NULL, &tv);
		switch (sret) {
			case -1: /* error */
				DBG(1, "select error: %s\n", strerror(errno));
				conn_errno = errno;
				break;
			case 0: /* nothing special */
				DBG(1,"noone waiting\n");
				break;
			default:
				DBG(1,"connection attempt\n");
				cli_socket = sret;
				break;
		}
	}

	DBG(1,"pending connections: %d\n", cli_socket);
	return cli_socket;
}

/**
 * accept incoming connection on server socket
 * will initialise the client socket structure
 * returns CONN_ERR if error encountered
 */
conn_res_t
conn_accept(conn_t * server_socket, conn_t * client_socket) {
	conn_res_t ret = CONN_ERR;
	DBG(1, "enter\n");
	if ((server_socket != NULL) && (client_socket != NULL)) {
		if (server_socket->type != CONN_T_SERVER) {
			/* server socket is not server type,
			 * return error */
			DBG(1,"socket 0x%x is not a server socket, type: %d\n",
					PTR_TO_UINT(server_socket),
					server_socket->type);
			goto at_return;
		}
		
		if (client_socket->sock_addr != NULL) {
			/* deallocate already present sockaddr
			 * in client connection */
			DBG(1,"client sockaddr already present?, freeing\n");
			free(client_socket->sock_addr);
			client_socket->sock_addr = NULL;
			client_socket->sock_size = 0;
		}
		/* check protocol */
		switch(server_socket->domain) {
			case PF_INET: {
				/* IPv4 socket */
				client_socket->sock_size = sizeof(struct sockaddr_in);
				client_socket->sock_addr = calloc(1, sizeof(struct sockaddr_in));
				client_socket->domain = PF_INET;
				if (client_socket->sock_addr == NULL) {
					DBG(1, "failed to allocate sockaddr_in\n");
					goto at_return;
				}
				break;
			}
			case PF_INET6:
			case PF_LOCAL:
				DBG(1,"domain unsupported\n");
				break;
			default:
				break;
		}
		/* try accept */
		socklen_t size = client_socket->sock_size;
		int fd = accept(server_socket->socket_fd, 
					    client_socket->sock_addr, 
						&size);
		if (fd == -1) {
			DBG(1, "accept failed: %s\n", strerror(errno));
			free(client_socket->sock_addr);
			conn_errno = errno;
		} else {
			if (client_socket->sock_size != size) {
				DBG(1,"sock_size != size\n");
			}
			DBG(1,"accepted connection, fd = %d\n", fd);
			client_socket->port = ntohs(((struct sockaddr_in *)client_socket->sock_addr)->sin_port);
			client_socket->socket_fd = fd;
			__translate_address(client_socket);
			ret = CONN_OK;
		}
	}
at_return:
	DBG(1,"returning: %d\n", ret);
	return ret;
}

/**
 * translate and print sockaddr_ associated with conn object
 */
static void
__translate_address(conn_t * conn) {
	/* use buffer on stack */
	char buf[80] = {0};
	if (conn != NULL) {
		const char * ret = inet_ntop(conn->domain, 
							   &(((struct sockaddr_in *)conn->sock_addr)->sin_addr),
							   buf,
							   sizeof(buf));
		if (ret != NULL) {
			DBG(1,"address = %s\n", buf);
			DBG(1,"port = %d\n", conn->port);
		} else {
			DBG(1,"error translating address: %s\n", strerror(errno));
		}
	}
}

