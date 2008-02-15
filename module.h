#ifndef __MODULE_H__
#define __MODULE_H__
#include "core.h"
/**
 * @brief structure describing module
 */
struct module_s {
    void (*init) (void);
    void (*finalize) (void);
};

#endif /* __MODULE_H__ */
