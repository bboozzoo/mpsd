#include "module.h"
#include <stdlib.h>

struct module_s * module_alloc(void) {
    struct module_s * mod = calloc(1, sizeof(struct module_s));
    if (NULL != mod)
        list_init(&mod->mod_entry, mod);
    return mod;
}
