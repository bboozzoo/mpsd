#ifndef __CONN_H__
#define __CONN_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

/************** GLOBAL PARAMS *******************/
#define CONN_QUEUE_LEN 5

/**
 * socket type identifier
 */
typedef enum {
	CONN_T_CLIENT,
	CONN_T_SERVER
} conn_type_t;

/**
 * connection object
 * sock_addr - pointer to appropriate sockaddr_* for this domain
 * sock_size - size of sockaddr_* structure
 */
typedef struct {
	int 			socket_fd;
	unsigned short 	port;
	int 			domain;
	void * 			sock_addr;
	socklen_t 		sock_size;
	conn_type_t		type;
} conn_t;

/* some general macros */
#define CONN_EMPTY { 						\
					 .socket_fd = 0, 		\
					 .port = 0, 			\
					 .domain = 0, 			\
					 .sock_addr = NULL, 	\
					 .sock_size = 0,		\
					 .type = CONN_T_CLIENT	\
					}

#define CONN_EMPTY_SERVER { 				\
					 .socket_fd = 0, 		\
					 .port = 0, 			\
					 .domain = 0, 			\
					 .sock_addr = NULL, 	\
					 .sock_size = 0,		\
					 .type = CONN_T_SERVER	\
					} 
/**
 * connection module errno value
 */
extern int conn_errno;

/**
 * connection return type
 */
typedef enum {
	CONN_OK = 0,
	CONN_ERR = -1
} conn_res_t;

/* initialisation function */
conn_res_t conn_server_init(conn_t * conn, unsigned short port, char * addr, int domain);
/* accept connection on server socket */
conn_res_t conn_accept(conn_t * server_socket, conn_t * client_socket);
/* clean up connection object */
void conn_finish(conn_t * conn);
/* check if there is a pending connection request */
int conn_pending(conn_t * conn);

/**
 * macros to simplify stuff
 */
#define conn_server_init4(c, p, a) \
	conn_server_init(c, p, a, PF_INET)

#define conn_server_init6(c, p, a) \
	conn_server_init(c, p, a, PF_INET6)

#define conn_init_local(c, p, a) \
	conn_server_init(c, p, a, PF_LOCAL)

#endif /* __CONN_H__ */
