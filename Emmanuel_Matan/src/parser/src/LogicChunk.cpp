//
// Created by irinag on 2/3/2020.
//

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "../include/LogicChunk.h"
#include <iostream>
#include <array>

/*****************************************************************************/
/**                              Defines                                    **/
/*****************************************************************************/
#define NUM_OF_VOVIT_CHUNK_PARTITIONS 6


/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace pcie;


/*****************************************************************************/
/**                     Class Methods Implementation                        **/
/*****************************************************************************/
LogicChunk::LogicChunk(uint32_t base_byte_idx, std::string file)  {
    this->data_block_size = static_cast<size_t>(DATA_BLOCK_PARTITION_SIZE);  // pre definition
    this->base_partition_idx = base_byte_idx / DATA_BLOCK_PARTITION_SIZE;
    uint32_t baseVovitChunkIdx = base_partition_idx / NUM_OF_VOVIT_CHUNK_PARTITIONS;
    uint32_t baseVovitChunkDataOffsetIdx = base_partition_idx % NUM_OF_VOVIT_CHUNK_PARTITIONS;

    int data_buffer_idx = 0;
    for(int chunk_i=0; data_buffer_idx<data_block_size; chunk_i++){
        auto* vovitChunk = new VovitChunk(baseVovitChunkIdx+chunk_i, file);
        int byte_idx=0;
        if(chunk_i == 0) {
            this->sync_count = vovitChunk->getSyncCount();
            this->port_width = vovitChunk->getPortWidth();
            this->data_block_size = static_cast<size_t>(port_width*DATA_BLOCK_PARTITION_SIZE);
            this->port_speed = vovitChunk->getPortSpead();
            data_buffer = static_cast<uint8_t *>(malloc(data_block_size));
            logic_data_buffer = static_cast<uint8_t *>(malloc(data_block_size));
            byte_idx = baseVovitChunkDataOffsetIdx*DATA_BLOCK_PARTITION_SIZE;
        }
        for(; byte_idx<DATA_BLOCK_SIZE && data_buffer_idx<data_block_size ; byte_idx++){
            data_buffer[data_buffer_idx++] = vovitChunk->vovit_chunk->data_block[byte_idx];
        }
    }
    toLogicBuffer();
    it = iterator(logic_data_buffer);
}


void LogicChunk::toLogicBuffer() {
    int buffer_idx = 0;
    int data_buffer_idx;
    for(int j=0; j<DATA_BLOCK_PARTITION_SIZE; j++){
        for(int partition_idx=0; partition_idx<port_width; partition_idx++){
            data_buffer_idx = partition_idx*DATA_BLOCK_PARTITION_SIZE + j;
            logic_data_buffer[buffer_idx++] = data_buffer[data_buffer_idx];
        }
    }
}

uint32_t LogicChunk::getSyncCount(){
    return sync_count;
}

size_t LogicChunk::getDataBlockSize(){
    return data_block_size;
}

void LogicChunk::printAll(){
    std::cout  << "============ LOGIC CHUNK, base partition index = " << base_partition_idx << " ============" << std::endl;
    std::cout  << "LOGIC CHUNK DATA SIZE (B) = " << data_block_size << std::endl;
    printf("LOGIC CHUNK SYNC COUNTER = %x\n", sync_count);
    std::cout  << "LOGIC CHUNK DATA: " << std::endl;
    for (int i=0; i<data_block_size; i++) {
        if(i % DATA_BLOCK_PARTITION_SIZE == 0 && i != 0) {
            printf(" | ");

        }
        printf("%02x", (uint32_t)(data_buffer[i] & 0xff));
    }

    std::cout  << "\n\nLOGIC CHUNK DATA (2): " << std::endl;
    for (int i=0; i<data_block_size; i++) {
        if(i % DATA_BLOCK_PARTITION_SIZE == 0 && i != 0) {
            printf("\n");

        }
        printf("%02x ", (uint32_t)(data_buffer[i] & 0xff));
    }

    std::cout  << "\n\nLOGIC CHUNK LOGIC BUFFER: " << std::endl;
    for (int i=0; i<data_block_size; i++) {
        if(i % DATA_BLOCK_PARTITION_SIZE == 0 && i != 0) {
            printf(" | ");

        }
        printf("%02x", (uint32_t)(logic_data_buffer[i] & 0xff));
    }
    std::cout  << "\n\n" << std::endl;
}


LogicChunk::~LogicChunk(){
    free(data_buffer);
    free(logic_data_buffer);
}


size_t LogicChunk::size() const { return data_block_size; }

uint8_t& LogicChunk::operator[](size_t index) {
    assert(index < data_block_size);
    return logic_data_buffer[index];
}

const uint8_t& LogicChunk::operator[](size_t index) const {
    assert(index < data_block_size);
    return logic_data_buffer[index];
}

LogicChunk::iterator LogicChunk::begin() {
    return {logic_data_buffer};
}

LogicChunk::iterator LogicChunk::end() {
    return {logic_data_buffer + data_block_size};
}

LogicChunk::const_iterator LogicChunk::begin() const {
    return {logic_data_buffer};
}

LogicChunk::const_iterator LogicChunk::end() const {
    return {logic_data_buffer + data_block_size};
}

