#ifndef __SCAN_H__
#define __SCAN_H__
#include "conf.h"
#include <stdio.h>
/**
 * @brief call this function to start parsing the config file
 */
extern int lexscan(struct conf_s * s, FILE * src);

#endif /* __SCAN_H__ */
