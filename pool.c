#include <pool.h>

/**
 * initialise connection pool
 */
pool_res_t
pool_init(pool_t * pool) {
	DBG_LEAK_ENTER00;
	pool_res_t ret = POOL_ERR;

	if (pool != NULL) {
		memset(pool, 0, sizeof(pool_t));
		FD_ZERO(&pool->read_fds);
		FD_ZERO(&pool->write_fds);
		FD_ZERO(&pool->err_fds);
		pool->tv.tv_sec = POOL_DEF_STMOUT;
		pool->tv.tv_usec = POOL_DEF_UTMOUT;
		ret = POOL_OK;	
	}

	DBG_LEAK_LEAVE("returning: %d\n", ret);
	return ret;
}

/**
 * free connection pool
 */
void
pool_free(pool_t * pool) {
	DBG_LEAK_ENTER00;

	if (pool != NULL) {
		// if list is present
		// then it needs to be deallocated
		if (pool->pool_start != NULL) {
			struct list_en * entry = pool->pool_start;
			DBG_LEAK("starting at: 0x%x\n", PTR_TO_UINT(entry));
			while((entry = list_remove(entry, NULL)) != NULL) {
				DBG_LEAK("next entry: 0x%x\n", PTR_TO_UINT(entry));
			}
		}
	}
	DBG_LEAK_LEAVE00;
}

/**
 * add connection to the pool
 */
pool_res_t
pool_add(pool_t * pool, conn_t * conn) {
	pool_res_t retval = POOL_ERR;
	DBG_LEAK_ENTER("pool: 0x%x conn: 0x%x\n", 
			PTR_TO_UINT(pool),
			PTR_TO_UINT(conn));
	if ((pool != NULL) && (conn != NULL)) {
		struct list_en * conn_entry = list_append(pool->pool_start, conn);
		if (conn_entry != NULL) {
			retval = POOL_OK;
			/* fix the beginning of a pool */
			if (pool->pool_start == NULL) {
				DBG_LEAK0("first entry in pool\n");
				pool->pool_start = conn_entry;
			}
			/* check if current file descriptor is 
			 * > than current max, if so fix max */
			if (conn->socket_fd > pool->max_fd) {
				DBG_LEAK("updating max_fd, old = %d, new = %d\n",
						 pool->max_fd,
						 conn->socket_fd);
				pool->max_fd = conn->socket_fd;
			}
			/* add file descriptor to read and error sets
			 * to ad connection to write ready set
			 * use @see POOL_check_write() */
			FD_SET(conn->socket_fd, &pool->read_fds);
			//FD_SET(conn->socket_fd, &pool->write_fds);
			FD_SET(conn->socket_fd, &pool->err_fds);
		}
	}
	DBG_LEAK_LEAVE("returning: %d\n", retval);
	return retval;
}

/**
 * remove connection from pool
 */
pool_res_t
pool_remove(pool_t * pool, conn_t * conn) {
	DBG_LEAK_ENTER00;
	if (pool != NULL) {
		int conn_fd = conn->socket_fd;
		/* check if current iterator points to conn
		 * if so decrement it so that POOL_it_next will
		 * return entry subsequent to this one */
		if (list_data(pool->pool_iterator) == conn) {
			pool->pool_iterator = list_prev(pool->pool_iterator);
		}
	
		/* just in case save the pointer to data associated
		 * with pool start */	
		void * start_data = list_data(pool->pool_start);
		/* remove returns next entry, NULL if none available */
	   	list_en_t * len = list_remove(pool->pool_start, conn);
		/* if the removed entry was at the start of the pool
		 * fix the pool_start element */
		if (start_data == conn) {
			DBG_LEAK("old start: 0x%x new start: 0x%x\n",
					 PTR_TO_UINT(pool->pool_start),
					 PTR_TO_UINT(len));
			pool->pool_start = len;
		}

		if (conn_fd > pool->max_fd) { 
			/* pool max fd is lower than current connection fd,
			 * should not happen at all (issue warning) */
			DBG_WARN("pool max_fd (%d) < conn serv_socke (%d)\n", 
				   pool->max_fd,	
				   conn->socket_fd);
		} else if (conn_fd == pool->max_fd) {
			/* need to find anther maximum fd now
			 */
			DBG_LEAK("pool old max fd: %d\n", pool->max_fd);
			list_en_t * en = pool->pool_start;
			int other_max_fd = 0;
			for (; en != NULL; en = list_next(en)) {
				if (((conn_t *)en->data)->socket_fd > other_max_fd) 
					other_max_fd = ((conn_t *)en->data)->socket_fd;
			}
			if (other_max_fd == 0) {
				DBG_WARN0("removing last conn from pool\n");
				/* we did not find anything, means it was the last one? */
				pool->max_fd = 0;
			} else {
				DBG_LEAK("pool, new max: %d\n", other_max_fd);
				pool->max_fd = other_max_fd;
			}
		} else {
			/* everything is ok, connection fd is below pool max
			 * no need for changes */
		}
		/* remove fd from fd sets */
		FD_CLR(conn_fd, &pool->read_fds);
		FD_CLR(conn_fd, &pool->write_fds);
		FD_CLR(conn_fd, &pool->err_fds);

	}
	DBG_LEAK_LEAVE00;
	return POOL_OK;
}

/**
 * return next connection in pool
 */
conn_t *
pool_it_next(pool_t * pool) {
	conn_t * retval = NULL;
	DBG_LEAK_ENTER("pool: 0x%x\n", PTR_TO_UINT(pool));
	if ((pool != NULL) && (pool->pool_iterator != NULL)) {
		pool->pool_iterator = list_next(pool->pool_iterator);
		retval = list_data(pool->pool_iterator);
	}
	DBG_LEAK_LEAVE("retuning: 0x%x\n", PTR_TO_UINT(retval));
	return retval;
}

/**
 * return first connection in pool
 */
conn_t * 
pool_it_begin(pool_t * pool) {
	conn_t * retval = NULL;
	DBG_LEAK_ENTER("pool: 0x%x\n", PTR_TO_UINT(pool));
	if (pool != NULL) {
		pool->pool_iterator = pool->pool_start;
		if (pool->pool_start != NULL) {
			DBG_LEAK0("assigning entry\n");
			retval = list_data(pool->pool_start);
		}
	}
	DBG_LEAK_LEAVE("returning: 0x%x\n", PTR_TO_UINT(retval));
	return retval;
}

/**
 * check for pending data for read/write in the pool
 * return the number of connections that have something 
 * going on them, 
 * -1 means an error or interruption
 */
int 
pool_poll(pool_t * pool) {
	DBG_LEAK_ENTER("pool: 0x%x\n", PTR_TO_UINT(pool));
	unsigned int ready_num = 0;
	if (pool != NULL) {
		DBG_LEAK("timeout: %ds %dus\n", 
				 (unsigned int) pool->tv.tv_sec, 
				 (unsigned int)pool->tv.tv_usec);
		DBG_LEAK0("attempting select\n");
		struct timeval tv = pool->tv;
		pool->sel_read_fds = pool->read_fds;
		pool->sel_write_fds = pool->write_fds;
		pool->sel_err_fds = pool->err_fds;
		/* try select */
		ready_num = select(	pool->max_fd + 1, 
							&pool->sel_read_fds,
							&pool->sel_write_fds,
							&pool->sel_err_fds,
							&tv);

		switch (ready_num) {
			case -1: /* error */
				if (errno == EINTR) {
					ready_num = POOL_INTR;
				} else {
					ready_num = POOL_ERR;
				}
				DBG_ERROR("select error: %s\n", strerror(errno));
				break;
			case 0: /* nothing special */
				DBG_LEAK0("no activity\n");
				break;
			default:
				DBG_LEAK("activity on %d sockets\n", ready_num);
				break;
		}
	}
	DBG_LEAK_LEAVE("ready connections: %d\n", ready_num);
	return ready_num;
}

/**
 * check connection status in pool
 * will return pool_status_t value ORed for READ|WRITE|ERR or NONE
 */
pool_status_t 
pool_conn_status(pool_t *pool, conn_t *conn) {
	DBG_LEAK_ENTER00;
	pool_status_t status = POOL_ST_NONE;
	if (pool != NULL && conn != NULL) {
		if (conn->socket_fd <= pool->max_fd) {
			if (FD_ISSET(conn->socket_fd, &pool->sel_read_fds))
				status |= POOL_ST_READ;
			if (FD_ISSET(conn->socket_fd, &pool->sel_write_fds))
				status |= POOL_ST_WRITE;
			if (FD_ISSET(conn->socket_fd, &pool->sel_err_fds))
				status |= POOL_ST_ERR;
		} else {
			DBG_WARN("connection: fd (%d) > max_fd (%d)\n", conn->socket_fd, pool->max_fd);
		}
	}
	DBG_LEAK_LEAVE("returning: 0%03o\n", status);
	return status;
}

/**
 * mark given connection to be checked for write ready
 * or remove it from checking set
 * returns POOL_ERR if error occured
 */
pool_res_t 
pool_check_write(pool_t * pool, conn_t * conn, bool check) {
	DBG_LEAK_ENTER("connection 0x%x, check: %d\n", PTR_TO_UINT(conn), check);
	pool_res_t ret = POOL_ERR;
	if ((pool != NULL) && (conn != NULL)) {
		if (conn->socket_fd <= pool->max_fd) {
			if (check) {
				FD_SET(conn->socket_fd, &pool->write_fds);
			} else {
				FD_CLR(conn->socket_fd, &pool->write_fds);
			}
				ret = POOL_OK;
		}
	}
	DBG_LEAK_LEAVE00;
	return ret;
}

