#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include "sock.h"
#include <stddef.h>

/**
 * @brief protocol description. The configuration is passed as conf parameter.
 * Callbacks handle proper events on given socket
 */
struct protocol_s {
    int (*conn)(struct sock_s * s); /* connect */
};

struct protocol_s * protocol_alloc(void * priv);
#endif /* __PROTOCOL_H__ */
