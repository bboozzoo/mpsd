#ifndef __CONF_H__
#define __CONF_H__
#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "list.h"

/**
 * @brief configuation element type
 */
typedef enum {
    CONF_TYPE_INT = 1,
    CONF_TYPE_STRING,
    CONF_TYPE_MAX,
} conf_element_type_t;

/** 
 * @brief desc. of conf. element that is expected
 * to be found in configuration file
 */
struct conf_element_s {
    char * name;
    uint32_t id;
    conf_element_type_t type;
};
#define CONF_ELEMENT_END {NULL, 0, 0}

/**
 * @brief configuration element value, either int_val
 * or str_val should be used depending on type of given
 * element 
 */
typedef union {
    uint32_t    int_val;
    char *      str_val;
} conf_value_t;

/**
 * @brief configuration strings for given protocol
 * these will be looked for in section with matching name,
 * upon fiding the parameter, handler will be called with
 * value passed in union, and the configuration element as first 
 * parameter. The value passed needs to be copied in handler (ex. strings
 * need to be copied with strdup) as after parsing all data belongign 
 * to this group will be freed.
 * The list of configuration elements shall end with one empty element
 * that is all fields are 0
 */
struct conf_desc_s {
    char * name;
    int (*handler)(struct conf_element_s * s, conf_value_t val, void * data);
    struct conf_element_s * conf;
};

/**
 * @brief wrapper for section 
 */
struct conf_group_s {
    char * name;
    struct conf_desc_s * desc;  /**< @brief reference to possible desc if later 
                                  matching one is registered by some plugin */
    struct list_head_s values;
    struct list_head_s groups_list;
};

/**
 * @brief
 * wrapper for tuples id = value within one config section
 */
struct conf_value_s {
    char * name;
    struct conf_element_s * elem;   /**< @brief reference to conf. element if
                                      matching happens to be found during plugin registration */
    conf_value_t value;
    struct list_head_s values_list;
};

/**
 * @brief 
 * wrapper for configuration
 */
struct conf_s {
    FILE * source;
    struct list_head_s groups;
};
#define INIT_CONF_S {NULL}

/**
 * @brief reload config file
 * @return true or false
 */
int conf_load(struct conf_s * c);

/**
 * @brief release configuration structure
 * should be called at exit
 * @return none
 */
int conf_unload(struct conf_s * c);

/**
 * @brief create config section with given section
 * name
 */
struct conf_group_s * conf_create_section(struct conf_s * c, char * name);

/**
 * @brief create value wrapper 
 */
struct conf_value_s * conf_create_value(struct conf_group_s * c, char * name);

/**
 * @brief store string value in config element, if a string is already exists,
 * the new value will be added to the old one
 */
void conf_value_add_str(struct conf_value_s * c, const char * strval);
 
/**
 * @brief store int value in config element
 */
void conf_value_set_int(struct conf_value_s * c, int intval);

/**
 * @brief register module within config, and initialte fetching of parameters 
 */
int conf_register(struct conf_s * conf, struct conf_desc_s * cdesc, void * data);

#endif /* __CONF_H__ */

