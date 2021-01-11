//
// Created by irinag on 2/1/2020.
//

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "../include/endianess.h"


/*****************************************************************************/
/**                        Function implementation                          **/
/*****************************************************************************/
/**
 * Returns data block of given Vovit Chunk
 */
void getChunkDataBlock(Chunk* chunk, uint8_t buffer[DATA_BLOCK_SIZE]){
    for(int i=0 ; i<DATA_BLOCK_SIZE; i++){

#ifdef __LITTLE_ENDIANESS__
        if(i == 0) {
            buffer[i] = chunk->data_block_zero;
            continue;
        }
        if(i >= DATA_BLOCK_SIZE-3) {
            buffer[i] = chunk->data_block_last[i-(DATA_BLOCK_SIZE-4)];
            continue;
        }
#endif /* __LITTLE_ENDIANESS__ */

        buffer[i] = chunk->data_block[i];
    };
}

/**
 * Returns Sync Counter of given Basic Metadata.
 */
uint32_t calcSyncCount(BasicMetadata* meta){
#ifdef __LITTLE_ENDIANESS__
    return sync_cout;
#endif /* __LITTLE_ENDIANESS__ */
#ifdef __BIG_ENDIANESS__
    return __builtin_bswap32(meta->sync_cout);
#endif /* __BIG_ENDIANESS__ */

}
