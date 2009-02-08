
%{
#include <math.h>
#include "conf.h"
#include "debug.h"
static int in_section = 0;
static int line_nr = 1;
static struct conf_s * conf = NULL;
static struct conf_group_s * last_section = NULL;
static struct conf_value_s * last_value = NULL;
%}

%s inside_section 

DIGIT    [0-9]
ID       [a-zA-Z0-9\-\._]+
SECT_START  \[
SECT_END    \]
EQUALS      =
%%

{DIGIT}+    {
                DBG(2, "integer: %s (%d)\n", yytext,
                        atoi( yytext ) );
                if (NULL == last_value) {
                    fprintf(stderr, "error: parsing line %d\n", line_nr);
                    return -1;
                }
                conf_value_set_int(last_value, atoi(yytext));
                DBG(1, "   %s -> %d\n", last_value->name, last_value->value.int_val);
                last_value = NULL;
            }

{SECT_START} {
                BEGIN(inside_section);
                DBG(2, "starting section\n");
                last_section = NULL;
            }

{SECT_END}  { 
                BEGIN(0);
                DBG(2,"ending section\n");
            }

{EQUALS}    {
                DBG(2, "equals: %s\n", yytext );
            }

<inside_section>{ID} {
                DBG(2, "identifier in section: %s\n", yytext);
                DBG(1, "[%s]\n");
                last_section = conf_create_section(conf, yytext);
            }

{ID}        {
                DBG(2, "identifier: %s\n", yytext );
                /* if last_section != null then we are already in section */
                if (NULL == last_section) {
                    fprintf(stderr, "error: identifier %s in line %d, not in section\n",
                                    yytext, line_nr);
                    return -1;
                }

                if (NULL == last_value) {
                    /* we are starting a new value element */
                    last_value = conf_create_value(last_section, yytext);
                } else {
                    DBG(2, "value: %s\n", yytext);
                    conf_value_add_str(last_value, yytext);
                    DBG(1, "   %s -> %s\n", last_value->name, last_value->value.str_val);
                    /*last_value = NULL; */
                }
            }

^#.*    /* eat up one-line comments */

<inside_section>[ ] {
                DBG(1,"space inside section name in line: %d\n", line_nr);
                return -1;
            }
[ \t]+          /* eat up whitespace */

\n          {
                line_nr++;
                last_value = NULL;
            }

.           {
                DBG(1,"error in line: %d\n", line_nr );
            }

%%
int yywrap() {
    return 1;
}

int lexscan(struct conf_s * c, FILE * src) {
    conf = c;
    yyin = src;
    yylex();
    yylex_destroy();
    return 0;
}

