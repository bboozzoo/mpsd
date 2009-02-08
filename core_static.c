#include "config.h"
#ifdef DYNAMIC_PLUGINS
#error this file should not be build with dynamic plugins enabled
#endif 

#include "tftp.h"
#include "core_static.h"

struct core_mod_static_s __static_mods[] = {
#ifdef MODULE_TFTP 
    { "tftp", {tftp_init, tftp_finalize}},
#endif
    {NULL, {NULL, NULL}}
};

