#!/bin/python

# Imports #
import binascii


# Main #
def main(link_table):
   
    VOVIT_packets=[]
    packet=''
    curr_vovit_symbol=0
    meta_data_is_valid=[]
    meta_data_is_data=[]
    meta_data_is_Ksymb=[]
    Data=[]
    
    #Iterating Link Symbols Table    
    for row in (link_table.table):
        for symbol in row:
            if (symbol != None):
                
                #Extracting symbols values and meta data from link table                
                meta_data_is_valid.append(symbol.is_valid)
                meta_data_is_data.append(symbol.is_data)
                meta_data_is_Ksymb.append(symbol.is_Ksymbol)
                Data.append('{0:02x}'.format(symbol.symbol_value))
                curr_vovit_symbol+=1
                    
                #Reached 64 Symbols. Build VOVIT Packet.
                if(curr_vovit_symbol==64):
                    # Converting lists to hex strings
                    meta_data_is_valid_as_string = ''.join(meta_data_is_valid)
                    meta_data_is_data_as_string = ''.join(meta_data_is_data)
                    meta_data_is_Ksymb_as_string = ''.join(meta_data_is_Ksymb)
                    meta_data_is_valid_as_hex='{0:016x}'.format(int(meta_data_is_valid_as_string,2))
                    meta_data_is_data_as_hex='{0:016x}'.format(int(meta_data_is_data_as_string,2))
                    meta_data_is_Ksymb_as_hex='{0:016x}'.format(int(meta_data_is_Ksymb_as_string,2))
                    Data_as_string = ''.join(Data)
                    
                    #VOVIT Packet is ready. Add to Packets List.
                    VOVIT_packets.append(meta_data_is_valid_as_hex + ' | ' + meta_data_is_data_as_hex + ' | ' + meta_data_is_Ksymb_as_hex + ' | ' + Data_as_string)
                    
                    #Prepare for next VOVIT packet 
                    curr_vovit_symbol=0
                    meta_data_is_valid.clear()
                    meta_data_is_data.clear()
                    meta_data_is_Ksymb.clear()
                    Data.clear()
                        
                        
    #Last VOVIT packet has less than 64 symbols
    if (curr_vovit_symbol != 0):
        #Pad with zeros
        while (curr_vovit_symbol < 64):
            meta_data_is_valid.append('0')
            meta_data_is_data.append('0')
            meta_data_is_Ksymb.append('0')
            Data.append('{0:02x}'.format(0))   
            curr_vovit_symbol+=1
        
        # Converting lists to hex strings
        meta_data_is_valid_as_string = ''.join(meta_data_is_valid)
        meta_data_is_data_as_string = ''.join(meta_data_is_data)
        meta_data_is_Ksymb_as_string = ''.join(meta_data_is_Ksymb)
        meta_data_is_valid_as_hex='{0:016x}'.format(int(meta_data_is_valid_as_string,2))
        meta_data_is_data_as_hex='{0:016x}'.format(int(meta_data_is_data_as_string,2))
        meta_data_is_Ksymb_as_hex='{0:016x}'.format(int(meta_data_is_Ksymb_as_string,2))
        Data_as_string = ''.join(Data)
        
        #VOVIT Packet is ready. Add to Packets List.
        VOVIT_packets.append(meta_data_is_valid_as_hex + ' | ' + meta_data_is_data_as_hex + ' | ' + meta_data_is_Ksymb_as_hex + ' | ' + Data_as_string)
     
    #Writing to files
    with open('./Output/Gen1_VOVIT_Packets.txt' , 'w') as vovit_as_text:
        vovit_as_text.write('\n'.join(VOVIT_packets))
    
    with open('./Output/Gen1_VOVIT_Packets.bin' , 'wb') as vovit_as_bin:
            vovit_as_bin.write(binascii.unhexlify(''.join([v.replace('|', '').replace(' ','') for v in VOVIT_packets])))



if __name__ == "__main__":
    # Should not get here unless executing this script manually without vovit_traffic_trace.py 
    print("Calling vovit_generator without any tlps,dllps or oss")
    tlps=[]
    dllps=[]
    oss=[]
    main(tlps,dllps,oss)
    