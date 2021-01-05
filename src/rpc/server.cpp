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

#define ADDRESS_START_BYTE 12

#define ADDRESS_LENGTH_BYTE 4



/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace std;

int flag =0;
uint32_t filter[ADDRESS_LENGTH_BYTE];


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


bool pcie_filter(parserContext_t* context){
    for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
        if((uint32_t) ((context->packetMetadata.data[i+ADDRESS_START_BYTE]) & 0xff)!=filter[i]) {
            return false;
        }

    }
    return true;
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

    if(flag<10){
        printCurrPacket( context->stream->getCurrPacketSizeBytes(),(const uint8_t*)&(context->packetMetadata.data)  );
        flag++;
    }


    if(pcie_filter(context)) {
        sendto(sockfd, (const char *) &(context->packetMetadata), context->stream->getCurrPacketSizeBytes(),
               MSG_CONFIRM, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));
    }


    //printf("Tusov message sent.\n");

    close(sockfd);
    return 0;
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


        return PARSER_ERROR;
    }

    return PARSER_SUCCESS;
}




/*
 * This method is copies as is from pci_sniff_svc.c. If we keep the file there, it does not compile due to c/c++ issues
 * Needs to be re-copied if we run rpcgen again.
 * The core will be later replaced with gRPC library, hopefully with better results.
 */



int char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(const char* src, char* target)
{
    while(*src && src[1])
    {
        *(target++) = char2int(*src)*16 + char2int(src[1]);
        src += 2;
    }
}



 /*
/*****************************************************************************/
/**                                  Main                                   **/
/*****************************************************************************/
int main(int argc, char **argv) {
    memset(&parserContext, 0, sizeof(parserContext_t));


    init_packet_stream(&parserContext);

    printf("pcie filter1 is :  %s\n",argv[1]);
    //printf("pcie filter2 is :  %02x", argv[1]);
    //printf("pcie filter2 is :  %08lx", argv[1]);

    char target[ADDRESS_LENGTH_BYTE]={0};
    hex2bin(argv[1], target);


    for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
        PARSER_PACKET_PRINT("%02x ", (uint32_t) ((target[i]) & 0xff));
        filter[i] =  (uint32_t) ((target[i]) & 0xff);
    }
                printf("\n");


    sleep(1);

    while (1){


            sendPacket(&parserContext);
            usleep((&parserContext)->streamingUSecInterval);


    }


}

