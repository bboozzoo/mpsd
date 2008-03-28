#ifndef __MODULE_H__
#define __MODULE_H__
#include "core.h"
#include "conf.h"
#include "list.h"
/**
 * @brief structure describing module
 */
struct module_s {
    void (*init) (struct conf_s *);
    void (*finalize) (void);
    struct list_head_s mod_entry;
};

struct module_s * module_alloc(void);

#endif /* __MODULE_H__ */
