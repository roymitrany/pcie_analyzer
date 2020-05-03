//
// Created by irinag on 2/1/2020.
//

#ifndef TLP_PARSER_ENDIANESS_H
#define TLP_PARSER_ENDIANESS_H


/*****************************************************************************/
/**                      ENDIANNESS DEFINITION                              **/
/*****************************************************************************/
#ifndef  __BIG_ENDIANESS__
#define __BIG_ENDIANESS__

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "structs.h"
#include "tokens.h"


/*****************************************************************************/
/**                        Function declarations                            **/
/*****************************************************************************/
void getChunkDataBlock(Chunk* chunk, uint8_t buffer[DATA_BLOCK_SIZE]);

#endif /* __BIG_ENDIANESS__ */

#endif //TLP_PARSER_ENDIANESS_H
