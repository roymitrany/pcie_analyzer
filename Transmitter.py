#!/bin/python

# Imports #

#Libraries
import sys
import random
import argparse
import zlib
import os
if not os.path.exists('./Output'):
    os.makedirs('./Output')
#Files
import TLPs_Generator
import DLLPs_Generator
import OSs_Generator
import Gen1_Link_Table
import Gen1_VOVIT_Generator
import Gen3_VOVIT_Generator



# Functions #

def Parse_Arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-g", "--Gen", required=True,  help="PCIe Gen 1 or 3 Packets")
    parser.add_argument("-t", "--tlps_count", required=False, help="Generated TLPs count")
    parser.add_argument("-d", "--dllps_count", required=False, help="Generated DLLPs count")
    parser.add_argument("-o", "--oss_count", required=False, help="Generated OSs count")
    parser.add_argument("-e", "--Encode", required=False, action="store_true", default=False , help="Encode Gen1 Packets or Not")
    parser.add_argument("-c", "--Colors", required=False, action="store_true", default=False , help="Use Colors")
    parser.add_argument("-l", "--Link_Table", required=False, action="store_true", default=False , help="Print Link Table")
    
    return vars(parser.parse_args())
  
    
def Add_Seq_Add_LCRC(TLPs_list):
    seq_num=0
    for TLP in TLPs_list:
        # Add sequence number.
        TLP.insert(1 , ('Seq' , (seq_num >> 8)) )
        TLP.insert(2 , ('Seq' , (seq_num & 0xff)) )
        
        # Append 32bit LCRC. Currently zeros
        TLP.append(('LCRC' , 0))
        TLP.append(('LCRC' , 0))
        TLP.append(('LCRC' , 0))
        TLP.append(('LCRC' , 0))
        
        # Sequence number is mod 4096
        seq_num+=1
        if (seq_num == 4096):
            seq_num = 0
            
    

def Frame_Packet(Packets_List , Packets_Type):
    #Frame Start
    if Packets_Type == 'TLP':
        frame_start = ('STP',0xfb) # STP = 0xfb
    else:
        frame_start = ('SDP',0x5c) # SDP = 0x5c
    #Frame End
    for Packet in Packets_List:
        Packet.insert(1 , frame_start)
        Packet.append(('END',0xfd)) # END = 0xfd
    


#DEBUG NEED TO BE REMOVED
################################################################################################################
def Print_Binary(Packets_List , perform_encode):
    if ( perform_encode == 'Encode' ):
        for Packet in Packets_List:
            print('Packet is:\n')
            print("{}{}{} ,".format('[' , Packet[0] , ']') , end = ' ')
            for Symbol in range(1,len(Packet)):
                print("({},[{}]) ,".format(Packet[Symbol][0] , format(Packet[Symbol][1],'010b') ) , end=' ')
            print('\n\n')   
    else:
        for Packet in Packets_List:
            print('Packet is:\n')
            print("{}{}{} ,".format('[' , Packet[0] , ']') , end=' ')
            for Symbol in range(1,len(Packet)):
                print("({},[{}]) ,".format(Packet[Symbol][0] , format(Packet[Symbol][1],'08b') ) , end=' ')
            print('\n\n')
#################################################################################################################
            
def main(): 
    #Get Script's Arguments
    args = Parse_Arguments()
    tlps_count = int( 0 if args['tlps_count'] is None else args['tlps_count'] )
    dllps_count = int( 0 if args['dllps_count'] is None else args['dllps_count'] )
    oss_count = int( 0 if args['oss_count'] is None else args['oss_count'] )
    perform_encode = args['Encode']
    use_colors = args['Colors']
    print_link_table = args['Link_Table']
        
    #Validate Arguments
    if ( (tlps_count == 0) and (dllps_count == 0) and (oss_count == 0) ) :  
        sys.stderr.write("At least one packet type count should be provided (run {0} -h for help)\n".format(sys.argv[0]))
        exit(1)
    elif ( (tlps_count < 0) or (dllps_count < 0) or (oss_count < 0) ) :  
        sys.stderr.write("Negative count is not valid (run {0} -h for help)\n".format(sys.argv[0]))
        exit(1)

    ################################### User Chose Gen1 ##########################################
    #                                                                                            #
    # 1. Transaction Layer:  (1) Generate TLPs                                                   #
    #                                                                                            #
    # 2. Data Link Layer:    (1) Add TLPs Sequence Number and LCRC                               #
    #                        (2) Generate DLLPs                                                  #
    #                                                                                            #
    # 3. Physical Layer:     (1) Frame TLPs                                                      #
    #                        (2) Frame DLLPs                                                     #
    #                        (3) Generate OSs                                                    #
    #                        (4) Shuffle Packets Order (for simulation and debug purposes only)  #
    #                        (5) Draw Link Table (with encoded packets if requested)             #
    #                        (6) Generate VOVIT Packets                                          #
    #                                                                                            #
    ##############################################################################################
    if (args['Gen'] == '1'):
        print('PCIe Gen1 Packets')    
           
        #------------- Transaction Layer -------------#
        
        #(1) Generate TLPs 
        TLPs_list = TLPs_Generator.main( '1' , tlps_count )
             
        
        #------------- 2. Data Link Layer -------------#
        
        #(1) Add TLPs Sequence Number and LCRC  
        Add_Seq_Add_LCRC(TLPs_list)
          
        #(2) Generate DLLPs   
        DLLPs_list = DLLPs_Generator.main( '1' , dllps_count )   
            
        
        #------------- 3. Physical Layer -------------#        
            
        #(1) Frame TLPs    
        Frame_Packet(TLPs_list,'TLP')
        
        #(2) Frame DLLPs 
        Frame_Packet(DLLPs_list,'DLLP')

        #(3) Generate OSs 
        OSs_list = OSs_Generator.main( '1' , oss_count )

        #(4) Shuffle Packets Order
        packets = TLPs_list + DLLPs_list + OSs_list
        random.shuffle(packets)
        
        #(5+6) Draw Link Table (with encoded packets if requested)
        #Create Gen1_Packets_List.txt (if requested)
                                          #All packets (shuffled)    use 8bit or 10bit symbols      use colors or not      
        link_table = Gen1_Link_Table.main(      packets          ,        perform_encode        ,     use_colors       )
        
        #Print Link Symbols Table if Requested
        if (print_link_table):
            print(link_table)
              
        #(7) Generate VOVIT Packets
        Gen1_VOVIT_Generator.main(link_table)
       
       
    # User Chose Gen3. Calling Gen3_VOVIT_Generator.py.
    elif (args['Gen'] == '3'):
        print('PCIe Gen3 Packets')
        TLPs = TLPs_Generator.main( 3 , tlps_count )
        DLLPs = DLLPs_Generator.main( 3 , dllps_count )
        OSs = OSs_Generator.main( 3 , oss_count )
        Gen3_VOVIT_Generator.main(TLPs,DLLPs,OSs)
    
    
    # User Chose Invalid Generation 
    else:
       sys.stderr.write("Invalid Generation. Choose Gen1 [-g1] or Gen3 [-g3] (run {0} -h for help)\n".format(sys.argv[0]))
       exit(1) 
    
    

main()