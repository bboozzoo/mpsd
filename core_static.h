#ifndef __CORE_STATIC_H__
#define __CORE_STATIC_H__
#include "module.h"
#include "tftp.h"

struct core_mod_static_s {
    char * name;
    struct module_s mod;
};

static struct core_mod_static_s mods[] = {
    { "tftp", {tftp_init, tftp_finalize}},
    {NULL, {NULL, NULL}}
};

#endif /* __CORE_STATIC_H__ */
