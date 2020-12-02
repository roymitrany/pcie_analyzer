#!/bin/python
import sys
try:
    import prettytable
except ImportError:
    sys.stderr.write('You have no prettytable installed on your machine\nrun \'pip install prettytable\' to install it\n')
    exit(1)

from pcie_packets import TLP, DLLP, OS
import random
import binascii
import argparse
import os
import re

def verify_packet_format(packet):
    if re.match('^[0-9a-fA-F]+$', packet) is None:
        raise argparse.ArgumentTypeError('Packet(s) must be specified as hex string')
    
    return packet

def verify_existance(file_name):
    if not os.path.isfile(file_name):
        raise argparse.ArgumentTypeError('file {0} does not exist'.format(file_name))
    
    if not os.access(file_name, os.R_OK):
        raise argparse.ArgumentTypeError('file {0} is not readable'.format(file_name))

def verify_format(file_name):
    with open(file_name) as packets_file:
        packets = packets_file.readlines()
    
    for packet in packets:
        verify_packet_format(packet)

def verify_file(file_name):
    verify_existance(file_name)
    verify_format(file_name)
    return file_name



class SyncHeader:

    def __init__(self, data_type):
        self.value = 0b01 if data_type == 'D' else 0b10
        self.sync_header_token = '0\n1' if data_type == 'D' else '1\n0'

    def get_value(self):
        return self.value

    def __str__(self):
        return self.sync_header_token

class Symbol:

    def __init__(self, symbol_type, symbol_token, symbol_value):
        assert type(symbol_value) == int
        self.symbol_type = symbol_type
        self.symbol_token = symbol_token
        self.symbol_value = symbol_value

    def get_value(self):
        return self.symbol_value

    def __str__(self):
        return self.symbol_token + '\n{0:02x}'.format(self.get_value())


class SymbolTable:

    def __init__(self, width = 1):
        self.table_width = width
        self.current_symbol = 0
        self.current_block = 0
        self.num_of_rows_in_block = 17
        self.data_stream_count = 0
        self.table = [[None] * width for i in range(self.num_of_rows_in_block)]

    def add_symbol(self, symbol_type, symbol_token, symbol_value):
        if not self.current_symbol % (self.table_width * self.num_of_rows_in_block):
            self.data_stream_count += 1
            self.table[self.current_symbol // self.table_width] = [SyncHeader(symbol_type)] * self.table_width
            self.current_symbol += self.table_width

        self.table[self.current_symbol // self.table_width][self.current_symbol % self.table_width] = Symbol(symbol_type, symbol_token, symbol_value)
        self.current_symbol += 1

        if self.current_symbol // self.table_width >= len(self.table):
            self.table += [[None] * self.table_width for i in range(self.num_of_rows_in_block)]

    
    def build_stp(self, tlp):
        length = 1 + tlp.get_header_size() + tlp.get_payload_size() + tlp.prefix_count() + 1 # STP + header + payload + prefix (always 0) + TLP Digest (always 0) + LCRC (1)
        sequence_number = random.randrange(4096)
        
        stp = []
        stp.append(((length & 0xf) << 4) | 0xf)
        stp.append((length >> 4) & 0x7f)
        stp.append(sequence_number >> 8)
        stp.append(sequence_number & 0xff)
        
        return stp
        
    
    def add_tlp(self, tlp):
        stp = self.build_stp(tlp)
        for i in range(4):
            self.add_symbol('D', 'STP', stp[i])

        for i in range(tlp.prefix_count() * 4):
            self.add_symbol('D', 'TLP Prefix', tlp[i // 4][((4 - (i % 4))) * 8 - 1:(3 - (i % 4)) * 8])

        for i in range(tlp.get_header_size() * 4):
            self.add_symbol('D', 'TLP Header', tlp[i // 4][((4 - (i % 4))) * 8 - 1:(3 - (i % 4)) * 8])

        for i in range(tlp.get_header_size() * 4, tlp.get_header_size() * 4 + tlp.get_payload_size() * 4, 1):
            self.add_symbol('D', 'TLP Data', tlp[i // 4][((4 - (i % 4))) * 8 - 1:(3 - (i % 4)) * 8])

        for i in range(4):
            self.add_symbol('D', 'LCRC', 0xcc)

    def add_dllp(self, dllp):
        sdp_values = [0xf0, 0xac]
        for i in range(2):
            self.add_symbol('D', 'SDP', sdp_values[i])

        for i in range(4):
            self.add_symbol('D', 'DLLP', dllp[i // 4][((4 - (i % 4))) * 8 - 1:(3 - (i % 4)) * 8])

        for i in range(2):
            self.add_symbol('D', 'CRC', 0xcc)

    def close_data_stream(self):
        if not self.current_symbol % (self.table_width * self.num_of_rows_in_block):
            return

        while (self.current_symbol + 4) % (self.table_width * self.num_of_rows_in_block):
            self.add_idle(1)

        eds_values = [0x1f, 0x80, 0x90, 0x00]
        for i in range(4):
            self.add_symbol('D', 'EDS', eds_values[i])

    def add_os(self, os):
        self.close_data_stream()
        for i in range(16):
            for j in range(self.table_width):
                self.add_symbol('O', os.get_symbol_token(i), os[i // 4][((4 - (i % 4))) * 8 - 1:(3 - (i % 4)) * 8])

    def add_idle(self, num_of_symbols):
        for i in range(num_of_symbols):
            self.add_symbol('D', 'IDL', 0)

    def get_block(self, block_idx, include_sync_header = False):
        block = []
        for i in range((block_idx // self.table_width) * self.num_of_rows_in_block + int(not include_sync_header), ((block_idx // self.table_width) + 1) * self.num_of_rows_in_block, 1):
            block.append(self.table[i][block_idx % self.table_width])

        return block
    
    def get_num_of_blocks(self):
        return self.data_stream_count * self.table_width
    
    def is_data_block(self, block_idx):
        assert self.table[self.num_of_rows_in_block * (block_idx // self.table_width)][0].get_value() == 0b01 or self.table[self.num_of_rows_in_block * (block_idx // self.table_width)][0].get_value() == 0b10
        return self.table[self.num_of_rows_in_block * (block_idx // self.table_width)][0].get_value() == 0b01
    
    def to_vovit(self):
        self.close_data_stream()
        
        vovit = []
        vovit_chunk = ''
        j = 0
        data_block = []
        valid = 0
        data = 0
        error = 0
        fake_timer = 0
        meta_data = 0
        for i in range(self.get_num_of_blocks()):
            data_block.append(self.get_block(i))
            
            valid |= (1 << j)
            data  |= (int(self.is_data_block(i)) << j)
            error |= (0 << j)
            
            fake_timer += random.randrange(1 << 15)
            
            j += 1
            if j == 6 or i + 1 == self.get_num_of_blocks():
                meta_data |= (0x70 if random.randrange(2) else 0xa0) << 56
                meta_data |= 0x3 << 53
                meta_data |= self.table_width << 48
                meta_data |= fake_timer & (0xffffffff)
                
                vovit_chunk += '{0:016x}    '.format(meta_data)
                vovit_chunk += '{0:02x}'.format(valid) + '{0:02x}'.format(data) + '{0:02x}'.format(error) + '    '
                
                vovit_chunk += '|'
                for db in data_block:
                    vovit_chunk += ''.join(['{0:02x}'.format(b.get_value()) for b in db]) + '|'
                
                if j < 6:
                    vovit_chunk += ('0' * 32 + '|') * (6 - j)
                
                vovit_chunk += '    '
                vovit_chunk += '00' * 21
                vovit.append(vovit_chunk)
                
                j = 0
                valid = 0
                data = 0
                error = 0
                meta_data = 0
                data_block[:] = []
                vovit_chunk = ''

        return vovit
                

    def __str__(self):
        self.close_data_stream()

        pt = prettytable.PrettyTable(hrules = prettytable.ALL)
        pt.field_names = ['Lane {0}'.format(lane) for lane in range(self.table_width)]
        for i in range(len(self.table)):
            pt.add_row([str(table_entry) if table_entry is not None else '' for table_entry in self.table[i]])

        return pt.get_string()


def randomize_idles():
    idles = 0
    dist = [50, 60, 70, 80, 90, 95]
    random_num = random.randrange(100)
    if random_num >= dist[1] and random_num < dist[2]:
        idles = 4
    elif random_num >= dist[2] and random_num < dist[3]:
        idles = 8
    elif random_num >= dist[3] and random_num < dist[4]:
        idles = 12
    elif random_num >= dist[4] and random_num < dist[5]:
        idles = 24
    
    return idles

def read_packets(tlps, dllps, oss, packets_file_name):
    symbol_table = SymbolTable(8)

    packets = [('TLP', tlp) for tlp in tlps] + [('DLLP', dllp) for dllp in dllps] + [('OS', os) for os in oss]
    random.shuffle(packets)
    
    with open(packets_file_name, 'w') as packets_file:
        for packet in packets:
            if packet[0] in ['TLP', 'DLLP']:
                symbol_table.add_idle(randomize_idles())
                
                if packet[0] == 'TLP':
                    tlp = TLP(packet[1])
                    symbol_table.add_tlp(tlp)
                    packets_file.write(tlp.display(True) + '\n')
                elif packet[0] == 'DLLP':
                    dllp = DLLP(packet[1])
                    symbol_table.add_dllp(dllp)
                    packets_file.write(dllp.display() + '\n')
            else:
                os = OS(packet[1] , '3')
                symbol_table.add_os(os)
                packets_file.write(os.display() + '\n')
    
    return symbol_table


def main(tlps, dllps, oss):
    
    symbol_table = read_packets(tlps, dllps, oss, './Output/Gen3_Packets_List.txt')
    vovit = symbol_table.to_vovit()
    
    with open('./Output/Gen3_Link_Table.txt'.format('.'), 'w') as symbol_table_as_text:
        symbol_table_as_text.write(str(symbol_table))
        
    with open('./Output/Gen3_VOVIT_Trace.txt'.format('.'), 'w') as vovit_as_text:
        vovit_as_text.write('\n'.join(vovit))
    
    with open('./Output/Gen3_VOVIT_Trace.bin'.format('.'), 'wb') as vovit_as_bin:
        vovit_as_bin.write(binascii.unhexlify(''.join([v.replace('|', '').replace(' ','') for v in vovit])))
   
       
    if not tlps and not dllps and not oss:
        sys.stderr.write("At least one file should be provided (run {0} -h for help)\n".format(sys.argv[0]))
        exit(1)
    


if __name__ == "__main__":
    # Should not get here unless executing this script manually without vovit_traffic_trace.py 
    print("Calling vovit_generator without any tlps,dllps or oss")
    tlps=[]
    dllps=[]
    oss=[]
    main(tlps,dllps,oss)
# does not calculate CRC
# randomizes sequence number in tlps
# randomizes IDLEs
# no prefix
# no logical sense
