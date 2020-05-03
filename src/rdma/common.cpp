//
// Created by Alexey on 26/02/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <getopt.h>
#include <string.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
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

/*****************************************************************************/
/**                        Function implementatios                          **/
/*****************************************************************************/
int parse_args(int argc, char** argv, serverInfo_t* serverInfo){
    int flag = 0;

    serverInfo->server = PARSER_DEFAULT_SERVER;
    serverInfo->port = PARSER_DEFAULT_PORT;

    while((flag = getopt(argc, argv, "m:s:p:")) != -1){
        switch(flag){
            case 's':
                serverInfo->server = optarg;
                break;
            case 'p':
                serverInfo->port = optarg;
                break;
            default:
                PARSER_DBG("Unsupported argument\n");
                return PARSER_ERROR;
        }
    }

    return PARSER_SUCCESS;
}

void print_header_common(void){
    PARSER_PRINT("Alexey Tusov, tusovalexey@gmail.com\nIrina Gorodovskaya, ir.gorod@gmail.com\n");
    PARSER_PRINT("Protocol meta-data parser over RDMA\n");
}
