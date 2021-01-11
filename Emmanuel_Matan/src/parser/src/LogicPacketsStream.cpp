//
// Created by irinag on 2/6/2020.
//

#include <utility>
#include "../include/LogicPacketsStream.h"
#include <iostream>
#include "../include/LogicChunk.h"
#include "../include/VovitChunk.h"
#include "../include/tokens.h"
#include <assert.h>
#include "../include/Exeptions.h"

using namespace pcie::gen3;
using namespace std;


LogicPacketsStream::LogicPacketsStream(string txFile, string rxFile) {
    file[RX] = std::move(rxFile);
    file[TX] = std::move(txFile);
    for(int rx_tx_ : {0,1}){
        curr_logic_data_byte_idx[rx_tx_] = 0;
        currPhyChunk[rx_tx_] = new LogicChunk(curr_logic_data_byte_idx[rx_tx_], file[rx_tx_]);
        verifyPortSpeed(rx_tx_);
    }
}

LogicPacketsStream::~LogicPacketsStream(){
    for(int rx_tx_ : {0,1}){
        free(currPhyChunk[rx_tx_]);
    }
    free(curr_packet);
    free(buffer);
}

void LogicPacketsStream::updateNextSyncCount(){
    curr_rx_tx_ = TX;
    if(currPhyChunk[RX]->getSyncCount() < currPhyChunk[TX]->getSyncCount()){
        curr_rx_tx_ = RX;
    }
}

void LogicPacketsStream::getNextLogicChunk(){
    free(currPhyChunk[curr_rx_tx_]);
    currPhyChunk[curr_rx_tx_] = new LogicChunk(curr_logic_data_byte_idx[curr_rx_tx_], file[curr_rx_tx_]);
    verifyPortSpeed(curr_rx_tx_);
}

void LogicPacketsStream::getNextXBytes(uint32_t x) {
    free(buffer);
    buffer = static_cast<uint8_t *>(malloc(x));
    for(uint32_t byte_idx = 0; byte_idx < x; (currPhyChunk[curr_rx_tx_]->it)++) {
        if (currPhyChunk[curr_rx_tx_]->it == currPhyChunk[curr_rx_tx_]->end()) {
            curr_logic_data_byte_idx[curr_rx_tx_] += currPhyChunk[curr_rx_tx_]->getDataBlockSize();
            getNextLogicChunk();
        }
        buffer[byte_idx++] = **(currPhyChunk[curr_rx_tx_]->it);
    }

}

void LogicPacketsStream::OnNext(){
    updateNextSyncCount();
    int token_type = ERR_TYPE;
    while(token_type == ERR_TYPE || token_type == IDL_TYPE) {
        getNextXBytes(TOKEN_BUFFER_SIZE);
        token_type = getTokenType(buffer);
    }
    if(token_type != STP_TYPE) return; // need to add flows for EDS, EDB and SDP

    curr_packet_size_dw = GET_TLP_LENGTH(((Stp *) buffer)->length_msb, ((Stp *) buffer)->length_lsb);
    uint32_t curr_packet_size_bytes = curr_packet_size_dw * sizeof(uint32_t);
    getNextXBytes(curr_packet_size_bytes-4);

    curr_packet = static_cast<uint8_t *>(malloc(curr_packet_size_bytes));
    curr_packet[0] = GEN3;
    curr_packet[1] = static_cast<uint8_t >(token_type);  // 0 for TLP
    curr_packet[2] = static_cast<uint8_t >((curr_packet_size_dw&0xff00) >> 0x8);
    curr_packet[3] = static_cast<uint8_t >(curr_packet_size_dw&0xff);

    for (int i = 0; i < curr_packet_size_bytes-4; i++) {
        curr_packet[i+4] = buffer[i];
    }
}


uint8_t* LogicPacketsStream::getCurrPacket(){
    /*printf("getCurrPacket (Tx=0, Rx=1) from %d:\n", curr_rx_tx_);
    for (int i = 0; i < curr_packet_size_dw*sizeof(uint32_t); i++) {
        printf("%02x ", (uint32_t) ((curr_packet[i]) & 0xff));
        if(i%4 == 3) printf("| ");
    }
    printf("\n");*/
    return curr_packet;
}

void LogicPacketsStream::verifyPortSpeed(int rx_tx_){
    uint8_t port_speed = currPhyChunk[rx_tx_]->getPortSpead();
    if(port_speed != GEN3){
        throw pcie::LogicPacketStreamInvlidSpeedException(curr_logic_data_byte_idx[rx_tx_], port_speed);
    }
}

uint32_t LogicPacketsStream::getCurrPacketSizeBytes(){ return curr_packet_size_dw*sizeof(uint32_t);}