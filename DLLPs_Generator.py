#!/bin/python

# Imports #
import random


def main(gen, dllps_count):
    
    dllps_packets_list = []
    dllp_types = [
                         (0b00000000 , 'Ack' ),
                         (0b00000010 , 'Data_Link_Feature' ),
                         (0b00010000 , 'Nack' ),
                         (0b00100000 , 'PM_Enter_L1' ),
                         (0b00100001 , 'PM_Enter_L23' ),
                         (0b00100011 , 'PM_Active_State_Request_L1' ),
                         (0b00100100 , 'PM_Request_Ack' ),
                         (0b00110001 , 'NOP' ),
                         (0b01000000 , 'InitFC1-P' ),
                         (0b01010000 , 'InitFC1-NP' ),
                         (0b01100000 , 'InitFC1-CPL' ),
                         (0b11000000 , 'InitFC2-P' ),
                         (0b11010000 , 'InitFC2-NP' ),
                         (0b11100000 , 'InitFC2-CPL' ),
                         (0b10000000 , 'UpdateFC-P' ),
                         (0b10010000 , 'UpdateFC-NP' ),
                         (0b10100000 , 'UpdateFC-CPL') ,
                       ]
    random_symbol=random.randrange(256)
    
    # Generating packet for gen1 #
    if( gen == '1'): 
        for i in range(dllps_count):
            dllp_packet=[]
            rand_type = random.randrange(len(dllp_types))
            
            # PACKETS #
            
            #ACK/NACK Packet
            if dllp_types[rand_type][1] in ['Ack', 'Nack']:       
               #First Symbol: According to packet type
               #ACK Packet
               if (dllp_types[rand_type][1] == 'Ack'):
                   dllp_packet.append(('ACK',dllp_types[rand_type][0]))
               #NACK Packet
               else:
                   dllp_packet.append(('NACK',dllp_types[rand_type][0]))    
               #Second Symbol: Reserved. Zero.               
               dllp_packet.append(('RESERVED',0))
               #Third - Fourth Symbols , Currently Random Symbols 
               dllp_packet.append(('RES/SEQ',random_symbol))
               dllp_packet.append(('SEQ',random_symbol))
               
            
            #FEATURE Packet            
            elif dllp_types[rand_type][1] in ['Data_Link_Feature']:
                #First Symbol: According to Feature packet
                dllp_packet.append(('FEATURE',dllp_types[rand_type][0]))
                #Second - Fourth Symbols , Currently Random Symbols 
                dllp_packet.append(('F.SUPPORT',random_symbol))             
                dllp_packet.append(('F.SUPPORT',random_symbol))
                dllp_packet.append(('F.SUPPORT',random_symbol))
                
            
            #InitFC Packet , Currently Random Packet          
            elif dllp_types[rand_type][1] in ['InitFC1-P', 'InitFC1-NP', 'InitFC1-CPL', 'InitFC2-P', 'InitFC2-NP', 'InitFC2-CPL', 'UpdateFC-P', 'UpdateFC-NP', 'UpdateFC-CPL']:
                #First Symbol: According to packet type
                if (dllp_types[rand_type][1] in ['InitFC1-P', 'InitFC1-NP', 'InitFC1-CPL']):
                    dllp_packet.append(('InitFC1',dllp_types[rand_type][0]))
                elif (dllp_types[rand_type][1] in ['InitFC2-P', 'InitFC2-NP', 'InitFC2-CPL']):
                    dllp_packet.append(('InitFC2',dllp_types[rand_type][0]))
                else:
                    dllp_packet.append(('UpdateFC',dllp_types[rand_type][0]))
                #Second - Fourth Symbols
                dllp_packet.append(('HdrFC',random_symbol))
                dllp_packet.append(('HdrFC/DataFC',random_symbol))
                dllp_packet.append(('DataFc',random_symbol))
                
                
            #NOP Packet              
            elif dllp_types[rand_type][1] in ['NOP']:
                #First Symbol: According to NOP packet
                dllp_packet.append(('NOP',dllp_types[rand_type][0]))
                #Second - Fourth Symbols , Currently Random Symbols               
                dllp_packet.append(('Arbitrary',random_symbol))
                dllp_packet.append(('Arbitrary',random_symbol))
                dllp_packet.append(('Arbitrary',random_symbol))
                
                
            #PM Packet             
            elif dllp_types[rand_type][1] in ['PM_Enter_L1', 'PM_Enter_L23', 'PM_Active_State_Request_L1', 'PM_Request_Ack']:
                #First Symbol: According to PM packet
                dllp_packet.append(('PM',dllp_types[rand_type][0]))
                #Second - Fourth Symbols: Reserved. Zeros.               
                dllp_packet.append(('Reserved',0))
                dllp_packet.append(('Reserved',0))
                dllp_packet.append(('Reserved',0))
       
            
            #All Packets: CRC, Last Two Symbols , Currently Random Symbols
            dllp_packet.append(('CRC',random_symbol))
            dllp_packet.append(('CRC',random_symbol))
            
            #Each DLLP Will Contain a Tuple Header of ('DLLP') for Later Process by Symbol Table
            dllp_packet.insert( 0 , ( 'DLLP' ) )
            
            #DLLP Ready. Add to DLLPs List.
            dllps_packets_list.append(dllp_packet)
    
    
    
    # Generating packet for gen3 #    
    else:
        for i in range(dllps_count):
            rand_dllp = random.randrange(len(dllp_types))
            dllp = dllp_types[rand_dllp][0] << 24
            
            if dllp_types[rand_dllp][1] in ['Ack', 'Nack']:
                dllp |= random.randrange(4096)
                
            elif dllp_types[rand_dllp][1] in ['Data_Link_Feature']:
                dllp |= 1
                
            elif dllp_types[rand_dllp][1] in ['InitFC1-P', 'InitFC1-NP', 'InitFC1-CPL', 'InitFC2-P', 'InitFC2-NP', 'InitFC2-CPL', 'UpdateFC-P', 'Update-NP', 'Update-CPL']:
                dllp |= (random.randrange(256) << 12) | random.randrange(1024)
                
            elif dllp_types[rand_dllp][1] in ['NOP']:
                dllp |= random.randrange(0xffffff + 1)
                
            elif dllp_types[rand_dllp][1] in ['PM_Enter_L1', 'PM_Enter_L23', 'PM_Active_State_Request_L1', 'PM_Request_Ack']:
                pass
                        
            dllps_packets_list.append('{0:08x}'.format(dllp))
        
        
    return dllps_packets_list    



if __name__ == "__main__":
    # Should not get here unless executing this script manually without vovit_traffic_trace.py 
    print("Calling randomize_dllps with dlps_count=3")
    main(3)