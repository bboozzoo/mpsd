#ifndef __CORE_STATIC_H__
#define __CORE_STATIC_H__
#include "module.h"
#include "tftp.h"
#include "config.h"

struct core_mod_static_s {
    char * name;
    struct module_s mod;
};

static struct core_mod_static_s mods[] = {
#ifdef MODULE_TFTP 
    { "tftp", {tftp_init, tftp_finalize}},
#endif
    {NULL, {NULL, NULL}}
};

#endif /* __CORE_STATIC_H__ */
