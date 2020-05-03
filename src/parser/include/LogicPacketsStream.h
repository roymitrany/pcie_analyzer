//
// Created by irinag on 2/6/2020.
//

#ifndef TLP_PARSER_LOGICDATASTREAM_H
#define TLP_PARSER_LOGICDATASTREAM_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <string>
#include "LogicChunk.h"


/*****************************************************************************/
/**                               Classes                                   **/
/*****************************************************************************/
namespace pcie {

    /**
     * Represents logic stream of link data transfer.
     */
    class LogicPacketsStream{
    public:

        /**
         * Retrieves next link packet in logic order.
         */
        virtual void OnNext(){}

        /**
         * Returnt reference to current packet.
         */
        virtual uint8_t* getCurrPacket(){}

        /**
         * Returnt size of current packet.
         */
        virtual uint32_t getCurrPacketSizeBytes(){}

        virtual ~LogicPacketsStream(){}
    };


namespace gen3 {

    enum {
        TX = 0,
        RX = 1
    };

    /**
     * Represents logic stream of PCIe GEN3.
     */
    class LogicPacketsStream : public pcie::LogicPacketsStream{

    private:
        std::string file[2];
        uint32_t curr_logic_data_byte_idx[2];
        LogicChunk* currPhyChunk[2];
        int curr_rx_tx_;

        uint8_t* curr_packet = nullptr;
        uint16_t curr_packet_size_dw = 0;
        uint8_t* buffer = nullptr;

        void updateNextSyncCount();
        void getNextLogicChunk();
        void getNextXBytes(uint32_t x);
        void verifyPortSpeed(int rx_tx_);

        public:

        LogicPacketsStream(std::string txFile, std::string rxFile);
        ~LogicPacketsStream() override;
        void OnNext() override;  // free mem
        uint8_t* getCurrPacket() override;
        uint32_t getCurrPacketSizeBytes() override;

    };

}  // namespace gen3
}  // namespace pcie

#endif //TLP_PARSER_LOGICDATASTREAM_H
