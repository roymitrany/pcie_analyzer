//
// Created by Alexey on 26/02/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <getopt.h>
#include <string.h>
#include "common.h"


/*****************************************************************************/
/**                              Defines                                    **/
/*****************************************************************************/
#define PARSER_DEFAULT_SERVER "0.0.0.0"
#define PARSER_DEFAULT_PORT "10000"


/*****************************************************************************/
/**                          Global variables                               **/
/*****************************************************************************/
#ifndef PARSER_DEBUG_ENABLE
int parser_debug = 0;
#else
int parser_debug = 1;
#endif


void print_header_common(void){
    PARSER_PRINT("Alexey Tusov, tusovalexey@gmail.com\nIrina Gorodovskaya, ir.gorod@gmail.com\n");
}
