#ifndef __CORE_STATIC_H__
#define __CORE_STATIC_H__

#include "config.h"

#ifdef DYNAMIC_PLUGINS
#error this file should not be included when dynamic plugins are enabled
#endif 

#include "module.h"

struct core_mod_static_s {
    char * name;
    struct module_s mod;
};

extern struct core_mod_static_s __static_mods[];

#endif /* __CORE_STATIC_H__ */
