//
// Created by Alexey on 06/03/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"

/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace std;

/*****************************************************************************/
/**                            Type definitions                             **/
/*****************************************************************************/
/**
 * CLI command definitions
 */
typedef enum parserInteractiveMenu_e{
    RPC_START=0,
    RPC_PAUSE,
    RPC_CHANGE_INTERVAL,
    RPC_STOP
}parserInteractiveMenu;

/*****************************************************************************/
/**                            Static variables                             **/
/*****************************************************************************/
/**
 * Main parser context instance
 */
static parserContext_t parserContext;

/*****************************************************************************/
/**                            Static functions                             **/
/*****************************************************************************/
/**
 * Prints program information header
 * @param serverInfo Connection information
 */
static void print_header(serverInfo_t* serverInfo){
    if(NULL == serverInfo){
        return;
    }
    print_header_common();
    PARSER_PRINT("Starting in client mode\n");
    PARSER_PRINT("Server: %s, Port:%s\n", serverInfo->server, serverInfo->port);
}

/**
 * Aux interactive handler for interval value parsing
 * @param menuPtr RPC message
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int interactive_menu_parse_interval(parserRpcMessage_t* menuPtr){
    int result = 0;

    PARSER_PRINT("Enter new time interval in usec for packets streaming\n");
    result = scanf("%d", (int*)(&menuPtr->connectionData.streamingUSecInterval));
    if(result <= 0){
        PARSER_DBG("Failed to parse interval\n");
        scanf("%*s");
        return PARSER_ERROR;
    }

    return PARSER_SUCCESS;
}

/**
 * User CLI interface
 * @param menuPtr RPC message
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int interactive_menu(parserRpcMessage_t* rpcLocal){
    int result = 0;
    parserInteractiveMenu menuItem = RPC_START;
    PARSER_PRINT("============================================\n"
                 " Enter next command for RPC:\n"
                 "    %d - Start streaming meta-data packets\n"
                 "    %d - Pause streaming process\n"
                 "    %d - Change streaming interval\n"
                 "    %d - Stop streaming process and exit\n"
                 "============================================\n"
    , RPC_START, RPC_PAUSE, RPC_CHANGE_INTERVAL, RPC_STOP);


    result = scanf("%d", (int*)&menuItem);
    if(result <= 0){
        PARSER_DBG("Failed to parse selection\n");
        scanf("%*s"); // clear invalid char(s) from stdin
        return 0;
    }

    switch (menuItem){
        case RPC_START:
            rpcLocal->status = PARSER_RPC_START;
            break;
        case RPC_PAUSE:
            rpcLocal->status = PARSER_RPC_PAUSE;
            break;
        case RPC_CHANGE_INTERVAL:
            // Pause the streaming until user enters new value
            rpcLocal->status = PARSER_RPC_SET_INTERVAL;
            result = interactive_menu_parse_interval(rpcLocal);
            if(PARSER_SUCCESS != result){
                return 0;
            }
            rpcLocal->status = PARSER_RPC_START;
            break;
        case RPC_STOP:
            rpcLocal->status = PARSER_RPC_STOP;
            return 1;
        default:
            PARSER_DBG("Invalid RPC command\n");
    }

    return 0;
}

/**
 * CLI thread funtion
 * @param ptr RPC message
 * @return 0
 */
static void* thread_menu(void* ptr){
    parserRpcMessage_t* rpcLocal = (parserRpcMessage_t*)ptr;
    while(1){
        if(interactive_menu(rpcLocal)){
            return 0;
        }
    }
}

/**
 * RPC message send handler
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int rdma_send_rpc(parserContext_t* context){
    int result;
    struct ibv_wc wc;
    result = rdma_post_send(context->rdmaMetadata.id,
                            NULL,
                            &(context->rpcMetadata),
                            sizeof(parserRpcMessage_t),
                            context->rdmaMetadata.rpc_mr,
                            context->rdmaMetadata.send_flags);
    if(result){
        PARSER_DBG("Failed in rdma_post_send\n");
        return PARSER_ERROR;
    }

    result = rdma_get_send_comp(context->rdmaMetadata.id, &wc);
    if(result < 0){
        PARSER_DBG("Failed in rdma_get_send_comp\n");
        return PARSER_ERROR;
    }else if(result > 0){
        if(wc.status != 0){
            PARSER_DBG("Got send completion with status code %d\n", wc.status);
            return PARSER_ERROR;
        }
    }

    return PARSER_SUCCESS;
}

/**
 * Clients entry point to main loop after successful connection
 * Main process handles RPC message flow to server and nested thread handles
 * CLI user interface
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int main_loop(parserContext_t* context){
    pthread_t cliThread;
    struct ibv_wc wc;
    int result;

    // Reset CLI and RPC interface
    context->rpcMetadata.status = PARSER_RPC_INVALID;
    context->rpcLocal.status = PARSER_RPC_INVALID;

    // Start another thread for CLI
    result = pthread_create(&cliThread,
                            NULL,
                            thread_menu,
                            (void*)(&context->rpcLocal));
    if(result){
        PARSER_DBG("Failed to create CLI thread\n");
        return PARSER_ERROR;
    }

    PARSER_DBG("Starting main loop\n");
    while (1){
        if(context->rpcMetadata.status != context->rpcLocal.status){
            PARSER_DBG("RPC status changed\n");
            // RPC status changed because of CLI input
            //context->rpcMetadata.status = context->rpcLocal.status;
            switch (context->rpcLocal.status){
                case PARSER_RPC_START:
                    PARSER_PRINT("Sending RPC start command\n");
                    memcpy(&(context->rpcMetadata), &(context->rpcLocal), sizeof(parserRpcMessage_t));
                    break;
                case PARSER_RPC_PAUSE:
                    PARSER_PRINT("Sending RPC pause command\n");
                    context->rpcMetadata.status = context->rpcLocal.status;
                    break;
                case PARSER_RPC_SET_INTERVAL:
                    PARSER_PRINT("Sending RPC pause command for safe interval value change\n");
                    context->rpcMetadata.status = PARSER_RPC_PAUSE;
                    result = rdma_send_rpc(context);
                    if(result){
                        return PARSER_ERROR;
                    }
                    while(context->rpcLocal.status == PARSER_RPC_SET_INTERVAL){
                        //Wait until user enter new value
                    }
                    PARSER_PRINT("Sending RPC change interval command and start\n");
                    memcpy(&(context->rpcMetadata), &(context->rpcLocal), sizeof(parserRpcMessage_t));
                    break;
                case PARSER_RPC_STOP:
                    PARSER_PRINT("Sending RPC stop command\n");
                    memcpy(&(context->rpcMetadata), &(context->rpcLocal), sizeof(parserRpcMessage_t));
                    break;
                default:
                    PARSER_PRINT("Invalid RPC command\n");
                    return PARSER_ERROR;

            }
            result = rdma_send_rpc(context);
            if(result){
                return PARSER_ERROR;
            }

            PARSER_DBG("Send completed successfully\n");
            if(context->rpcMetadata.status==PARSER_RPC_STOP){
                pthread_join(cliThread, NULL);
                return PARSER_SUCCESS;
            }
        }
    }



}

/**
 * Entry point function for client process, initializing RDMA and RPC
 * connections
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int client(parserContext_t* context){
    int result = PARSER_ERROR;

    if(NULL == context){
        return PARSER_ERROR;
    }

    /*
     * Use rdma_getaddrinfo to determine if there is a path to the
     * other end. The server runs in passive mode so it waits for a
     * connection. The client uses this function call to determine if
     * a path exists based on the server name, port and the hints
     * provided. The result comes back in res.
     */
    context->rdmaMetadata.hints.ai_port_space = RDMA_PS_TCP;
    result = rdma_getaddrinfo(context->serverInfo.server,
                              context->serverInfo.port,
                              &(context->rdmaMetadata.hints),
                              &(context->rdmaMetadata.res));
    if(result){
        PARSER_DBG("Failed in rdma_getaddrinfo\n");
        return PARSER_ERROR;
    }

    /*
     * Now create a communication identifier and (on the client) a
     * queue pair (QP) for processing jobs. We set certain attributes
     * on this link.
     */
    context->rdmaMetadata.attr.cap.max_send_wr = context->rdmaMetadata.attr.cap.max_recv_wr = 1; //Max number of send/recv requests allowed in the queue
    context->rdmaMetadata.attr.cap.max_send_sge = context->rdmaMetadata.attr.cap.max_recv_sge = 1;
    context->rdmaMetadata.attr.qp_context = context->rdmaMetadata.id;
    context->rdmaMetadata.attr.sq_sig_all = 1;
    result = rdma_create_ep(&(context->rdmaMetadata.id),
                            context->rdmaMetadata.res,
                            NULL,
                            &(context->rdmaMetadata.attr));
    if(result){
        PARSER_DBG("Failed in rdma_create_ep\n");
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }


    /* Now either connect to the server or setup a wait for a
     * connection from a client. On the server side we also setup a
     * receive QP entry so we are ready to receive data.
     */
    context->rdmaMetadata.rpc_mr = rdma_reg_msgs(context->rdmaMetadata.id,
                                                 &(context->rpcMetadata),
                                                 sizeof(parserRpcMessage_t));
    if (!context->rdmaMetadata.rpc_mr){
        PARSER_DBG("Failed in rdma_reg_msgs for rpc\n");
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    context->rdmaMetadata.packet_mr = ibv_reg_mr(context->rdmaMetadata.id->pd,
                                                 &(context->packetMetadata),
                                                 sizeof(parserData_t),
                                                 IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE);
    if(!context->rdmaMetadata.packet_mr){
        PARSER_DBG("Failed in ibv_reg_mr\n");
        rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    context->rpcLocal.connectionData.bufferAddr = (uint64_t)(&context->packetMetadata.data);
    context->rpcLocal.connectionData.bufferRkey = context->rdmaMetadata.packet_mr->rkey;
    context->rpcLocal.connectionData.bufferLength = context->rdmaMetadata.packet_mr->length;
    context->rpcLocal.connectionData.streamingUSecInterval = PARSER_STREAMING_USEC_DEFAULT_INTERVAL;

    PARSER_PRINT("Connecting to server...\n");
    result = rdma_connect(context->rdmaMetadata.id, NULL);
    if(result){
        PARSER_DBG("Connection Failed\n");
        rdma_dereg_mr(context->rdmaMetadata.packet_mr);
        rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }
    PARSER_PRINT("Connection established\n");

    result = main_loop(context);

    rdma_disconnect(context->rdmaMetadata.id);
    rdma_dereg_mr(context->rdmaMetadata.packet_mr);
    rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
    rdma_destroy_ep(context->rdmaMetadata.id);
    rdma_freeaddrinfo(context->rdmaMetadata.res);

    return result;
}

/*****************************************************************************/
/**                                  Main                                   **/
/*****************************************************************************/
int main(int argc, char **argv) {
    int result = PARSER_ERROR;
    memset(&parserContext, 0, sizeof(parserContext_t));

    result = parse_args(argc, argv, &(parserContext.serverInfo));
    if(PARSER_SUCCESS != result){
        PARSER_DBG("Failed to parse args\n");
        return result;
    }
    print_header(&(parserContext.serverInfo));
    result = client(&parserContext);
    return result;
}


