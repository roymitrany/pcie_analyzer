//
// Created by irinag on 2/1/2020.
//

#ifndef TLP_PARSER_EXEPTIONS_H
#define TLP_PARSER_EXEPTIONS_H


/*****************************************************************************/
/**                                Exceptions                               **/
/*****************************************************************************/
namespace pcie {

    class PcieException {};

    class LogicPacketStreamInvlidSpeedException : public PcieException {
        private:
            uint32_t byte_offset;
            uint8_t port_speed;
        public:
            LogicPacketStreamInvlidSpeedException(uint32_t byte_offset, uint8_t port_speed):
                                                    byte_offset(byte_offset), port_speed(port_speed){}

            uint8_t getPortSpeed(){ return port_speed;}

            uint32_t getByteOffset(){ return byte_offset;}
    };

    class VovitChunkEOFException : public PcieException{};
    class VovitChunkInvalidArgsException : public PcieException {};

}  //  namespace pcie



#endif //TLP_PARSER_EXEPTIONS_H
