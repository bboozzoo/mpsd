#include <stddef.h>
#include "tftp.h"
#include "conf.h"
#include "debug.h"

typedef enum {
    ID_PORT,
    ID_HOST,
} conf_ids_t;

static struct conf_element_s tftp_conf_els[] = {
    {"port", ID_PORT, CONF_TYPE_INT},
    {"host", ID_HOST, CONF_TYPE_STRING},
    CONF_ELEMENT_END
};

static struct conf_desc_s tftp_conf = {
    .name = "tftp",
    .conf = tftp_conf_els,
    .handler = NULL
};

void tftp_init(void) {
    DBG(1, "tftp init\n");
}

void tftp_finalize(void) {
    DBG(1, "tftp finalize\n");
}

