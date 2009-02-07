/*#include "config.h"*/
#include <stddef.h>
#include "conf.h"
#include "debug.h"
#include "core.h"

/* local types */
#define BLOCK_SIZE 512
#define DEFAULT_PORT 69
#define DEFAULT_HOST "localhost"

typedef enum {
    ID_PORT,
    ID_HOST,
} conf_ids_t;

/* local functions */
static int tftp_handler(struct conf_element_s * s, conf_value_t val, void * data);
/* local data */
static struct conf_element_s tftp_conf_els[] = {
    {"port", ID_PORT, CONF_TYPE_INT},
    {"host", ID_HOST, CONF_TYPE_STRING},
    CONF_ELEMENT_END
};

static struct conf_desc_s tftp_conf = {
    .name = "tftp",
    .conf = tftp_conf_els,
    .handler = tftp_handler
};

/* functions */

static int tftp_handler(struct conf_element_s * s, conf_value_t val, void * data) {
    int ret = 0;
    DBG(1, "found element of id %d\n", s->id);
    switch (s->id) {
        case ID_PORT:
            DBG(1, "got port: %d\n", val.int_val);
            break;
        case ID_HOST:
            DBG(1, "got host: %s\n", val.str_val);
            break;
        default:
            ret = RET_ERR;
    }
    return ret;
}

void tftp_init(struct conf_s * conf) {
    DBG(1, "tftp init\n");
    conf_register(conf, &tftp_conf, NULL);
}

void tftp_finalize(void) {
    DBG(1, "tftp finalize\n");
}

#ifdef CORE_LOAD_DYNAMIC
struct module_s * init(void) {
    struct module_s * mod = module_alloc();
    if (NULL != mod) {
        mod->init = tftp_init;
        mod->finalize = tftp_finalize;
    }
    return mod;
}
#endif 

