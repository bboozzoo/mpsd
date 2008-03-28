#ifndef __CORE_H__
#define __CORE_H__
#include "conf.h"
#include "protocol.h"
#include "module.h"

/**
 * @brief init core
 */
int core_init(struct conf_s * conf);

/**
 * @brief deinit core
 */
void core_deinit(struct conf_s * conf);

/**
 * @brief register new protocol
 */
int core_register_protocol(struct protocol_s * proto); 

#endif /* __CORE_H__ */
