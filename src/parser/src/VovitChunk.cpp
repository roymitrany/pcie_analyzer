//
// Created by irinag on 1/31/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <sstream>
#include "../include/VovitChunk.h"
#include "../include/Exeptions.h"
#include "../include/endianess.h"


/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace pcie;
using namespace std;


/*****************************************************************************/
/**                     Class Methods Implementation                        **/
/*****************************************************************************/
VovitChunk::VovitChunk(int chunk_idx, string file) : chunk_idx(chunk_idx) {
    FILE *ptr;
    ptr = fopen(file.c_str(), "rb");  // r for read, b for binary
    if (ptr == nullptr){
        printf("Error! opening file");
        exit(1);
    }
    fseek(ptr, VOVIT_CHUNK_SIZE*chunk_idx, SEEK_SET );
    if(feof(ptr)) {
        throw VovitChunkEOFException();
    }
    fread(buffer, sizeof(buffer), 1, ptr);
    fclose(ptr);

    vovit_chunk = (Chunk*) buffer;
    port_speed = vovit_chunk->metadata.port_speed;
    port_width = vovit_chunk->metadata.port_width;
    if (port_width > 16) {
        throw VovitChunkInvalidArgsException();
    }
}

uint32_t VovitChunk::getSyncCount(){
    return calcSyncCount(&(this->vovit_chunk->metadata));
}

uint8_t VovitChunk::getPortWidth(){
    return this->port_width;
}

void VovitChunk::printAll(){
    std::cout  << "============== VOVIT CHUNK " << chunk_idx << " ===============" << std::endl;
    for (int i=0; i<VOVIT_CHUNK_SIZE; i++) {
        if(i == 8 || i == 11 || (((i-11) % DATA_BLOCK_PARTITION_SIZE == 0) && i > 16 && i <= 107)) {
            printf(" | ");
        }
        printf("%02x", (uint32_t)(buffer[i] & 0xff));
    }
    std::cout << std::endl;

    std::cout  << "CHUNK CONTROL: " << std::endl;
    printf("%x ",   vovit_chunk->control);
    printf("%x ",   vovit_chunk->control.is_valid);
    printf("%x ",   vovit_chunk->control.sync_hdr);
    printf("%x ",   vovit_chunk->control.sync_hdr_error);
    std::cout  << std::endl;

    std::cout  << "CHUNK METADATA: " << std::endl;
    printf("%x ",  vovit_chunk->metadata);
    printf("%x ", (vovit_chunk->metadata.port_state));
    printf("%x ", (vovit_chunk->metadata.port_speed));
    printf("%x ", (vovit_chunk->metadata.port_width));
    std::cout  << std::endl;

    std::cout  << "DATA BLOCK: " << std::endl;
    uint8_t data[DATA_BLOCK_SIZE];
    getChunkDataBlock(vovit_chunk, data);
    for (int i=0; i<DATA_BLOCK_SIZE; i++) {
        if(i % DATA_BLOCK_PARTITION_SIZE == 0 && i != 0) {
            printf(" | ");

        }
        printf("%02x", (uint32_t)(data[i] & 0xff));
    }
    std::cout  << "\n============================= " << std::endl;
}