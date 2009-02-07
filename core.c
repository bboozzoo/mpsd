/*#include "config.h"*/
#include "core.h"
#include "conf.h"
#include "debug.h"
#include "module.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

#ifdef CORE_LOAD_DYNAMIC
#include <dlfcn.h>
#else /* CORE_LOAD_STATIC */
#include "core_static.h"
#endif 

/* local types */
struct core_s {
    char * modules_dir;
    char * modules;
    struct list_head_s modules_list;
    struct conf_s * conf;
};

/* IDs, local only */
typedef enum {
    ID_MODULES = 0,
    ID_MODULE_DIR,
} core_ids_t;

/* local functions */
static int core_handler(struct conf_element_s * s, conf_value_t val, void * data);
static int core_load_modules(struct core_s * core);
static void core_unload_modules(struct core_s * core);
static int core_register_module(struct core_s * core, struct module_s * mod);
#ifdef CORE_LOAD_DYNAMIC
static int __core_load_module_dynamic(struct core_s * core, const char * mod);
static void __core_unload_module_dynamic(struct core_s * core, struct module_s * mod); 
#else /* CORE_LOAD_STATIC */
static int __core_load_module_static(struct core_s * core, const char * mod);
static void __core_unload_module_static(struct core_s * core, struct module_s * mod);
#endif
/* local data */

/* conf. elements */
static struct conf_element_s core_conf_els[] = {
    {"modules", ID_MODULES, CONF_TYPE_STRING},
    {"modules-dir", ID_MODULE_DIR, CONF_TYPE_STRING},
    CONF_ELEMENT_END
};

/* config desc. of core */
static struct conf_desc_s core_conf = {
    .name = "core",
    .handler = core_handler,
    .conf = core_conf_els
};

/* core ctx */
static struct core_s * core_ctx = NULL;

/**
 * @brief handler for core related config calls 
 */
static int core_handler(struct conf_element_s * s, conf_value_t val, void * data) {
    int ret_val = 0;
    struct core_s * core_c = (struct core_s *) data;
    
    if (NULL == data)
        return -1;

    switch (s->id) {
        case ID_MODULE_DIR:
            core_c->modules_dir = strdup(val.str_val);
            break;
        case ID_MODULES:
            core_c->modules = strdup(val.str_val);
            break;
        default:
            ret_val = RET_ERR;
    }

    return ret_val;
}

int core_init(struct conf_s * conf) {
    core_ctx = calloc(1, sizeof(struct core_s));
    if (NULL == core_ctx)
        return -1;
    
    list_init(&core_ctx->modules_list, NULL);
    core_ctx->conf = conf;
    conf_register(conf, &core_conf, core_ctx);
    /* handler was called */
    if (NULL == core_ctx->modules || NULL == core_ctx->modules_dir) {
        DBG(1, "loading core config failed\n");
        return RET_ERR;
    }
    
    core_load_modules(core_ctx);
    return 0;
}

void core_deinit(struct conf_s * conf) {
   if (NULL == core_ctx)
       return;

   core_unload_modules(core_ctx);
   if (NULL != core_ctx->modules)
       free(core_ctx->modules);
   if (NULL != core_ctx->modules_dir)
       free(core_ctx->modules_dir);
   free(core_ctx);
}

static int core_load_modules(struct core_s * core) {
    DBG(2, "loading modules\n");
    int ret = 0;
    char * strstate = NULL;
    char * ostr = core->modules;
    ostr = strtok_r(ostr, " ", &strstate);
    if (NULL != ostr) {
        do {
            int val = 0;
            DBG(1, "module: %s\n", ostr);
#if defined(CORE_LOAD_DYNAMIC)
            val = __core_load_module_dynamic(ostr);
#else
            val = __core_load_module_static(core, ostr);
#endif            
            if (0 != val)
                DBG(1, "error loading module: %s\n", ostr);
        } while (NULL != (ostr = strtok_r(NULL, " ", &strstate)));
    } else {
        DBG(1, "extracting any value from string failed\n");
        ret = RET_ERR; 
    }
    return ret;
}

#if defined(CORE_LOAD_DYNAMIC)
static int __core_load_module_dynamic(struct core_s * core, const char * modname) {
    return 0;
}
static void __core_unload_module_dynamic(struct core_s * core, struct module_s * mod) {

}

#else /* CORE_LOAD_STATIC */
static int __core_load_module_static(struct core_s * core, const char * modname) {
    struct core_mod_static_s * mod = &mods;
    int ret = 0;
    while (NULL != mod->name) {
        if (0 == strcmp(mod->name, modname)) {
            struct module_s * m = &mod->mod;
            list_init(&m->mod_entry, m);
            DBG(1, "loading module: %s\n", mod->name); 
            core_register_module(core, m);
            m->init(core->conf);
            break;
        }
        mod++;
    }
    return ret;
}

static void __core_unload_module_static(struct core_s * core, struct module_s * mod) {
        mod->finalize();
}
#endif

static int core_register_module(struct core_s * core, struct module_s * mod) {
    list_add(&core->modules_list, &mod->mod_entry);
    return 0;
}

int core_register_protocol(struct protocol_s * proto) {
    return 0;
}

static void core_unload_modules(struct core_s * core) {
    struct list_head_s * it;
    DBG(1, "unloading modules\n");
    list_for(&core->modules_list, it) {
        struct module_s * mod = LIST_DATA(it, struct module_s);
        struct list_head_s * ldel = it;
        it = it->prev;
        list_del(ldel);
        __core_unload_module_static(core, mod);
    }
}

