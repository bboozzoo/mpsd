#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "conf.h"
#include "debug.h"
#include "scan.h"

int conf_load(struct conf_s * c) {
    DBG(1, "loading config\n");
    if (c->source != NULL) {
        FILE * conf_file = fopen(c->source, "r");
        if (conf_file != NULL) {
            list_init(&c->groups, NULL);
            lexscan(c, conf_file);
            fclose(conf_file);
            return RET_OK;
        } else {
            DBG(1, "error loading config file: %s\n", strerror(errno));
        }
    } 
    return RET_ERR;
}

int conf_unload(struct conf_s * c) {
    DBG(1, "config cleaning up\n");
    conf_release_left(c);
    if (NULL == c)
        return -1;
    if (c->source != NULL)
        free(c->source);
    return 0;
}

void conf_free_value(struct conf_value_s * val) {
    DBG(1, "free value: %p\n", val);
    if (val->detected_type == CONF_TYPE_STRING) {
        DBG(1, "\t\tstring: %s\n", val->value.str_val);
        free(val->value.str_val);
    } else {
        DBG(1, "\t\tint: %d\n", val->value.int_val);
    }
    free(val->name);
    free(val);
}

void conf_free_group(struct conf_group_s * group) {
    DBG(1, "free group: %p\n", group);
    free(group->name);
    free(group);
}

void conf_release_left(struct conf_s * c) {
    struct list_head_s * iter;
    DBG(1, "release groups that are still left\n");
    list_for(&c->groups, iter) {
        struct conf_group_s * group = LIST_DATA(iter, struct conf_group_s);
        struct list_head_s * value_iter;
        struct list_head_s * tmp_grp;
        DBG(1, "group: %s desc: %p\n", group->name, group->desc);
        list_for(&group->values, value_iter) {
            struct conf_value_s * val = LIST_DATA(value_iter, struct conf_value_s);
            struct list_head_s * tmp;
            DBG(1, "\tvalue: %s\n", val->name);
            DBG(1, "\telem: %p\n", val->elem);
            DBG(1, "\ttype: %s\n", (val->detected_type == CONF_TYPE_INT ) ? "int" : "string");
            tmp = value_iter;
            value_iter = value_iter->prev;
            list_del(tmp);
            conf_free_value(val);
        }
        tmp_grp = iter;
        iter = iter->prev;
        list_del(tmp_grp);
        conf_free_group(group);
    }
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
    else
        c->detected_type = CONF_TYPE_STRING;
}
 
void conf_value_set_int(struct conf_value_s * c, int intval) {
    if (NULL == c)
        return;

    c->detected_type = CONF_TYPE_INT; 
    c->value.int_val = intval;
}

/*
 * after a certain group has been identified and values have been passed
 * to the handler, all data will be freed
 */
int conf_register(struct conf_s * conf, struct conf_desc_s * cdesc, void * data) {
    struct list_head_s * iter = NULL;
    struct conf_group_s * grp = NULL;
    struct conf_element_s * el = NULL;

    if (NULL == conf || NULL == cdesc || 
        NULL == cdesc->name || NULL == cdesc->conf ||
        NULL == cdesc->handler) {
        DBG(1, "bad parameters passed for registration\n");
        return -1;
    }
    
    DBG(1,"[%s]\n", cdesc->name);
    for (el = cdesc->conf; el->name != NULL; el++)
        DBG(1, "   -> %s\n", el->name);

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
            struct conf_element_s * e = NULL;
            struct conf_value_s * val = LIST_DATA(iter, struct conf_value_s);
            struct list_head_s * tmp;

            DBG(1, "checking value: %s\n", val->name);
            for (e = cdesc->conf; e->name != NULL; e++) {
                DBG(1, "++checking value: %s\n", val->name);
                if (0 == strcmp(e->name, val->name)) {
                    if (e->type == val->detected_type) 
                        cdesc->handler(e, val->value, data);
                    else {
                        DBG(1, "warning: detected type != expected type\n");
                    }
                }
            }
            tmp = iter;
            iter = iter->prev;
            list_del(tmp);
            conf_free_value(val);
        }
        /* release group info */
        list_del(&grp->groups_list);
        conf_free_group(grp);
    }
    return 0;
}


