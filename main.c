//#include "sock.h"
#include "conf.h"
#include "core.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

/* local functions */
static void show_help(const char * progname);
/* local data */
static struct conf_s conf = INIT_CONF_S;

int main(int argc, char * argv[]) {
    int verbose = 0;
    int level = 0;
    FILE * dest = stderr;
    char optchar = -1;
    while (optchar = getopt(argc, argv, "c:vd:e:h")) {
        if (-1 == optchar)
            break;
        switch (optchar) {
            case 'c':
              conf.source = strdup(optarg); 
              if (NULL == conf.source) 
                  fprintf(stderr, "cannot open config file: %s\n", optarg);
              break;
            case 'v':
              verbose = 1;
              break;
            case 'e':
                if (0 == strcmp(optarg, "stderr"))
                    dest = stderr;
                else if (0 == strcmp(optarg, "stdout"))
                    dest = stdout;
                else {
                    FILE * out = fopen(optarg, "r");
                    if (NULL == out) {
                        fprintf(stderr, "failed to open destination for debug, using stderr\n");
                        dest = stderr;
                    } else
                        dest = out;
                }
                break;
            case 'd':
                level = atoi(optarg);
                break;
            case 'h':
                show_help(argv[0]);
                exit(1);
                break;
            default:
                break;
        }
    }

    if (NULL == conf.source) {
        fprintf(stderr, "no config file in use\n");
        exit(1);
    }

    dbg_init(dest, level, verbose);  

    DBG(1, "mpsd startup\n");
    if (RET_OK == conf_load(&conf)) {
        core_init(&conf);
        core_deinit(&conf);
        DBG(1, "mpsd closing\n");
    }
    conf_unload(&conf);
    return 0;
}

void show_help(const char * progname) {
    printf("usage: %s [options]\n"
           "where options are:\n"
           "    -h          help\n"
           "    -c  <file>  use <file> as configuration\n"
           "    -d  <int>   debug level\n"
           "    -e  <dest>  debug text destination:\n"
           "                can be: stdout|stderr|file name\n"
           "    -v          verbose debug\n", progname);
}

