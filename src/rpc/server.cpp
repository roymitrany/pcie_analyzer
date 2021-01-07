//
// Created by Alexey on 06/03/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
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
char* client_ip;
int flag =0;
bool trigger_flag=false;
bool filter_flag=false;
uint32_t filter[ADDRESS_LENGTH_BYTE];
uint32_t trigger[ADDRESS_LENGTH_BYTE];
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

    if(filter_flag){
       return true;
    }

    for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
        if((uint32_t) ((context->packetMetadata.data[i+ADDRESS_START_BYTE]) & 0xff)!=filter[i]) {
            return false;
        }

    }
    return true;
}



void pcie_trigger(parserContext_t* context){
    for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
        if((uint32_t) ((context->packetMetadata.data[i+ADDRESS_START_BYTE]) & 0xff)!=trigger[i]) {
            return;
        }

    }
    trigger_flag=true;

}

/**
 * Data stream send packet function
 * @param ptr parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int sendPacket(parserContext_t* context){
    int result;


    try {
        context->stream->OnNext();
        memset(&(context->packetMetadata), 0, sizeof(parserData_t));
        memcpy(&(context->packetMetadata),
                context->stream->getCurrPacket(),
                context->stream->getCurrPacketSizeBytes());
        //printCurrPacket(context->stream->getCurrPacketSizeBytes(),
        //                context->stream->getCurrPacket());
    }catch (LogicPacketStreamInvlidSpeedException& e){
        return PARSER_ERROR;
    }

    int sockfd;

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
    inet_pton(AF_INET, client_ip, &(servaddr.sin_addr));

    int n, len;

    if(flag<10){
        printCurrPacket( context->stream->getCurrPacketSizeBytes(),(const uint8_t*)&(context->packetMetadata.data)  );
        flag++;
    }

    if(!trigger_flag){
        pcie_trigger(context);
    }

    if(trigger_flag) {
        if (pcie_filter(context)) {
            sendto(sockfd, (const char *) &(context->packetMetadata), context->stream->getCurrPacketSizeBytes(),
                   MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                   sizeof(servaddr));
        }
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






int char2int(char input) {
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
    client_ip = argv[2];
    memset(&parserContext, 0, sizeof(parserContext_t));


    init_packet_stream(&parserContext);

    printf("pcie filter1 is :  %s\n",argv[1]);
    printf("pcie trigger is :  %s\n",argv[3]);
    printf("ip address is :  %s\n",argv[2]);

    if(argv[1]!= (string)"empty") {
        char target1[ADDRESS_LENGTH_BYTE] = {0};
        hex2bin(argv[1], target1);

        for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
            PARSER_PACKET_PRINT("%02x ", (uint32_t) ((target1[i]) & 0xff));
            filter[i] =  (uint32_t) ((target1[i]) & 0xff);

        }
        printf("\n");

    }else{
        filter_flag=true;
    }

    if(argv[3]!= (string)"empty") {
        printf("trigger is not empty\n");
        char target3[ADDRESS_LENGTH_BYTE] = {0};
        hex2bin(argv[3], target3);

        for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {

            PARSER_PACKET_PRINT("%02x ", (uint32_t) ((target3[i]) & 0xff));
            trigger[i]=(uint32_t) ((target3[i]) & 0xff);
        }
        printf("\n");

    }else{
        trigger_flag=true;
    }


    sleep(1);

    while (1){


            sendPacket(&parserContext);
            usleep((&parserContext)->streamingUSecInterval);


    }


}

