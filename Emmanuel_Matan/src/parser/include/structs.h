//
// Created by irinag on 1/31/2020.
//

#ifndef TLP_PARSER_STRUCTS_H
#define TLP_PARSER_STRUCTS_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <cstdint>


/*****************************************************************************/
/**                              Defines                                    **/
/*****************************************************************************/
#define GEN3 3
#define VOVIT_CHUNK_SIZE 128
#define DATA_BLOCK_SIZE 96
#define DATA_BLOCK_PARTITION_SIZE (DATA_BLOCK_SIZE/6)   //16B


/*****************************************************************************/
/**                             Vovit structure                             **/
/*****************************************************************************/
/**
 * Represents Basic Metadata of Vovit Chunk.
 */
typedef struct _BasicMetadata{

#ifdef __LITTLE_ENDIANESS__
    uint8_t meta       : 8;
    uint8_t reserved   : 8;
    uint8_t port_width : 5;
    uint8_t port_speed : 3;
    uint8_t port_state : 8;
    uint32_t sync_cout : 32;
#endif /* __LITTLE_ENDIANESS__ */

#ifdef __BIG_ENDIANESS__
    uint8_t port_state : 8;   // [56:63]
    uint8_t port_width : 5;   // little endian in byte [48:52]
    uint8_t port_speed : 3;   // little endian in byte [53:55]
    uint8_t reserved   : 8;   // [40:47]
    uint8_t meta       : 8;   // [32:39]
    uint32_t sync_cout : 32;  // [0:31]
#endif /* __BIG_ENDIANESS__ */

} BasicMetadata;

/**
 * Represents Control bits of Vovit Chunk.
 */
typedef struct _Control {
#ifdef __LITTLE_ENDIANESS__
    uint8_t  sync_hdr_error : 6;
    uint8_t : 0;
    uint8_t  sync_hdr : 6;
    uint8_t : 0;
    uint8_t  is_valid : 6;

#endif /* __LITTLE_ENDIANESS__ */

#ifdef __BIG_ENDIANESS__
    uint8_t  is_valid : 6;          // 6 bit
    uint8_t : 0;
    uint8_t  sync_hdr : 6;          // 6 bit
    uint8_t : 0;
    uint8_t  sync_hdr_error : 6;    // 6 bit
#endif /* __BIG_ENDIANESS__ */
} Control;

/**
 * Represents Vovit Chunk.
 */
typedef struct _Chunk {
#ifdef __LITTLE_ENDIANESS__
    BasicMetadata  metadata;   // 8B
    uint8_t  data_block_zero     // 1B
    Control control;           // 3B
    uint8_t  data_block[92];     // 92B
    uint8_t  alignment_zero      // 1B
    uint8_t  data_block_last[3]  // 1B
    uint8_t  alignment[20];      // 20B

#endif /* __LITTLE_ENDIANESS__ */

#ifdef __BIG_ENDIANESS__
    BasicMetadata  metadata;                    // 8B
    Control control;                            // 3B
    uint8_t  data_block[DATA_BLOCK_SIZE];       // 96B
    uint8_t  alignment[21];                     // 21B
#endif /* __BIG_ENDIANESS__ */
} Chunk;


/*****************************************************************************/
/**                        Function declarations                            **/
/*****************************************************************************/
uint32_t calcSyncCount(BasicMetadata* meta);


#endif //TLP_PARSER_STRUCTS_H
