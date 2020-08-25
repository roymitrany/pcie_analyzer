//
// Created by Alexey on 26/02/2020.
//

#ifndef PCIE_PARSER_COMMON_H
#define PCIE_PARSER_COMMON_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <stdio.h>
#include "../parser/include/LogicPacketsStream.h"

/*****************************************************************************/
/**                              Defines                                    **/
/*****************************************************************************/
#define PARSER_SUCCESS 0
#define PARSER_ERROR 1

#define PARSER_PACKET_MAX_SIZE 1500
#define PARSER_STREAMING_USEC_DEFAULT_INTERVAL 2000

#define PARSER_DBG(...) do {     \
if(parser_debug){            \
printf("[DBG] ");        \
printf(__VA_ARGS__);     \
}                            \
} while (0)

#define PARSER_PACKET_PRINT(...) do {     \
printf(__VA_ARGS__);     \
} while (0)

#define PARSER_PRINT(...) do { \
printf(__VA_ARGS__);       \
} while(0)

/*****************************************************************************/
/**                                Externs                                  **/
/*****************************************************************************/
extern int parser_debug;

/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace pcie;

/*****************************************************************************/
/**                            Type defenitions                             **/
/*****************************************************************************/
typedef char uint8;

/**
 * Streamed packet structure
 */
typedef struct parserData_s{
    uint8 data[PARSER_PACKET_MAX_SIZE];
}parserData_t;


/*****************************************************************************/
/**                Client-Server module context structures                  **/
/*****************************************************************************/
/**
 * Connection information
 */
typedef struct serverInfo_s {
    char *server;
    char *port;
}serverInfo_t;


/**
 * Main module context descriptor
 */
typedef struct parserContext_s{
    parserData_t packetMetadata;
    serverInfo_t serverInfo;
    uint32_t streamingUSecInterval;
    uint32_t status;
    LogicPacketsStream* stream;
}parserContext_t;


/*****************************************************************************/
/**                        Function declarations                            **/
/*****************************************************************************/
int parse_args(int argc, char** argv, serverInfo_t* serverInfo);

void print_header_common(void);


#endif //PCIE_PARSER_COMMON_H
