//
// Created by irinag on 2/3/2020.
//

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "../include/endianess.h"
#include <cstdio>


typedef struct _TokenTmpl {
    uint32_t dw;
} TokenTmplt;


/*****************************************************************************/
/**                        Function implementation                          **/
/*****************************************************************************/
/**
 * Returns Token type stored at buffer
 */
int getTokenType(uint8_t* buffer){
#ifdef __LITTLE_ENDIANESS__
    uint32_t token = ((TokenTmplt*) buffer)->dw;
#endif /* __LITTLE_ENDIANESS__ */
#ifdef __BIG_ENDIANESS__
    uint32_t token = __builtin_bswap32(((TokenTmplt*) buffer)->dw);
#endif /* __BIG_ENDIANESS__ */

   if((token&0xff000000) == IDL) return IDL_TYPE;
   if(token == EDS) return EDS_TYPE;
   if(token == EDB) return EDB_TYPE;
   if(token == SDP) return SDP_TYPE;
   return STP_TYPE;
}