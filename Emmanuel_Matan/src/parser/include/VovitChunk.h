//
// Created by irinag on 2/1/2020.
//

#ifndef TLP_PARSER_VOVITCHUNK_H
#define TLP_PARSER_VOVITCHUNK_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "endianess.h"
#include "structs.h"
#include <iostream>


/*****************************************************************************/
/**                               Class                                     **/
/*****************************************************************************/
namespace pcie {

    /**
     * Represents physical Vovit Chunk.
     */
    class VovitChunk {
    private:
        uint8_t port_speed;
        uint8_t port_width;
        uint8_t buffer[VOVIT_CHUNK_SIZE];
        int chunk_idx;

    public:
        Chunk* vovit_chunk;

        VovitChunk(int chunk_idx, std::string file);

        uint32_t getSyncCount();

        uint8_t getPortWidth();

        uint8_t getPortSpead(){return port_speed;}

        /**
         * For Debug only.
         */
        void printAll();
    };


}  // namespace pcie




#endif //TLP_PARSER_VOVITCHUNK_H
