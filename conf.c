#include <string.h>
#include <stdlib.h>
#include "conf.h"
#include "debug.h"
#include "scan.h"

int conf_load(struct conf_s * c) {
    DBG(1, "loading config\n");
    list_init(&c->groups, NULL);
    return lexscan(c);
}

int conf_unload(struct conf_s * c) {
    if (NULL == c)
        return -1;

    return 0;
}

struct conf_group_s * conf_create_section(struct conf_s * c, char * name) {
    struct conf_group_s * group = NULL;

    if (NULL == c || NULL == name)
        return NULL;

    group = calloc(1, sizeof(struct conf_group_s));
    if (NULL != group) {
        /* data of values == NULL */
        list_init(&group->values, NULL);
        /* data of list element in this group == this group */
        list_init(&group->groups_list, group);
        /* add it to global configuration */
        list_add_tail(&c->groups, &group->groups_list);

        group->desc = NULL;
        group->name = strdup(name);
        if (NULL == group->name) {
            DBG(1, "allocation of group failed\n");
            free(group);
            group = NULL;
        }
    } else {
        DBG(1, "allocation of group failed\n");
    }
    return group;
}

struct conf_value_s * conf_create_value(struct conf_group_s * c, char * name) {
    struct conf_value_s * val = NULL;

    if (NULL == c || NULL == name)
        return NULL;

    val = calloc(1, sizeof(struct conf_value_s));
    if (NULL != val) {
        list_init(&val->values_list, val);
        list_add_tail(&c->values, &val->values_list);

        val->elem = NULL;
        val->name = strdup(name);
        if (NULL == val->name) {
            DBG(1, "allocation of value failed\n");
            free(val);
            val = NULL;
        }
    } else {
        DBG(1, "allocation of value failed\n");
    }
    return val;
}

void conf_value_add_str(struct conf_value_s * c, const char * strval) {
    if (NULL == c || NULL == strval)
        return;
    
    if (NULL != c->value.str_val) {
        /* concatenate values */
        unsigned int reqspace = strlen(c->value.str_val);
        reqspace += 1; /* for ' ' */
        reqspace += strlen(c->value.str_val);
        reqspace += 1; /* for \0 */
        DBG(2, "new size: %d\n", reqspace);
        /* get mem */
        c->value.str_val = realloc(c->value.str_val, reqspace);
        if (NULL != c->value.str_val) {
            strcat(c->value.str_val, " ");
            strcat(c->value.str_val, strval);
            DBG(1, "new value: \'%s\'\n", c->value.str_val);
        }
    } else
        c->value.str_val = strdup(strval);

    if (NULL == c->value.str_val) 
        DBG(1, "allocation of value(str) failed\n");
}
 
void conf_value_set_int(struct conf_value_s * c, int intval) {
    if (NULL == c)
        return;

    c->value.int_val = intval;
}

/*
 * after a certain group has been identified and values have been passed
 * to the handler, all data will be freed
 */
int conf_register(struct conf_s * conf, struct conf_desc_s * cdesc, void * data) {
    struct list_head_s * iter = NULL;
    struct conf_group_s * grp = NULL;

    if (NULL == conf || NULL == cdesc || 
        NULL == cdesc->name || NULL == cdesc->conf ||
        NULL == cdesc->handler) {
        DBG(1, "bad parameters passed for registration\n");
        return -1;
    }
    
    DBG(1,"[%s]\n", cdesc->name);
    for (struct conf_element_s * e = cdesc->conf; e->name != NULL; e++)
        DBG(1, "   -> %s\n", e->name);

    list_for(&conf->groups, iter) {
        grp = LIST_DATA(iter, struct conf_group_s);
        if (0 == strcmp(grp->name, cdesc->name)) {
            DBG(1, "found matching group: %s\n", grp->name);
            grp->desc = cdesc;
            break;
        } else
            grp = NULL;
    }

    if (NULL != grp) {
        /* now call handler for each value that is found */
        list_for(&grp->values, iter) {
            struct conf_value_s * val = LIST_DATA(iter, struct conf_value_s);
            DBG(1, "checking value: %s\n", val->name);
            for (struct conf_element_s * el = cdesc->conf; el->name != NULL; el++) {
                DBG(1, "++checking value: %s\n", val->name);
                if (0 == strcmp(el->name, val->name)) {
                    cdesc->handler(el, val->value, data);
                }
            }
            struct list_head_s * i = iter;
            iter = iter->prev;
            list_del(i);
            free(val);
        }
        /* release group info */
        list_del(&grp->groups_list);
        free(grp);
    }
    return 0;
}


