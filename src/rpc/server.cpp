//
// Created by Alexey on 06/03/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "pci_sniff.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <infiniband/verbs.h>
#include <rpc/rpc.h>
#include "../parser/include/Exeptions.h"
#include "common.h"

/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace std;

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
static void print_header(serverInfo_t* serverInfo){
    if(NULL == serverInfo){
        return;
    }
    print_header_common();
    PARSER_PRINT("Starting in server mode\n");
    PARSER_PRINT("Port:%s\n", serverInfo->port);
}

/**
 * Packet print utility
 * @param packet_size_bytes packet size
 * @param packet packet start address
 */
void printCurrPacket(uint32_t packet_size_bytes, const uint8_t* packet){
    printf("============================================================\n");

    for (int i = 0; i < packet_size_bytes; i++) {
        PARSER_PACKET_PRINT("%02x ", (uint32_t) ((packet[i]) & 0xff));
        if(i%4 == 3) PARSER_PACKET_PRINT("| ");
    }
    PARSER_PACKET_PRINT("\n");
    printf("============================================================\n");
}

/**
 * Data stream send packet function
 * @param ptr parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int sendPacket(parserContext_t* context){
    int result;
    PARSER_DBG("Sending new packet\n");

    try {
        context->stream->OnNext();
        memset(&(context->packetMetadata), 0, sizeof(parserData_t));
        memcpy(&(context->packetMetadata),
                context->stream->getCurrPacket(),
                context->stream->getCurrPacketSizeBytes());
        //printCurrPacket(context->stream->getCurrPacketSizeBytes(),
        //                context->stream->getCurrPacket());
    }catch (LogicPacketStreamInvlidSpeedException& e){
        PARSER_DBG("Failed in stream get packet\n");
        return PARSER_ERROR;
    }

    int sockfd;
    //char buffer[MAXLINE];
    //char *hello = "Hello from client";
    struct sockaddr_in     servaddr;
    //char str[INET_ADDRSTRLEN];



    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(21212);
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    // store this IP address in sa:
    inet_pton(AF_INET, "192.168.0.2", &(servaddr.sin_addr));

    int n, len;

    sendto(sockfd, (const char *)&(context->packetMetadata), context->stream->getCurrPacketSizeBytes(),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    printf("Tusov message sent.\n");

    /*n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);*/

    close(sockfd);
    return 0;
    /*result = rdma_post_write(context->rdmaMetadata.id,
                             NULL,
                             &context->packetMetadata,
                             context->stream->getCurrPacketSizeBytes(),
                             context->rdmaMetadata.packet_mr,
                             0,
                             context->rpcMetadata.connectionData.bufferAddr,
                             context->rpcMetadata.connectionData.bufferRkey);


     if(result){
        PARSER_DBG("Failed in rdma_post_write\n");
        return PARSER_ERROR;
    }*/
}

/**
 * Data stream thread function
 * @param ptr parser context instance
 * @return NULL
 */
static void* streamingThreadFunction(void* ptr){
    parserContext_t* context= (parserContext_t*)ptr;
    while (1){
        if(context->rpcLocal.status == PARSER_RPC_STOP){
            return 0;
        }

        if(context->rpcLocal.status == PARSER_RPC_START){
            sendPacket(context);
            usleep(context->rpcLocal.connectionData.streamingUSecInterval);
        }
    }
}

/**
 * RPC received message handler
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int handle_message(parserContext_t* context){
    switch (context->rpcMetadata.status){
        case PARSER_RPC_START:
            PARSER_PRINT("Streaming started. Streaming packets interval: %d [usec]\n",
                         context->rpcMetadata.connectionData.streamingUSecInterval);
            break;
        case PARSER_RPC_PAUSE:
            PARSER_PRINT("Streaming paused.\n");
            break;
        case PARSER_RPC_SET_INTERVAL:
            PARSER_PRINT("Changing streaming packets interval to %d [usec].\n",
                         context->rpcMetadata.connectionData.streamingUSecInterval);
            break;
        case PARSER_RPC_STOP:
            PARSER_PRINT("Streaming terminated.\n");
            break;
        default:
            return PARSER_ERROR;
    }
    memcpy(&context->rpcLocal, &context->rpcMetadata, sizeof(parserRpcMessage_t));
    return PARSER_SUCCESS;
}

/**
 * Servers entry point to main loop after successful connection
 * Main process handles RPC client and the nested thread handles data stream
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int main_loop(parserContext_t* context){
    pthread_t streamingThread;
    struct ibv_wc wc;
    int result;

    context->rpcMetadata.status = PARSER_RPC_INVALID;
    context->rpcLocal.status = PARSER_RPC_INVALID;

    result = pthread_create(&streamingThread,
                            NULL,
                            streamingThreadFunction,
                            (void*)(context));
    if(result){
        PARSER_DBG("Failed to create CLI thread\n");
        return PARSER_ERROR;
    }

    PARSER_DBG("Starting main loop\n");
    while (1){
        //result = rdma_get_recv_comp(id, &wc);
        result = rdma_get_recv_comp(context->rdmaMetadata.id, &wc);
        if(result < 0){
            PARSER_DBG("Failed in rdma_get_recv_comp\n");
            return PARSER_ERROR;
        }else if(result > 0){
            //PARSER_DBG("Got something in receive queue, result: %d\n", result);
            if(IBV_WC_SUCCESS != wc.status){
                PARSER_DBG("Got receive completion with status code %d\n", wc.status);
                return PARSER_ERROR;
            }
            if(wc.byte_len != sizeof(parserRpcMessage_t)){
                PARSER_DBG("Got receive completion with not matching length\n");
                return PARSER_ERROR;
            }

            result = handle_message(context);
            if(PARSER_SUCCESS != result){
                PARSER_DBG("Failed in handle_message of received message\n");
                return PARSER_ERROR;
            }
            if(context->rpcLocal.status == PARSER_RPC_STOP){
                pthread_join(streamingThread, NULL);
                return PARSER_SUCCESS;
            }

            result = rdma_post_recv(context->rdmaMetadata.id,
                                    (void *)(uintptr_t)0,
                                    &context->rpcMetadata,
                                    sizeof(parserRpcMessage_t),
                                    context->rdmaMetadata.rpc_mr);
            if (result){
                PARSER_DBG("Failed in rdma_post_recv\n");
                return PARSER_ERROR;
            }

        }

    }
}

/**
 * Entry point function for server process,
 * Initializing RDMA and RPC connections
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int server(parserContext_t* context){
    int result = PARSER_ERROR;

    if(NULL == context){
        return PARSER_ERROR;
    }

    context->rdmaMetadata.hints.ai_flags = RAI_PASSIVE;
    context->rdmaMetadata.hints.ai_port_space = RDMA_PS_TCP;
    result = rdma_getaddrinfo(NULL,
                              context->serverInfo.port,
                              &(context->rdmaMetadata.hints),
                              &(context->rdmaMetadata.res));
    if(result){
        PARSER_DBG("Failed in rdma_getaddrinfo\n");
        return PARSER_ERROR;
    }

    context->rdmaMetadata.attr.cap.max_send_wr = 1;
    context->rdmaMetadata.attr.cap.max_recv_wr = 1;
    context->rdmaMetadata.attr.cap.max_send_sge = 1;
    context->rdmaMetadata.attr.cap.max_recv_sge = 1;
    context->rdmaMetadata.attr.sq_sig_all = 1;
    result = rdma_create_ep(&(context->rdmaMetadata.listen_id),
                            context->rdmaMetadata.res,
                            NULL,
                            &(context->rdmaMetadata.attr));
    if(result){
        PARSER_DBG("Failed in rdma_create_ep\n");
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    result = rdma_listen(context->rdmaMetadata.listen_id, 0);
    if (result) {
        PARSER_DBG("Failed in rdma_listen\n");
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    result = rdma_get_request(context->rdmaMetadata.listen_id,
                              &(context->rdmaMetadata.id));
    if (result) {
        PARSER_DBG("Failed in rdma_get_request\n");
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    context->rdmaMetadata.rpc_mr = rdma_reg_msgs(context->rdmaMetadata.id,
                                                 &(context->rpcMetadata), sizeof(parserRpcMessage_t));
    if (!context->rdmaMetadata.rpc_mr){
        PARSER_DBG("Failed in rdma_reg_msgs for rpc\n");
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    result = rdma_post_recv(context->rdmaMetadata.id,
                            (void *)(uintptr_t)0,
                            &(context->rpcMetadata),
                            sizeof(parserRpcMessage_t),
                            context->rdmaMetadata.rpc_mr);
    if (result){
        PARSER_DBG("Failed in rdma_post_recv\n");
        rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    context->rdmaMetadata.packet_mr = ibv_reg_mr(context->rdmaMetadata.id->pd,
                                                 &(context->packetMetadata),
                                                 sizeof(parserData_t),
                                                 IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE);
    if(!context->rdmaMetadata.packet_mr){
        PARSER_DBG("Failed in ibv_reg_mr\n");
        rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    result = rdma_accept(context->rdmaMetadata.id, NULL);
    if(result){
        PARSER_DBG("Failed in rdma_accept\n");
        rdma_dereg_mr(context->rdmaMetadata.packet_mr);
        rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
        rdma_destroy_ep(context->rdmaMetadata.id);
        rdma_destroy_ep(context->rdmaMetadata.listen_id);
        rdma_freeaddrinfo(context->rdmaMetadata.res);
        return PARSER_ERROR;
    }

    result = main_loop(context);

    PARSER_DBG("Main loop completed\n");
    rdma_disconnect(context->rdmaMetadata.id);
    rdma_dereg_mr(context->rdmaMetadata.packet_mr);
    rdma_dereg_mr(context->rdmaMetadata.rpc_mr);
    rdma_destroy_ep(context->rdmaMetadata.id);
    rdma_destroy_ep(context->rdmaMetadata.listen_id);
    rdma_freeaddrinfo(context->rdmaMetadata.res);

    return result;
}

/**
 * Init packet stream module as packets data manager
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int init_packet_stream(parserContext_t* context){
    std::string txFile = R"(../bin/tx/vovit.bin)";
    std::string rxFile = R"(../bin/rx/vovit.bin)";

    try {
        context->stream = new gen3::LogicPacketsStream(txFile, rxFile);
    }
    catch (LogicPacketStreamInvlidSpeedException& e) {
        std::cout << " Invalid port speed. Try another LogicPacketStream. The speed is " << (int)e.getPortSpeed()
                  << ", byte offset is " << (int) e.getByteOffset() << std::endl;

        /*
        switch(e.getPortSpeed()){
            case GEN1:
                stream = new gen1::LogicPacketsStream(txFile, rxFile);
                break;
            case GEN2:
                stream = new gen2::LogicPacketsStream(txFile, rxFile);
                break;
            case GEN4:
                stream = new gen4::LogicPacketsStream(txFile, rxFile);
                break;
        }*/
        return PARSER_ERROR;
    }

    return PARSER_SUCCESS;
}
int * start_1_svc(void *v, struct svc_req *){
    int result = 0;
    printf("Strating!!!!!!!\n");
    return &result;
}

int * pause_1_svc(void *v, struct svc_req *){
    printf("Pausing!!!!!!!\n");
}

int * interval_1_svc(int *interval, struct svc_req *){
    printf("Changing interval to %d\n", *interval);
}

int * stop_1_svc(void *v, struct svc_req *){
    printf("Stopping!!!!!!!\n");
}

static void
pcisniff_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
    union {
        int interval_1_arg;
        int stop_1_arg;
    } argument;
    char *result;
    xdrproc_t _xdr_argument, _xdr_result;
    char *(*local)(char *, struct svc_req *);

    switch (rqstp->rq_proc) {
        case NULLPROC:
            (void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
            return;

        case START:
            _xdr_argument = (xdrproc_t) xdr_void;
            _xdr_result = (xdrproc_t) xdr_long;
            local = (char *(*)(char *, struct svc_req *)) start_1_svc;
            break;

        case PAUSE:
            _xdr_argument = (xdrproc_t) xdr_void;
            _xdr_result = (xdrproc_t) xdr_long;
            local = (char *(*)(char *, struct svc_req *)) pause_1_svc;
            break;

        case INTERVAL:
            _xdr_argument = (xdrproc_t) xdr_int;
            _xdr_result = (xdrproc_t) xdr_long;
            local = (char *(*)(char *, struct svc_req *)) interval_1_svc;
            break;

        case STOP:
            _xdr_argument = (xdrproc_t) xdr_int;
            _xdr_result = (xdrproc_t) xdr_long;
            local = (char *(*)(char *, struct svc_req *)) stop_1_svc;
            break;

        default:
            svcerr_noproc (transp);
            return;
    }
    memset ((char *)&argument, 0, sizeof (argument));
    if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
        svcerr_decode (transp);
        return;
    }
    result = (*local)((char *)&argument, rqstp);
    if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
        svcerr_systemerr (transp);
    }
    if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
        fprintf (stderr, "%s", "unable to free arguments");
        exit (1);
    }
    return;
}
/*****************************************************************************/
/**                                  Main                                   **/
/*****************************************************************************/
int main(int argc, char **argv) {
/*    int result;

    memset(&parserContext, 0, sizeof(parserContext_t));
    result = init_packet_stream(&parserContext);
    if(PARSER_SUCCESS != result){
        PARSER_DBG("Failed to init packet stream\n");
        return result;
    }

    result = parse_args(argc, argv, &parserContext.serverInfo);
    if(PARSER_SUCCESS != result){
        PARSER_DBG("Failed to parse args\n");
        return result;
    }
    print_header(&parserContext.serverInfo);
    result = server(&parserContext);
    return result;*/
    register SVCXPRT *transp;

    pmap_unset (PCISNIFF, PCISNIFF_V1);

    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create udp service.");
        exit(1);
    }
    if (!svc_register(transp, PCISNIFF, PCISNIFF_V1, pcisniff_1, IPPROTO_UDP)) {
        fprintf (stderr, "%s", "unable to register (PCISNIFF, PCISNIFF_V1, udp).");
        exit(1);
    }

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create tcp service.");
        exit(1);
    }
    if (!svc_register(transp, PCISNIFF, PCISNIFF_V1, pcisniff_1, IPPROTO_TCP)) {
        fprintf (stderr, "%s", "unable to register (PCISNIFF, PCISNIFF_V1, tcp).");
        exit(1);
    }

    svc_run ();
    fprintf (stderr, "%s", "svc_run returned");
    exit (1);
    /* NOTREACHED */

}

