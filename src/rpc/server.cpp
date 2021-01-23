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
#include "server.h"

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
bool trigger_changed=false;
int packet_num=0;
uint32_t filter[ADDRESS_LENGTH_BYTE];
uint32_t trigger[ADDRESS_LENGTH_BYTE];


parserData_t* data_before_trigger;
uint32_t* data_size_before_trigger;
int last_index=0;



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


void left_shift(){


        for (int i = 1; i < packet_num; i++){
            data_before_trigger[i - 1] = data_before_trigger[i];
            data_size_before_trigger[i - 1] = data_size_before_trigger[i];
        }


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
    if(trigger_flag) return;
    for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
        if((uint32_t) ((context->packetMetadata.data[i+ADDRESS_START_BYTE]) & 0xff)!=trigger[i]) {
            return;
        }

    }
    trigger_changed=true;
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

    pcie_trigger(context);

    if(!trigger_flag and packet_num>0 and pcie_filter(context) ){

        if(last_index==packet_num){
            left_shift();
            memcpy(&data_before_trigger[last_index-1],&(context->packetMetadata), sizeof(parserData_t));
            data_size_before_trigger[last_index-1] = context->stream->getCurrPacketSizeBytes();

        }else{
            memcpy(&data_before_trigger[last_index],&(context->packetMetadata), sizeof(parserData_t));
            data_size_before_trigger[last_index] = context->stream->getCurrPacketSizeBytes();
            last_index++;

        }


    }

    if(trigger_flag ) {
        if(trigger_changed and packet_num>0){

            for(int i=0; i<packet_num;i++){
                if(data_size_before_trigger[i]>0){
                    sendto(sockfd, (const char *) &(data_before_trigger[i]), context->stream->getCurrPacketSizeBytes(),
                           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                           sizeof(servaddr));
                }
            }
            trigger_changed=false;
        }
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

    if(argv[1]!= (string)"empty") {
        char target1[ADDRESS_LENGTH_BYTE] = {0};
        hex2bin(argv[1], target1);

        for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
            filter[i] =  (uint32_t) ((target1[i]) & 0xff);

        }


    }else{
        filter_flag=true;
    }

    if(argv[3]!= (string)"empty") {

        char target3[ADDRESS_LENGTH_BYTE] = {0};
        hex2bin(argv[3], target3);

        for (int i = 0; i < ADDRESS_LENGTH_BYTE; i++) {
            trigger[i]=(uint32_t) ((target3[i]) & 0xff);
        }


    }else{
        trigger_flag=true;
    }
    if(stoi(argv[4])>0){

        packet_num=stoi(argv[4]);
        data_before_trigger=(parserData_t*) malloc(sizeof(*data_before_trigger)*packet_num);
        data_size_before_trigger=(uint32_t*) malloc(sizeof(*data_size_before_trigger)*packet_num);

        for(int i=0; i<packet_num; i++){
            data_size_before_trigger[i]=0;
        }
       

    }

    sleep(1);

    while (1){


            sendPacket(&parserContext);
            usleep((&parserContext)->streamingUSecInterval);


    }


}

