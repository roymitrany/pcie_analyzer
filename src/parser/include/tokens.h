//
// Created by irinag on 2/3/2020.
//

#ifndef TLP_PARSER_TOCKENS_H
#define TLP_PARSER_TOCKENS_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <cstdint>

/*****************************************************************************/
/**                              Defines                                    **/
/*****************************************************************************/
#define TOKEN_BUFFER_SIZE 4
#define IDL 0x0
#define GET_TLP_LENGTH(msb,lsb) (((msb) <<4) | (lsb))

#ifdef __LITTLE_ENDIANESS__
#define EDS : 0x0090801f
#define EDB : 0xc0c0c0c0
#define SDP : 0xacf0
#endif /* __LITTLE_ENDIANESS__ */
#ifdef __BIG_ENDIANESS__
#define EDS 0x1f809000
#define EDB 0xc0c0c0c0
#define SDP 0xf0ac
#endif /* __BIG_ENDIANESS__ */

/*****************************************************************************/
/**                              Tokens types                               **/
/*****************************************************************************/
/**
 * Token types of PCIe GEN3
 */
enum _TokenType {
    STP_TYPE = 0,
    EDS_TYPE = 1,
    EDB_TYPE = 2,
    SDP_TYPE = 3,
    IDL_TYPE = 4,
    ERR_TYPE = 5
};


/*****************************************************************************/
/**                             Tokens structures                           **/
/*****************************************************************************/

/**
 * GEN3 Stp Token structure
 */
typedef struct _Stp {

#ifdef __LITTLE_ENDIANESS__

#endif /* __LITTLE_ENDIANESS__ */

#ifdef __BIG_ENDIANESS__
    uint8_t             : 4;
    uint8_t length_lsb  : 4;   // [4:7]     // STP+TLP length in DWs
    uint8_t length_msb  : 7;   // [8:14]
    uint8_t parity      : 1;   // [15:15]
    uint8_t seq_num_msb : 4;   // [16:19]
    uint8_t crc         : 4;   // [20:23]
    uint8_t seq_num_lsb : 8;   // [24:31]

#endif /* __BIG_ENDIANESS__ */

} Stp;

/*****************************************************************************/
/**                        Function declarations                            **/
/*****************************************************************************/
int getTokenType(uint8_t* buffer);

#endif //TLP_PARSER_TOCKENS_H
