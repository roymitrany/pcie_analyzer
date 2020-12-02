from collections import defaultdict

class Color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class Packet(object):
    
    def __init__(self, raw_packet):
        self._raw_packet = [int(raw_packet[i : i + 8], 16) for i in range(0, len(raw_packet), 8)]
        self._as_str = raw_packet
        self._second_access = False

    def __getitem__(self, key):
        if not self._second_access:
            self._first_key = key
            self._second_access = True
            return self
        else:
            self._second_access = False
            try:
                dword = self._raw_packet[self._first_key]
            except IndexError:
                raise IndexError('Could not parse {0}; wrong header definition.\nExiting...'.format(self._as_str))
            
            if isinstance(key, slice):
                if key.start > 31 or key.stop < 0:
                    raise IndexError('dword index [{0}:{1}] out of range'.format(key.start, key.stop))

                mask = (1 << (key.start - key.stop + 1)) - 1
                return (dword >> key.stop) & mask
                
            else:
                if key > 31 or key < 0:
                    raise IndexError('dword index [{0}] out of range'.format(key))
                
                return (dword >> key) & 0x1
    
                
class OS(Packet):
    
    Type = dict({
                 0xaa : 'SKP' ,
                 0x00 : 'EIEOS' ,
                 0x66 : 'EIOS',
                 0xe1 : 'SDS' ,
                 0x1e : 'TS1' ,
                 0x2d : 'TS2' ,
                 0x3c : 'FTS' ,
                 0x1c : 'SKP' ,
               })
    
    def __init__(self, raw_packet , gen):
        super(OS, self).__init__(raw_packet)
        self.gen = gen
        self.parse_os()
    
    def parse_os(self):
        if (self.gen == '1'):
            if (self[0][24:16] == 0x7c):         # 0x7c = IDL
                self.os_type = OS.Type[0x66]     # 0x66 = EIOS
            elif (self[0][24:16] == 0xf7):       # 0xf7 = PAD
                if (self[3][24:16] == 0x4a):     # 0x4a = TS1 ID
                    self.os_type = OS.Type[0x1e] # 0x1e = TS1
                else:
                    self.os_type = OS.Type[0x2d] # 0x2d = TS2
            else:
                self.os_type = OS.Type[self[0][24:16]]
        else:
            self.os_type = OS.Type[self[0][31:24]]
    
    def display(self):
        return self.os_type

    def get_symbol_token(self, symbol_idx):
        if self.os_type == 'SKP':
            if symbol_idx < 12:
                return 'SKP'
            if symbol_idx == 12:
                return 'SKP_END'
            else:
                return 'LFSR'
        elif self.os_type in ['EIEOS', 'EIOS', 'SDS']:
            return self.os_type
        elif self.os_type in ['TS1', 'TS2']:
            if symbol_idx == 0:
                return self.os_type
            elif symbol_idx == 1:
                return 'LINK #'
            elif symbol_idx == 2:
                return 'LANE #'
            elif symbol_idx == 3:
                return 'N_FTS'
            elif symbol_idx == 4:
                return 'DATA_RATE'
            elif symbol_idx == 5:
                return 'CTRL'
            else:
                return self.os_type
            
            


class DLLP(Packet):
    
    Type = dict({
                 0b00000000 : 'Ack' ,
                 0b00000001 : 'MRInit' ,
                 0b00000010 : 'Data_Link_Feature',
                 0b00010000 : 'Nack' ,
                 0b00100000 : 'PM_Enter_L1' ,
                 0b00100001 : 'PM_Enter_L23' ,
                 0b00100011 : 'PM_Active_State_Request_L1' ,
                 0b00100100 : 'PM_Request_Ack' ,
                 0b00110000 : 'Vendor_Specific' ,
                 0b00110001 : 'NOP' ,
                 0b01000000 : 'InitFC1-P' ,
                 0b01010000 : 'InitFC1-NP' ,
                 0b01100000 : 'InitFC1-CPL' ,
                 0b01110000 : 'MRInitFC1' ,
                 0b11000000 : 'InitFC2-P' ,
                 0b11010000 : 'InitFC2-NP' ,
                 0b11100000 : 'InitFC2-CPL' ,
                 0b11110000 : 'MRInitFC2' ,
                 0b10000000 : 'UpdateFC-P' ,
                 0b10010000 : 'UpdateFC-NP' ,
                 0b10100000 : 'UpdateFC-CPL' ,
                 0b10110000 : 'MRUpdateFC' ,
               })

    def __init__(self, raw_packet):
        super(DLLP, self).__init__(raw_packet)
        self.parse_dllp()


    def parse_dllp(self):
        self.dllp_fields = []
        self.dllp_type = DLLP.Type[self[0][31:24]] # throw exception on KeyError
        
        if self.dllp_type in ['Ack', 'Nack']:
            self.dllp_fields += [['Sequence Number', '0x{0:03x}'.format(self[0][11:0])]]
        elif self.dllp_type in ['Data_Link_Feature']:
            if self[0][23]:
                self.dllp_type += ' Ack'
            self.dllp_fields += [['Feature Support', '0b{0:023b}'.format(self[0][22:0])]]
        elif self.dllp_type in ['InitFC1-P', 'InitFC1-NP', 'InitFC1-CPL', 'InitFC2-P', 'InitFC2-NP', 'InitFC2-CPL', 'UpdateFC-P', 'Update-NP', 'Update-CPL']:
            self.dllp_fields += [['Data Scale | Data FC', '0b{0:02b} | 0x{1:03x}'.format(self[0][13:12], self[0][11:0])]]
            self.dllp_fields += [['Hdr Scale | Hdr FC', '0b{0:02b} | 0x{1:03x}'.format(self[0][21:14], self[0][23:22])]]
        elif self.dllp_type in ['NOP']:
            pass
        elif self.dllp_type in ['PM_Enter_L1', 'PM_Enter_L23', 'PM_Active_State_Request_L1', 'PM_Request_Ack']:
            pass
        else:
            pass
            
    

    def display(self):
        human_readable_form = self.dllp_type
        for field in self.dllp_fields:
            human_readable_form += ', ' + field[0] + ': ' + field[1]
        
        return human_readable_form


class TLP(Packet):

    CompletionStatus = defaultdict(lambda: 'Unknown Completion Status',
                       {
                         0b000 : 'CS' ,
                         0b001 : 'UR' ,
                         0b010 : 'CRS',
                         0b100 : 'CA' ,
                       })

    MessageRouting   = defaultdict(lambda: 'Unknown Message Routing',
                       {
                         0b10000 : 'Implicitly routed to RC'     ,
                         0b10001 : 'Routed by Address'           ,
                         0b10010 : 'Routed by ID'                ,
                         0b10011 : 'Implicitly Broadcast from RC',
                         0b10100 : 'Local, terminate at receiver',
                         0b10101 : 'Gather & route to RC'        ,
                       })

    MessageCode      = defaultdict(lambda: 'Unknown Message Code',
                       {
                         0b00000000 : 'Unlock Message'                         ,
                         0b00010000 : 'Lat. Tolerance Reporting'               ,
                         0b00010010 : 'Optimized Buffer Flush/Fill'            ,
                         0b00010100 : 'Power Mgt. Message; PM_Active_State_Nak',
                         0b00011000 : 'Power Mgt. Message; PM_PME'             ,
                         0b00011001 : 'Power Mgt. Message; PM_Turn_Off'        ,
                         0b00011011 : 'Power Mgt. Message; PME_TO_Ack'         ,
                         0b00100000 : 'INTx Message; Assert_INTA'              ,
                         0b00100001 : 'INTx Message; Assert_INTB'              ,
                         0b00100010 : 'INTx Message; Assert_INTC'              ,
                         0b00100011 : 'INTx Message; Assert_INTD'              ,
                         0b00100100 : 'INTx Message; Deassert_INTA'            ,
                         0b00100101 : 'INTx Message; Deassert_INTB'            ,
                         0b00100110 : 'INTx Message; Deassert_INTC'            ,
                         0b00100111 : 'INTx Message; Deassert_INTD'            ,
                         0b00110000 : 'Error Message; ERR_COR'                 ,
                         0b00110001 : 'Error Message; ERR_NONFATAL'            ,
                         0b00110011 : 'Error Message; ERR_FATAL'               ,
                         0b01010000 : 'Set_Slot_Power_Limit'                   ,
                         0b01111110 : 'Vendor Defined 0'                       ,
                         0b01111111 : 'Vendor Defined 1'                       ,
                       })

    def __init__(self, raw_packet, bdf_as_ari = False, reverse_endieness = False):
        super(TLP, self).__init__(raw_packet)
        self.data_padded = False
        self.parse_tlp(bdf_as_ari)


    def reverse_dword_endieness(self, dword):
        return (((dword << 24) & 0xff000000) | ((dword << 8) & 0x00ff0000) | ((dword >> 8) & 0x0000ff00) | ((dword >> 24) & 0x000000ff))


    def reverse_tlp_endieness(self, tlp):
        return [self.reverse_dword_endieness(dword) for dword in tlp]

        def get_tag(self):
                return '{0:02x}'.format(self[2][15:8])

    def parse_tlp(self, bdf_as_ari):

        self.tlp_fields = []        
        
        if self[0][28:24] == 0b01010:  # Completion
            self.tlp_type    = 'CplD' if self[0][31:29] == 0b010 else 'Cpl'
            self.tlp_fields += [['Status'     , format(TLP.CompletionStatus[self[1][15:13]])]]
            self.tlp_fields += [['CompleterID', format(self.to_bdf(self[1][31:16], bdf_as_ari))]]
            self.tlp_fields += [['RequesterID', format(self.to_bdf(self[2][31:16], bdf_as_ari))]]
            self.tlp_fields += [['Tag'        , '{0:03x}'.format((self[0][23] << 9) | (self[0][19] << 8) | self[2][15:8])]]
            if self[0][31:29] == 0b010:
                self.tlp_fields += [['Data'   , format(self.parse_data(3, self[0][9:0]))]]

            self.header_size = 3
            self.contains_data = self[0][31:29] == 0b010
            
        elif self[0][31] == 0b0 and self[0][29:25] == 0b00010: # Configuration Request
            self.tlp_type    = "CfgRd{0}".format(str(self[0][24])) if self[0][31:29] == 0b000 else "CfgWr{0}".format(str(self[0][24]))
            self.tlp_fields += [['Register Number', '{0:03x}'.format(self[2][11:0])]]
            self.tlp_fields += [['RequesterID'    , format(self.to_bdf(self[1][31:16], bdf_as_ari))]]
            self.tlp_fields += [['BDF'            , format(self.to_bdf(self[2][31:16], bdf_as_ari))]]
            self.tlp_fields += [['Tag'            , '{0:03x}'.format((self[0][23] << 9) | (self[0][19] << 8) | self[1][15:8])]]
            if self[0][31:29] == 0b010:
                self.tlp_fields += [['Data'       , '{0} (BE={1:04b})'.format(self.parse_data(3, 1), self[1][3:0])]]

            self.header_size = 3
            self.contains_data = self[0][31:29] != 0b000
            
        elif self[0][31] == 0b0 and self[0][29:27] == 0b110: # Message
            self.tlp_type = 'Msg{0}'.format('D' if self[0][30] else '')
            self.tlp_fields += [[''            , format(TLP.MessageRouting[self[0][28:24]])]]
            self.tlp_fields += [[''            , format(TLP.MessageCode[self[1][7:0]])]]
            self.tlp_fields += [['RequesterID' , format(self.to_bdf(self[1][31:16], bdf_as_ari))]]

            self.header_size = 4
            self.contains_data = False # TODO

        elif self[0][31] == 0b0 and self[0][28:25] == 0b0000: # Memory Request
            self.tlp_type = 'Mem{0}'.format('Wr' if self[0][30] == 0b1 else 'Rd')
            self.tlp_fields += [['RequesterID', format(self.to_bdf(self[1][31:16], bdf_as_ari))]]
            if self[0][29]:
                address = '0x{0:08x} {1:08x} (64Bit Access)'.format(self[2][31:0], self[3][31:0])
            else:
                address = '0x{0:08x} (32Bit Access)'.format(self[2][31:0])
            self.tlp_fields += [['{0} Address'.format('To' if self[0][30] else 'From'), address]]
            self.tlp_fields += [['Tag', '{0:03x}'.format((self[0][23] << 9) | (self[0][19] << 8) | self[1][15:8])]]
            self.tlp_fields += [['Length', '{0} DWORD{1}'.format(str(self[0][9:0]), 'S' if self[0][9:0] > 1 else '')]]
            self.tlp_fields += [['Last DW BE | 1st DW BE', '{0:04b} | {1:04b}'.format(self[1][7:4], self[1][3:0])]]
            if self[0][30] == 0b1:
                self.tlp_fields += [('Data', format(self.parse_data(4 if self[0][29] else 3, self[0][9:0])))]

            self.header_size = 3 + self[0][29]
            self.contains_data = self[0][30] == 0b1
        else:
            raise ValueError('Could not parse {0}; No such TLP type'.format(self._as_str))


    def get_header_size(self):
        return self.header_size

    def contains_data(self):
        return self.contains_data

    def prefix_count(self):
        return 0 # TODO

    def get_payload_size(self):
        return self[0][9:0] if self.contains_data else 0

    def parse_data(self, header_size_dw, data_size_dw):
        if len(self._raw_packet) < header_size_dw + data_size_dw:
            self.data_padded = True
            self._raw_packet += [0] * ((header_size_dw + data_size_dw) - len(self._raw_packet))

        formatted_data = ['{0:08x}'.format(self.reverse_dword_endieness(self[dword][31:0])) for dword in range(header_size_dw, header_size_dw + data_size_dw)]
        splitted_data = [formatted_data[i : i + 8] for i in range(0, len(formatted_data), 8)]
        return '\t'.join([' '.join(bunch_of_dwords) for bunch_of_dwords in splitted_data])
        
    def to_bdf(self, bdf, as_ari):
        bus = (bdf >> 8) & 0xff
        device = (bdf >> 3) & 0x1f
        function = bdf & 0x7
        return '{0:02x}:{1:02x}.{2:x}'.format(bus, device, function) if not as_ari else '{0:02x}:{1:02x}'.format(bus, device | function)


    def as_dwords(self):
        formatted_data = ["{0:08x}".format(self._raw_packet[i]) for i in range(len(self._raw_packet))]
        splitted_data = [formatted_data[i : i + 8] for i in range(0, len(formatted_data), 8)]
        return '\n'.join([' '.join(bunch_of_dwords) for bunch_of_dwords in splitted_data])


    def colorize(self):
        self.tlp_type = Color.BOLD + Color.OKGREEN + self.tlp_type + Color.OKGREEN + Color.BOLD + Color.ENDC
        
        for field in self.tlp_fields:
            field[0] = Color.OKBLUE + field[0] + Color.ENDC
        

    def display(self, oneline = False):
        human_readable_form = self.tlp_type
        for field in self.tlp_fields:
            if field[0] == 'Data':
                human_readable_form += (', ' if oneline else '\n\t') + field[0] + ': \t' + field[1]
            elif field[0] == '':
                human_readable_form += (', ' if oneline else '\n\t') + field[1]
            else:
                human_readable_form += (', ' if oneline else '\n\t') + field[0] + ': ' + field[1]


        assert human_readable_form is not None
        return human_readable_form + \
              ((Color.OKBLUE + '\nWARNING: packet header shows {0} dwords of data, but some data is missing from packet.\n'
               '         missing data padded with zeros...'.format(self[0][9:0]) + Color.ENDC) if self.data_padded else '')
