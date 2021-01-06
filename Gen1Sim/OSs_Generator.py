#!/bin/python

# Imports #
import random


def main(gen,oss_count):
    
    os_packets_list = []
    random_symbol=random.randrange(256)
     
    # Generating packet for gen1 #
    if( gen == '1'):
    
        os_types = [            
                 [0x1c , 'SKP'],
                 [0x66 , 'EIOS'] ,
                 [0x1e , 'TS1'] ,
                 [0x2d , 'TS2'] ,
                 [0x3c , 'FTS']
               ]
    
        for i in range(oss_count):
            os_packet = []
            rand_type = random.randrange(len(os_types))
            
            #First Symbol is Always 'COM'
            os_packet.append(('COM',0xbc)) # COM = 0xbc
            
            
            # PACKETS #
            
            #SKP Packet
            if( os_types[rand_type][1] == 'SKP' ):
                for curr_symbol in range(3):
                    os_packet.append(('SKP',0x1c)) # SKP = 0x1c
                packet_length = 4  
                   
            #FTS Packet        
            elif( os_types[rand_type][1] == 'FTS' ):
                for curr_symbol in range(3):
                    os_packet.append(('FTS',0x3c)) # FTS = 0x3c
                packet_length = 4
                
            #Electrical Idle Packet        
            elif( os_types[rand_type][1] == 'EIOS' ):
                for curr_symbol in range(3):
                    os_packet.append(('IDL',0x7c)) # IDL = 0x7c
                packet_length = 4
               
            #TS1 Packet
            elif( os_types[rand_type][1] == 'TS1' ):
                # 2nd - 3rd Symbols , Currently PAD
                for curr_symbol in range(2):
                    os_packet.append(('PAD',0xf7)) # PAD = 0xf7
                    
                # 4th - 6th Symbols , Currently Random
                for curr_symbol in range(3):
                    os_packet.append(('RAND',random_symbol))
                    
                # 7th - 16th Symbols
                for curr_symbol in range(10):
                    os_packet.append(('TS1_ID',0x4a)) # TS1 ID = 0x4a
                packet_length = 16    
                    
            #TS2 Packet
            elif( os_types[rand_type][1] == 'TS2' ):
                # 2nd - 3rd Symbols , Currently PAD
                for curr_symbol in range(2):
                    os_packet.append(('PAD',0xf7)) # PAD = 0xf7
                    
                # 4th - 6th Symbols , Currently Random
                for curr_symbol in range(3):
                    os_packet.append(('RAND',random_symbol))
                    
                # 7th - 16th Symbols
                for curr_symbol in range(10):
                    os_packet.append(('TS2_ID',0x45)) # TS2 ID = 0x45
                packet_length = 16    
            
            #Each OS Will Contain a Tuple Header of ( 'OS' , OS Length) for Later Process by Symbol Table
            os_packet.insert( 0 , ( 'OS' , packet_length ) )
            
            #OS is ready. Add to OSs List.
            os_packets_list.append(os_packet)
    
    
    # Generating packet for gen3 #
    else:
        os_types = [
                     [0xaa , 'SKP'],
                     [0x00 , 'EIEOS'],
                     [0x66 , 'EIOS'] ,
                     [0xe1 , 'SDS'] ,
                     [0x1e , 'TS1'] ,
                     [0x2d , 'TS2'] 
                   ]
    
        for i in range(oss_count):
            rand_type = random.randrange(len(os_types))
            os_packet = os_types[rand_type][0] << 24
            os_packets_list.append('{0:08x}000000000000000000000000'.format(os_packet))
           
    return os_packets_list
    
    
if __name__ == "__main__":
    # Should not get here unless executing this script manually without vovit_traffic_trace.py 
    print("Calling randomize_oss with oss_count=3")
    main(3)