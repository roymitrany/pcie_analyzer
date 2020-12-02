#!/bin/python

# Import #
import random

# Functions #
def rand_bdf0():
    bdfs = [0x0400, 0x0401, 0x8000, 0x8001, 0x4200, 0xaf08]
    return bdfs[random.randrange(len(bdfs))]

def rand_bdf1():
    bdfs = [0x0304, 0x0301, 0x0201, 0x0200, 0x0500, 0x0510]
    return bdfs[random.randrange(len(bdfs))]

def rand_data():
    data = [0xfa000000, 0x8a000040, 0x00004000, 0x00808800, 0x0000aa00, 0x40400000, 0x00af0000, 0x45004600, 0x8080a000, 0xa8000000, 0x000000a0, 0x00000020, 0xffffffff, 0xffff0000, 0x0000ffff]
    return data[random.randrange(len(data))]

def rand_address(which):
    address0 = [0xaf000000, 0xa0000000, 0xcc000000, 0xca000000, 0xab000000, 0x32000000, 0xe2000000]
    address1 = [0x00000032, 0x0000000f, 0x000000e2, 0x00000000, 0x000000af, 0x00003200, 0x00000000]
    return address0[random.randrange(len(address0))] if which else address1[random.randrange(len(address1))]

def rand_intx():
    intx = [0b00100000, 0b00100001, 0b00100010, 0b00100011, 0b00100100, 0b00100101, 0b00100110, 0b00100111]
    return intx[random.randrange(len(intx))]

def rand_err():
    err = [0b00110000, 0b00110001, 0b00110011]
    return err[random.randrange(len(err))]

def rand_cpl_stat():
    cpl_stat = [0b000, 0b001, 0b010, 0b100]
    return cpl_stat[random.randrange(len(cpl_stat))]


# Main #
def main(gen,tlps_count):
    
    tlp_packets_list = []
    tlp_types = [
            (0b00001010, 'Cpl'),
            (0b01001010, 'CplD'),
            (0b00000100, 'CfgRd0'),
            (0b01000100, 'CfgWr0'),
            (0b00000101, 'CfgRd1'),
            (0b01000101, 'CfgWr1'),
            (0b00000000, 'MemRd32'),
            (0b01000000, 'MemWr32'),
            (0b00100000, 'MemRd64'),
            (0b01100000, 'MemWr64'),
            (0b00110100, 'MessageAssertDeassert'),
            (0b00110101, 'MessagePME_TO_Ack'),
            (0b00110011, 'MessagePME_Turn_Off'),
            (0b00110000, 'MessageError'),
            ]
                
    # Generating packet for gen1 #
    if( gen == '1'):
        for i in range(tlps_count):
            tlp_packet = [0]*12
            rand_type = random.randrange(len(tlp_types))
            rand_symbol = random.randrange(256) 
            
            # PACKETS #
            
            #1: Cpl / CplD Packet
            if( tlp_types[rand_type][1] in ['Cpl', 'CplD'] ):
                
                #Cpl Packet. Doesn't have a Data Payload
                if( tlp_types[rand_type][1] == 'Cpl' ):
                    data_payload_length = 0
                    
                    #First Symbol According to Cpl Packet Format.
                    tlp_packet[0]=(('Cpl',tlp_types[rand_type][0]))
                
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('Header',rand_symbol))
                    
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    symbol = ( random.randrange(64) << 2 )
                    tlp_packet[2]=(('Header|Length',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    tlp_packet[3]=(('Length',0))
                
                #CplD Packet. Has a Data Payload
                else:
                    data_payload_length = random.randrange(5) #data payload length is currently 5 (max) for debug only. Needs to be 1024. 
                    
                    #First Symbol According to CplD Packet Format.
                    tlp_packet[0]=(('CplD',tlp_types[rand_type][0]))
                    
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('Header',rand_symbol))
                    
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    data_payload_length_two_msb = data_payload_length >> 8
                    symbol = ( (random.randrange(64) << 2) | data_payload_length_two_msb )
                    tlp_packet[2]=(('Header|Length',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    symbol = data_payload_length & 0xff
                    tlp_packet[3]=(('Length',symbol))
                
                    #CplD Contains Data, Currently Random
                    for curr_symbol in range(data_payload_length):
                        tlp_packet.append(('Data',rand_symbol)) 
                    
                #Fifth - Twelfth Symbols , Currently Random Symbols 
                for curr_symbol in range(4,12):
                    tlp_packet[curr_symbol]=(('Header',rand_symbol))
                                 
       
            #2: CfgRd0 / CfgRd1 / CfgWr0 / CfgWr1 Packet
            elif( tlp_types[rand_type][1] in ['CfgRd0', 'CfgRd1', 'CfgWr0', 'CfgWr1'] ):
                
                #CfgRd Packet. Doesn't have a Data Payload.
                if( tlp_types[rand_type][1] in ['CfgRd0', 'CfgRd1']):
                    data_payload_length = 0
                    
                    #First Symbol According to CfgRd Packet Format.
                    tlp_packet[0]=(('CfgRd',tlp_types[rand_type][0]))
                    
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('Header',rand_symbol))
                    
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    symbol = ( random.randrange(64) << 2 )
                    tlp_packet[2]=(('Header|Length',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    tlp_packet[3]=(('Length',0))
                
                #CfgWr Packet. Has a Data Payload
                else:
                    data_payload_length = random.randrange(3) #data_payload_length is currently 3 for debug only. Needs to be 1024.
                      
                    #First Symbol According to CfgWr Packet Format.
                    tlp_packet[0]=(('CfgWr',tlp_types[rand_type][0]))
                    
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('Header',rand_symbol))
                      
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    data_payload_length_two_msb = data_payload_length >> 8
                    symbol = ( (random.randrange(64) << 2) | data_payload_length_two_msb )
                    tlp_packet[2]=(('Header|Length',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    symbol = data_payload_length & 0xff
                    tlp_packet[3]=(('Length',symbol))
                
                    #CfgWr0 / CfgWr1 Contain Data, Currently Random
                    for curr_symbol in range(data_payload_length):
                        tlp_packet.append(('Data',rand_symbol)) 
                    
                #Fifth - Twelfth Symbols , Currently Random Symbols 
                for curr_symbol in range(4,12):
                    tlp_packet[curr_symbol]=(('Header',rand_symbol))
                
                
            #3: MemRd32 / MemRd64 / MemWr32 / MemWr64 Packet
            elif( tlp_types[rand_type][1] in ['MemRd32', 'MemRd64', 'MemWr32', 'MemWr64'] ):
                
                #MemRd Packet. Doesn't have a Data Payload
                if( tlp_types[rand_type][1] in ['MemRd32', 'MemRd64']):
                    data_payload_length = 0
                    
                    #First Symbol, According to MemRd Packet Format
                    tlp_packet[0]=(('MemRd',tlp_types[rand_type][0]))
                
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('Header',rand_symbol))
                    
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    symbol = ( random.randrange(64) << 2 )
                    tlp_packet[2]=(('Header|Length',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    tlp_packet[3]=(('Length',0))
                
                #MemWr Packet. Has a Data Payload
                else:
                    data_payload_length = random.randrange(3) #data_payload_length is currently 3 for debug only. Needs to be 1024.
                      
                    #First Symbol, According to Packet Type
                    tlp_packet[0]=(('MemWr',tlp_types[rand_type][0]))
                
                    #Second Symbol, Currently Random
                    tlp_packet[1]=(('T9|TC|T8|Attr|LN|TH',rand_symbol))  
                      
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    data_payload_length_two_msb = data_payload_length >> 8
                    symbol = ( (random.randrange(64) << 2) | data_payload_length_two_msb )
                    tlp_packet[2]=(('TD|EP|Attr|AT|Len',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    symbol = data_payload_length & 0xff
                    tlp_packet[3]=(('Length',symbol))
                
                    #MemWr32 / MemWr64 Contain Data, Currently Random
                    for curr_symbol in range(data_payload_length):
                        tlp_packet.append(('Data',rand_symbol)) 
                    
                #Fifth - Twelfth Symbols , Currently Random Symbols 
                for curr_symbol in range(4,12):
                    tlp_packet[curr_symbol]=(('Header',rand_symbol))
                
                #MemRd64 / MemWr64 are 16 symbols long. Currently 13th-16th symbols are random symbols.
                if( tlp_types[rand_type][1] in ['MemRd64', 'MemWr64']):
                    for i in range(4):
                        tlp_packet.insert(12,('Address',rand_symbol))
                        
                                        
            #4: MessageAssertDeassert / MessagePME_TO_Ack / MessagePME_Turn_Off / MessageError Packet
            elif( tlp_types[rand_type][1] in ['MessageAssertDeassert', 'MessagePME_TO_Ack', 'MessagePME_Turn_Off', 'MessageError'] ):
                
                #First Symbol, According to Packet Type
                tlp_packet[0]=(('Message',tlp_types[rand_type][0]))
                
                #Second Symbol, Currently Random
                tlp_packet[1]=(('Header' , rand_symbol))
                
                #Packet doesn't have a Data Payload
                if( tlp_types[rand_type][1] in ['MessageAssertDeassert', 'MessagePME_TO_Ack', 'MessagePME_Turn_Off', 'MessageError']):
                    data_payload_length = 0
                    
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    symbol = ( random.randrange(64) << 2 )
                    tlp_packet[2]=(('Header|Len',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    tlp_packet[3]=(('Len',0))
                
                #Packet has a Data Payload
                else:
                    data_payload_length = random.randrange(3) #data_payload_length is currently 3 for debug only. Needs to be 1024.
                      
                    #Third Symbol, Currently Random with Consideration in Packet Length
                    data_payload_length_two_msb = data_payload_length >> 8
                    symbol = ( (random.randrange(64) << 2) | data_payload_length_two_msb )
                    tlp_packet[2]=(('Header|Len',symbol))
                    
                    #Fourth Symbol, According to Packet Length
                    symbol = data_payload_length & 0xff
                    tlp_packet[3]=(('Len',symbol))
                
                    # Contain Data, Currently Random
                    for curr_symbol in range(data_payload_length):
                        tlp_packet.append(rand_symbol) 
                    
                #Fifth - Twelfth Symbols , Currently Random Symbols 
                for curr_symbol in range(4,12):
                    tlp_packet[curr_symbol]=(('Random',rand_symbol))
                
                #MessageAssertDeassert / MessagePME_TO_Ack / MessagePME_Turn_Off / MessageError are 16 symbols long. Currently 13th-16th symbols are random symbols.
                for i in range(4):
                    tlp_packet.insert(12,('Random',rand_symbol))
            
            
            #Each TLP Will Contain a Tuple Header of ('TLP' , TLP Length , Data Payload Length) for Later Process by Symbol Table
            tlp_packet.insert( 0 , ( 'TLP' , len(tlp_packet) , data_payload_length) )
            
            #TLP Ready. Add to TLPs List.
            tlp_packets_list.append(tlp_packet)   
            
            
    # Generating packet for gen3
    else:
        for i in range(tlps_count):
            tlp_packet = []
            rand_type = random.randrange(len(tlp_types))
            
            for i in range(3):
                tlp_packet.append(0)
            tlp_packet[0] |= tlp_types[rand_type][0] << 24
            
            if tlp_types[rand_type][1] in ['Cpl', 'CplD']:
                tlp_packet[1] |= rand_bdf0() << 16
                tlp_packet[2] |= rand_bdf1() << 16
                tlp_packet[1] |= rand_cpl_stat() << 13
                tlp_packet[1] |= random.randrange(4096)
                if tlp_types[rand_type][1] == 'CplD':
                    length = 1 + random.randrange(24)
                    tlp_packet[0] |= length
                    for i in range(length):
                        tlp_packet.append(rand_data())
                    
            elif tlp_types[rand_type][1] in ['CfgRd0', 'CfgRd1', 'CfgWr0', 'CfgWr1']:
                tlp_packet[0] |= 0x1
                tlp_packet[1] |= rand_bdf0() << 16
                tlp_packet[2] |= rand_bdf1() << 16
                
                tlp_packet[1] |= random.randrange(256) << 8
                tlp_packet[1] |= random.randrange(16)
                tlp_packet[2] |= random.randrange(1024) << 2
                
                if tlp_types[rand_type][1] in ['CfgWr0', 'CfgWr1']:
                    tlp_packet.append(rand_data())
                            
            elif tlp_types[rand_type][1] in ['MemRd32', 'MemRd64', 'MemWr32', 'MemWr64']:
                tlp_packet[1] |= rand_bdf0() << 16
                tlp_packet[1] |= random.randrange(256) << 8
                tlp_packet[1] |= random.randrange(256)
                
                length = 1 + random.randrange(24)
                tlp_packet[0] |= length
                
                if tlp_types[rand_type][1] in ['MemRd64', 'MemWr64']:
                    tlp_packet[2] |= rand_address(False)
                    tlp_packet.append(rand_address(True))
                else:
                    tlp_packet[2] |= rand_address(True)
                
                if tlp_types[rand_type][1] in ['MemWr32', 'MemWr64']:
                    for i in range(length):
                        tlp_packet.append(rand_data())
                    
            elif tlp_types[rand_type][1] in ['MessageAssertDeassert', 'MessagePME_TO_Ack', 'MessagePME_Turn_Off', 'MessageError']:
                tlp_packet.append(0)
                tlp_packet[1] |= rand_bdf1() << 16
                tlp_packet[1] |= random.randrange(256) << 8
                
                if tlp_types[rand_type][1] == 'MessageAssertDeassert':
                    tlp_packet[1] |= rand_intx()
                elif tlp_types[rand_type][1] == 'MessagePME_TO_Ack':
                    tlp_packet[1] |= 0b00011011
                elif tlp_types[rand_type][1] == 'MessagePME_Turn_Off':
                    tlp_packet[1] |= 0b00011001
                elif tlp_types[rand_type][1] == 'MessageError':
                    tlp_packet[1] |= rand_err()
        
            tlp_packets_list.append(''.join(['{0:08x}'.format(dword) for dword in tlp_packet]))
    return tlp_packets_list


if __name__ == "__main__":
    # Should not get here unless executing this script manually without vovit_traffic_trace.py 
    print("Calling randomize_tlps with tlps_count=3")
    main(3)