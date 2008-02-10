#include "core.h"
#include "conf.h"
#include "debug.h"
#include <stdlib.h>

/* local types */
struct core_s {

};

/* IDs, local only */
typedef enum {
    ID_PLUGINS,
    ID_PLUGIN_DIR,
} core_ids_t;

/* local functions */
static int core_handler(struct conf_element_s * s, conf_value_t val, void * data);

/* local data */

/* conf. elements */
static struct conf_element_s core_conf_els[] = {
    {"plugins", ID_PLUGINS, CONF_TYPE_STRING},
    {"plugins-dir", ID_PLUGIN_DIR, CONF_TYPE_STRING},
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

    DBG(1, "got element, of id: %d, type: %d\n", s->id, s->type);
    if (CONF_TYPE_STRING == s->type) {
        DBG(1, "it is a string: \'%s\'\n", val.str_val);
    } else if (CONF_TYPE_INT == s->type) {
        DBG(1, "it is an int: %d\n", val.int_val);
    } else {
        ret_val = RET_ERR;
    }
    return ret_val;
}

int core_init(struct conf_s * conf) {
    core_ctx = calloc(1, sizeof(struct core_s));
    if (NULL == core_ctx)
        return -1;

    conf_register(conf, &core_conf, core_ctx);
    /* handler was called */

    return 0;
}

void core_deinit(struct conf_s * conf) {
   if (NULL != core_ctx)
       free(core_ctx);
}

