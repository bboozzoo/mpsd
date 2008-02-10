#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include "sock.h"
#include "conf.h"
#include <stddef.h>


/**
 * @brief protocol description. The configuration is passed as conf parameter.
 * Callbacks handle proper events on given socket
 */
struct protocol_s {
    struct conf_desc_s * conf;
    int (*conn)(struct sock_s * s); /* connect */

};

#endif /* __PROTOCOL_H__ */
