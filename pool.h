#ifndef __POOL_H__
#define __POOL_H__

#include <conn.h>
#include <list.h>
#include <debug.h>
#include <sys/select.h>

#define POOL_DEF_STMOUT	1
#define POOL_DEF_UTMOUT 500000

/**
 * main connection pool structure
 */
typedef struct {
	struct list_en *	pool_start;
	struct list_en *  	pool_iterator;
	int 				max_fd;			/* maximum fd in pool */
	fd_set				read_fds;		/* read fd_set for whole pool */
	fd_set				write_fds;		/* write ... */
	fd_set				err_fds;		/* error .. */
	fd_set				sel_read_fds;	/* read fd_set, modified after POOL_poll */
	fd_set				sel_write_fds;	/* write ... */
	fd_set				sel_err_fds;	/* error ... */
	struct timeval		tv;				/* timeout for select */
} pool_t;

/**
 * pool return status
 */
typedef enum {
	POOL_INTR = -2,
	POOL_ERR = -1,
	POOL_OK = 0
} pool_res_t;

/** 
 * connection status as returned by POOL_conn_status
 * value can be ORed
 */
typedef enum {
	POOL_ST_NONE 	= 0000,		/* none */
	POOL_ST_READ 	= 0001,		/* ready to read */
	POOL_ST_WRITE 	= 0002,		/* ready to write */
	POOL_ST_ERR		= 0004		/* got some error */
} pool_status_t;

/* coneniency macro */
#define POOL_EMPTY	{ 							\
			.pool_start 	= NULL,				\
			.pool_iterator 	= NULL,				\
			.max_fd 		= 0,				\
			.read_fds 		= 0,				\
			.write_fds 		= 0,				\
			.err_fds 		= 0,				\
			.sel_read_fds 	= 0,				\
			.sel_write_fds	= 0,				\
			.sel_err_fds 	= 0,				\
			.tv = {								\
				.tv_sec 	= POOL_DEF_STMOUT,	\
				.tv_usec 	= POOL_DEF_UTMOUT	\
			}									\
		}

/* initialise pool */
pool_res_t pool_init(pool_t *p);
/* free pool */
void pool_free(pool_t *p);
/* init iteration */
conn_t * pool_it_begin(pool_t *p);
/* get next connection */
conn_t * pool_it_next(pool_t *p);
// get next connection with data ready to read
// conn_t * pool_it_next_read(pool_t *p);
// get next connection with write ready
// conn_t * pool_it_next_write(pool_t *p);
// get next connection with err
// conn_t * pool_it_next_err(pool_t *p);

/* mark connection for write ready check */
pool_res_t pool_check_write(pool_t *p, conn_t * conn, bool check);
/* check connection status */
pool_status_t pool_conn_status(pool_t *p, conn_t * conn);
/* add connection */
pool_res_t pool_add(pool_t *p, conn_t *c);
/*/ remove connection from pool */
pool_res_t pool_remove(pool_t *p, conn_t *c);
/* poll the connections in pool */
int pool_poll(pool_t *p);


#endif /* __POOL_H__ */
