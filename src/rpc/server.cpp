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
int rpc_res;
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

    close(sockfd);
    return 0;
}

/**
 * Data stream thread function
 * @param ptr parser context instance
 * @return NULL
 */
static void* streamingThreadFunction(void* ptr){
    parserContext_t* context= (parserContext_t*)ptr;
    while (1){

        if(context->status == STOP){
            return 0;
        }

        if(context->status == START){
            sendPacket(context);
            usleep(context->streamingUSecInterval);
        }
        if(context->status == PAUSE){
            sleep(1);
        }

    }
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
/*int * start_3_svc(char *v, struct svc_req *){
    rpc_res = 10;
    printf("Strating!!!!!!!\n");
    parserContext.status=START;
    return &rpc_res;
}*/

int * start_3_svc(void *v, struct svc_req *){
    rpc_res = 10;
    printf("Starting!!!!!!!\n");
    parserContext.status=STOP;
    return &rpc_res;
}

int * pause_3_svc(void *v, struct svc_req *){
    rpc_res = 11;
    printf("Pausing!!!!!!!\n");
    parserContext.status=PAUSE;
    return &rpc_res;
}

int * interval_3_svc(int *interval, struct svc_req *){
    rpc_res = 12;
    printf("Changing interval to %d\n", *interval);
    parserContext.streamingUSecInterval = *interval;
    return &rpc_res;
}

int * stop_3_svc(void *v, struct svc_req *){
    rpc_res = 13;
    printf("Stopping!!!!!!!\n");
    parserContext.status=STOP;
    return &rpc_res;
}


static void
pcisniff_3(struct svc_req *rqstp, register SVCXPRT *transp)
{
    union {
        int interval_3_arg;
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
            _xdr_result = (xdrproc_t) xdr_int;
            local = (char *(*)(char *, struct svc_req *)) start_3_svc;
            break;

        case PAUSE:
            _xdr_argument = (xdrproc_t) xdr_void;
            _xdr_result = (xdrproc_t) xdr_int;
            local = (char *(*)(char *, struct svc_req *)) pause_3_svc;
            break;

        case INTERVAL:
            _xdr_argument = (xdrproc_t) xdr_int;
            _xdr_result = (xdrproc_t) xdr_int;
            local = (char *(*)(char *, struct svc_req *)) interval_3_svc;
            break;

        case STOP:
            _xdr_argument = (xdrproc_t) xdr_void;
            _xdr_result = (xdrproc_t) xdr_int;
            local = (char *(*)(char *, struct svc_req *)) stop_3_svc;
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
    memset(&parserContext, 0, sizeof(parserContext_t));


    init_packet_stream(&parserContext);
    int result;

    pthread_t streamingThread;
    result = pthread_create(&streamingThread,
                            NULL,
                            streamingThreadFunction,
                            (void*)(&parserContext));
    if(result){
        PARSER_DBG("Failed to create CLI thread\n");
        return PARSER_ERROR;
    }
    print_header(&parserContext.serverInfo);
    //rpc_mainn();

    register SVCXPRT *transp;

    pmap_unset (PCISNIFF, PCISNIFF_V2);

    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create udp service.");
        exit(1);
    }
    if (!svc_register(transp, PCISNIFF, PCISNIFF_V2, pcisniff_3, IPPROTO_UDP)) {
        fprintf (stderr, "%s", "unable to register (PCISNIFF, PCISNIFF_V2, udp).");
        exit(1);
    }

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create tcp service.");
        exit(1);
    }
    if (!svc_register(transp, PCISNIFF, PCISNIFF_V2, pcisniff_3, IPPROTO_TCP)) {
        fprintf (stderr, "%s", "unable to register (PCISNIFF, PCISNIFF_V2, tcp).");
        exit(1);
    }

    svc_run ();
    fprintf (stderr, "%s", "svc_run returned");
    exit (1);
    /* NOTREACHED */

}

