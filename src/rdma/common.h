//
// Created by Alexey on 26/02/2020.
//

#ifndef RDMA_PCIE_PARSER_COMMON_H
#define RDMA_PCIE_PARSER_COMMON_H

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

/*****************************************************************************/
/**                             RPC data types                              **/
/*****************************************************************************/
/**
 * RPC message id
 * Edit this struct to add support for additional RPC commands
 */
typedef enum parserRpcType_e{
    PARSER_RPC_START = 0,
    PARSER_RPC_PAUSE,
    PARSER_RPC_STOP,
    PARSER_RPC_SET_INTERVAL,
    PARSER_RPC_INVALID,
}parserRpcType;

/**
 * RPC message metadata
 * Edit this struct to add additional metadata fields for new RPC commands
 */
typedef struct connectionData_s{
    uint64_t bufferAddr;
    uint32_t bufferRkey;
    uint32_t bufferLength;
    uint32_t streamingUSecInterval;
}connectionData_t;

/**
 * RPC message, this structure sent as RPC object
 */
typedef struct parserRpcMessage_s{
    parserRpcType status;
    connectionData_t connectionData;
}parserRpcMessage_t;

/*****************************************************************************/
/**                       RDMA stream data types                            **/
/*****************************************************************************/
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
 * Context RDMA handlers
 */
typedef struct parserRDMA_s{
    struct rdma_cm_id *id, *listen_id;
    struct ibv_mr *rpc_mr, *packet_mr;
    int send_flags;
    struct rdma_addrinfo hints, *res;
    struct ibv_qp_init_attr attr;
}parserRDMA_t;

/**
 * Main module context descriptor
 */
typedef struct parserContext_s{
    parserRpcMessage_t rpcMetadata;
    parserRpcMessage_t rpcLocal;
    parserData_t packetMetadata;
    serverInfo_t serverInfo;
    parserRDMA_t rdmaMetadata;
    LogicPacketsStream* stream;
}parserContext_t;


/*****************************************************************************/
/**                        Function declarations                            **/
/*****************************************************************************/
int parse_args(int argc, char** argv, serverInfo_t* serverInfo);

void print_header_common(void);


#endif //RDMA_PCIE_PARSER_COMMON_H
