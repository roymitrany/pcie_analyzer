#!/bin/python

# Imports #
#Libraries
import sys
import random
import binascii
import re
try:
    import prettytable
except ImportError:
    sys.stderr.write('You have no prettytable installed on your machine\nrun \'pip install prettytable\' to install it\n')
    exit(1)
try:
    from colorama import init
    init(autoreset=True)
    from colorama import Fore, Back, Style
except ImportError:
    sys.stderr.write('You have no colorama installed on your machine\nrun \'pip install colorama\' to install it\n')
    exit(1)
#Files
from pcie_packets import TLP, DLLP, OS
from Encoder_Decoder import Encoder_Decoder


# Classes #

class Symbol:
                         #token          value       8b or 10b       color      symbol is valid   OS=0 TLP/DLLP=1   K symbol=0 regular = 1
    def __init__(self, symbol_token, symbol_value , print_format , symbol_color ,   is_valid    ,    is_data       ,      is_Ksymbol):
        assert type(symbol_value) == int
        self.symbol_token = symbol_token
        self.symbol_value = symbol_value
        self.print_format = print_format
        self.symbol_color = symbol_color
        self.is_valid = is_valid
        self.is_data = is_data
        self.is_Ksymbol = is_Ksymbol

    def __str__(self):
        return self.symbol_color + self.symbol_token + self.print_format.format(self.symbol_value) 
      
    

class Link:
                        #print 8b or 10b  , class encoder object ,     color    ,    color   ,  color   ,  color     ,  color    , lanes in link
    def __init__(self  ,   print_format   ,    encoder_object    ,    TLP_color , DLLP_color , OS_color , IDLE_color , PAD_color ,   width = 1):        
        self.table_width = width
        self.table = [[None] * width for i in range(50)]
        self.current_symbol = 0
        self.last_IDLE_symbol = -1
        self.last_SKP = 0
        self.print_format = print_format
        self.encoder_object = encoder_object
        self.TLP_color = TLP_color
        self.DLLP_color = DLLP_color
        self.OS_color = OS_color
        self.IDLE_color = IDLE_color
        self.PAD_color = PAD_color
   
                              #token         value          color      symbol is valid   OS=0 TLP/DLLP=1   K symbol=0 regular = 1
    def Insert_Symbol(self, symbol_token, symbol_value , symbol_color ,    is_valid    ,    is_data       ,      is_Ksymbol):
        # Insert Symbol Into Lane
        self.table[self.current_symbol // self.table_width][self.current_symbol % self.table_width] = Symbol(symbol_token, symbol_value , self.print_format , symbol_color , is_valid , is_data , is_Ksymbol )
        
        # Save Last IDLE and END Symbols Positions        
        if symbol_token == 'IDL':
            self.last_IDLE_symbol = self.current_symbol
        elif symbol_token == 'END':
            self.Last_END_Symbol = self.current_symbol
        
        # Advance to Next Symbol Slot
        self.current_symbol += 1
        self.last_SKP +=1
      
        # Reached Last Table Slot. Increase Table.
        if self.current_symbol / self.table_width >= len(self.table):
            self.table += [[None] * self.table_width for i in range(50)]#(self.num_of_rows_in_block)]
            
      
      
    def Insert_TLP(self, tlp):   
        curr_lane = self.current_symbol % self.table_width
  
        #--- Check conditions before adding packet ---#
      
        #Previous symbol is IDLE. STP location has to be lane 0.
        if ( (self.current_symbol - 1) == self.last_IDLE_symbol ):
            #Padding with not encoded PADs  
            while ( curr_lane != 0 ):                   
                self.Insert_Symbol('PAD', 0b11110111 , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width
        
        #Current lane is not 0 or 4. Need to add PAD.
        elif (curr_lane not in [0,4]):
            #Padding with not encoded PADs
            while ( curr_lane not in [0,4] ):
                self.Insert_Symbol('PAD', 0b11110111 , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)  
                curr_lane = self.current_symbol % self.table_width
        
        
        #--- Conditions checked. Inserting not encoded TLPs ---#

        packet_len = tlp[0][1] + 8  #(Header + Data) + (frame_start(1) + prefix(0) + sequence(2) + LCRC(4) + frmae_end(1)) = 8 
        
        #First symbol is frame_start                            
        self.Insert_Symbol(tlp[1][0] , tlp[1][1] , self.TLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
                
        #Inner symbols
        for symbol in range(2,packet_len):
            self.Insert_Symbol(tlp[symbol][0] , tlp[symbol][1] , self.TLP_color , '1' , '1' , '1')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
        
        #Last symbol is frame_end
        self.Insert_Symbol(tlp[packet_len][0] , tlp[packet_len][1] , self.TLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
        
        
        
    def Insert_Encoded_TLP(self , tlp , encoder_object):   
        curr_lane = self.current_symbol % self.table_width
  
        #--- Check conditions before adding packet ---#
      
        #Previous symbol is IDLE. STP location has to be lane 0.
        if ( (self.current_symbol - 1) == self.last_IDLE_symbol ):
            #Padding with encoded PADs
            while ( curr_lane != 0 ):             
                self.Insert_Symbol('PAD', encoder_object.Encode_Control_Symbol(0b11110111) , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width                
            
        
        #Current lane is not 0 or 4. Need to add PAD.
        elif (curr_lane not in [0,4]):
            #Padding with encoded PADs
            while ( curr_lane not in [0,4] ):
                self.Insert_Symbol('PAD', encoder_object.Encode_Control_Symbol(0b11110111) , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)  
                curr_lane = self.current_symbol % self.table_width           
            
        
      
        #--- Conditions checked. Inserting encoded TLPs ---#

        packet_len = tlp[0][1] + 8  #(Header + Data) + (frame_start(1) + prefix(0) + sequence(2) + LCRC(4) + frmae_end(1)) = 8 
        
        #First symbol is frame_start                            
        self.Insert_Symbol(tlp[1][0] , encoder_object.Encode_Control_Symbol(tlp[1][1]) , self.TLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
                
        #Inner symbols
        for symbol in range(2,packet_len):
            self.Insert_Symbol(tlp[symbol][0] , encoder_object.Encode_Data_Symbol(tlp[symbol][1]) , self.TLP_color , '1' , '1' , '1')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
        
        #Last symbol is frame_end
        self.Insert_Symbol(tlp[packet_len][0] , encoder_object.Encode_Control_Symbol(tlp[packet_len][1]) , self.TLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)
        
        
        
    def Insert_DLLP(self, dllp):
        curr_lane = self.current_symbol % self.table_width
        
        #--- Check conditions before adding packet ---#
        
        #Previous symbol is IDLE. SDP location has to be lane 0.
        if ( (self.current_symbol - 1) == self.last_IDLE_symbol ):
            #Padding with not encoded PADs
            while ( curr_lane != 0 ):
                self.Insert_Symbol('PAD', 0b11110111 , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width
        
        #Current Symbol is not 0 or 4. Need to Add PAD.
        elif (curr_lane not in [0,4]):
            #Padding with not encoded PADs
            while ( curr_lane not in [0,4] ):
                self.Insert_Symbol('PAD', 0b11110111 , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width
        
        
        #--- Conditions checked. Inserting not encoded DLLPs. They are 8 symbols long ---#
        
        #First symbol is frame_start                            
        self.Insert_Symbol(dllp[1][0] , dllp[1][1] , self.DLLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
                
        #Inner symbols
        for symbol in range(2,8):
            self.Insert_Symbol(dllp[symbol][0] , dllp[symbol][1] , self.DLLP_color , '1' , '1' , '1')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
        
        #Last symbol is frame_end
        self.Insert_Symbol(dllp[8][0] , dllp[8][1] , self.DLLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
    
    
    
    def Insert_Encoded_DLLP(self , dllp , encoder_object):
        curr_lane = self.current_symbol % self.table_width
        
        #--- Check conditions before adding packet ---#
        
        #Previous symbol is IDLE. SDP location has to be lane 0.
        if ( (self.current_symbol - 1) == self.last_IDLE_symbol ):
            #Padding with encoded PADs
            while ( curr_lane != 0 ):
                self.Insert_Symbol('PAD', encoder_object.Encode_Control_Symbol(0b11110111) , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width
        
        #Current Symbol is not 0 or 4. Need to Add PAD.
        elif (curr_lane not in [0,4]):
            #Padding with encoded PADs
            while ( curr_lane not in [0,4] ):
                self.Insert_Symbol('PAD', encoder_object.Encode_Control_Symbol(0b11110111) , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
                curr_lane = self.current_symbol % self.table_width
        
        
        #--- Conditions checked. Inserting encoded DLLPs. They are 8 symbols long ---#
        
        #First symbol is frame_start                            
        self.Insert_Symbol(dllp[1][0] , encoder_object.Encode_Control_Symbol(dllp[1][1]) , self.DLLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
                
        #Inner symbols
        for symbol in range(2,8):
            self.Insert_Symbol(dllp[symbol][0] , encoder_object.Encode_Data_Symbol(dllp[symbol][1]) , self.DLLP_color , '1' , '1' , '1')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
        
        #Last symbol is frame_end
        self.Insert_Symbol(dllp[8][0] , encoder_object.Encode_Control_Symbol(dllp[8][1]) , self.DLLP_color , '1' , '1' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)   
        
        
        
    def Insert_OS(self, os):
        OS_Len = os[0][1]

        # Align OS packet to lane 0 by adding not encoded PADs
        curr_lane = self.current_symbol % self.table_width
        while ( curr_lane != 0 ):
            self.Insert_Symbol('PAD', 0b11110111 , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)  
            curr_lane = self.current_symbol % self.table_width
        
        # Inserting not encoded OSs
        for symbol in range (1 , OS_Len):
            for lane in range(0,8):
               self.Insert_Symbol(os[symbol][0] , os[symbol][1] , self.OS_color , '1' , '0' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
    

    def Insert_Encoded_OS(self , os , encoder_object):
        OS_Len = os[0][1]

        # Align OS packet to lane 0 by adding encoded PADs
        curr_lane = self.current_symbol % self.table_width
        while ( curr_lane != 0 ):                 
            self.Insert_Symbol('PAD', encoder_object.Encode_Control_Symbol(0b11110111) , self.PAD_color , '1' , '0' , '0') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)  
            curr_lane = self.current_symbol % self.table_width
        
        # Inserting encoded OSs
        for symbol in range (1 , OS_Len):
            for lane in range(0,8):
               self.Insert_Symbol(os[symbol][0] , encoder_object.Encode_Control_Symbol(os[symbol][1]) , self.OS_color , '1' , '0' , '0')  #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1) 
 
    
    
    def Insert_Idle(self):
        for i in range(random.choice([4,8,12,24])):
            self.Insert_Symbol('IDLE', 0 , self.IDLE_color , '0' , '0' , '1') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)


    def Insert_Encoded_Idle(self , encoder_object):
        for i in range(random.choice([4,8,12,24])):
            self.Insert_Symbol('IDLE', encoder_object.Encode_Data_Symbol(0) , self.IDLE_color , '0' , '0' , '1') #(token , value , color , symbol is valid , OS=0 TLP/DLLP=1 , K symbol=0 regular = 1)
    
    
    def __str__(self):
        #Using PrettyTable Module
        pt = prettytable.PrettyTable(hrules = prettytable.ALL)
        pt.field_names = ['Lane {0}'.format(lane) for lane in range(self.table_width)]
        for i in range(len(self.table)):
            pt.add_row([str(table_entry) if table_entry is not None else '' for table_entry in self.table[i]])

        return pt.get_string()
    
    
        
def Transmit_Packets(packets , link_table):
    with open('./Output/Gen1_Packets_List.txt', 'w') as packets_file:
            for packet in packets:
              
                # Transmit a not encoded SKP OS every 100 Symbols (100 for debugging. Max Value is 1538)
                if ( link_table.last_SKP > 100 ):  
                    link_table.Insert_OS([('OS',4) , ('COM',0xbc) , ('SKP',0x1c) , ('SKP',0x1c) , ('SKP',0x1c)] )                   
                    link_table.last_SKP = 0
                
                
                # Transmit a not encoded TLP and add it to Gen1_Packets_List.txt #
                if packet[0][0] == 'TLP':
                    # Randomize Logical IDLE State
                    link_table.Insert_Idle()
                    # Transmit a not encoded TLP
                    link_table.Insert_TLP(packet)
                    # Add to Gen1_Packets_List.txt
                    packet_string = ''
                    for symbol in range(4,packet[0][1]+packet[0][2]+2):  #packet[0] contains TLP Header and Data Payload Lengths
                        packet_string += '{0:02x}'.format(packet[symbol][1])         
                    tlp_obj = TLP(packet_string)
                    packets_file.write(tlp_obj.display(True) + '\n')
                    
                    
                # Transmit a not encoded DLLP and add it to Gen1_Packets_List.txt #
                elif packet[0] == 'DLLP':
                    # Randomize Logical IDLE State
                    link_table.Insert_Idle()
                    # Transmit a not encoded DLLP
                    link_table.Insert_DLLP(packet)
                    # Add to Gen1_Packets_List.txt
                    packet_string = ''
                    for symbol in range(2,8):
                        packet_string += '{0:02x}'.format(packet[symbol][1])
                    dllp_obj = DLLP(packet_string)
                    packets_file.write(dllp_obj.display() + '\n')
                    
                     
                # Transmit a not encoded OS and add it to Gen1_Packets_List.txt #  
                else:
                    # Transmit a not encoded OS
                    link_table.Insert_OS(packet)
                    # Add to Gen1_Packets_List.txt
                    packet_string = ''
                    for symbol in range(1,packet[0][1]+1):
                        packet_string += '{0:02x}'.format(packet[symbol][1])
                    os_obj = OS(packet_string , '1')
                    packets_file.write(os_obj.display() + '\n')
                    
    

def Transmit_Encoded_Packets(packets , encoder_object , link_table):
    with open('./Output/Gen1_Packets_List.txt', 'w') as packets_file:
                for packet in packets:
                        
                    # Transmit an encoded SKP OS every 100 Symbols (100 for debugging. Max Value is 1538)
                    if ( link_table.last_SKP > 100 ):  
                        link_table.Insert_Encoded_OS( [('OS',4) , ('COM',0xbc) , ('SKP',0x1c) , ('SKP',0x1c) , ('SKP',0x1c)] , encoder_object )                                
                        link_table.last_SKP = 0
                    
                    
                    # Add TLP to Gen1_Packets_List.txt and Transmit an encoded TLP #
                    if packet[0][0] == 'TLP':
                        # Add to Gen1_Packets_List.txt
                        packet_string = ''
                        for symbol in range(4,packet[0][1]+packet[0][2]+2):  #packet[0] contains TLP Header and Data Payload Lengths
                            packet_string += '{0:02x}'.format(packet[symbol][1])         
                        tlp_obj = TLP(packet_string)
                        packets_file.write(tlp_obj.display(True) + '\n')
                        # Randomize Logical IDLE State
                        link_table.Insert_Encoded_Idle(encoder_object)
                        # Transmit an encoded TLP
                        link_table.Insert_Encoded_TLP(packet , encoder_object)
                        
                    
                    # Add DLLP to Gen1_Packets_List.txt and Transmit an encoded DLLP #
                    elif packet[0] == 'DLLP':
                        # Add to Gen1_Packets_List.txt
                        packet_string = ''
                        for symbol in range(2,8):
                            packet_string += '{0:02x}'.format(packet[symbol][1])
                        dllp_obj = DLLP(packet_string)
                        packets_file.write(dllp_obj.display() + '\n')
                        # Randomize Logical IDLE State
                        link_table.Insert_Encoded_Idle(encoder_object)
                        # Transmit an encoded DLLP
                        link_table.Insert_Encoded_DLLP(packet , encoder_object)
                         
                    # Add OS to Gen1_Packets_List.txt and Transmit an encoded OS #    
                    else:
                        # Add to Gen1_Packets_List.txt
                        packet_string = ''
                        for symbol in range(1,packet[0][1]+1):
                            packet_string += '{0:02x}'.format(packet[symbol][1])
                        os_obj = OS(packet_string , '1')
                        packets_file.write(os_obj.display() + '\n')
                        # Transmit an encoded OS
                        link_table.Insert_Encoded_OS(packet , encoder_object)
                        
    


# Main #
def main(packets , perform_encode , use_colors):
    
    #--- Initialize Empty Link With 8 Lanes ---#
    
    #If encoding requested, create encoder object for encoding packets and print with 10bit format
    if (perform_encode):
        print_format='\n{0:010b}' 
        encoder_object = Encoder_Decoder();        
    else:
        print_format='\n{0:08b}'
        encoder_object = None
    
    # Using colors or not
    if (use_colors):
        link_table = Link(print_format , encoder_object , Fore.RED , Fore.BLUE , Fore.GREEN , Fore.WHITE , Fore.YELLOW , 8)
    else:
        link_table = Link(print_format , encoder_object , '' , '' , '' , '' , '' , 8)
    
    
    #--- "Transmit" Packets Over Link ---#   
    
    # We can transmit encoded packets or not. This Code is almost duplicated 2 times for efficiency purposes #
    # when dealing with long lists(for 30,000 tlps we are saving 30,000 if statements) #
    
    if(perform_encode):
        Transmit_Encoded_Packets(packets , encoder_object , link_table)
    else:
        Transmit_Packets(packets , link_table)
    
                    
    #Write Link Table to File
    with open('./Output/Gen1_Link_Table.txt', 'w') as link_table_as_text:
        link_table_as_text.write(str(link_table))
        
    return (link_table)
    


if __name__ == "__main__":
    # Should never get here unless executing this script manually without Transmitter.py 
    print("Calling Link_Table_Gen_1.py without any packets")
    packets=[]
    encoder_object = Encoder_8b_10b();
    main(packets , False , encoder_object , False , False)
    