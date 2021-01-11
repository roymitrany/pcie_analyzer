#! /usr/bin/python3
# Copyright 2014 Roland Knall <rknall [AT] gmail.com>
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
#
# SPDX-License-Identifier: GPL-2.0-or-later
#

"""
This is a generic example, which produces pcap packages every n seconds, and
is configurable via extcap options.
@note
{
To use this script on Windows, please generate an extcap_example.bat inside
the extcap folder, with the following content:
-------
@echo off
<Path to python interpreter> <Path to script file> %*
-------
Windows is not able to execute Python scripts directly, which also goes for all
other script-based formats beside VBScript
}
"""

from __future__ import print_function
import io
import sys
import re
import argparse
import time
import struct
import os
import logging
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *
from threading import Thread
import xmlrpc.client

ERROR_USAGE          = 0
ERROR_ARG            = 1
ERROR_INTERFACE      = 2
ERROR_FIFO           = 3
ERROR_DELAY          = 4

CTRL_CMD_INITIALIZED = 0
CTRL_CMD_SET         = 1
CTRL_CMD_ADD         = 2
CTRL_CMD_REMOVE      = 3
CTRL_CMD_ENABLE      = 4
CTRL_CMD_DISABLE     = 5
CTRL_CMD_STATUSBAR   = 6
CTRL_CMD_INFORMATION = 7
CTRL_CMD_WARNING     = 8
CTRL_CMD_ERROR       = 9

#CTRL_ARG_MESSAGE     = 0
CTRL_ARG_DELAY       = 1
CTRL_ARG_VERIFY      = 2
CTRL_ARG_BUTTON      = 3
CTRL_ARG_HELP        = 4
CTRL_ARG_RESTORE     = 5
CTRL_ARG_LOGGER      = 6
CTRL_ARG_NONE        = 255

initialized = False
delay = 0.0
button = False
button_disabled = False
pcap_fifo= 0
proxy=0

fifo_handler=0

"""
This code has been taken from http://stackoverflow.com/questions/5943249/python-argparse-and-controlling-overriding-the-exit-status-code - originally developed by Rob Cowie http://stackoverflow.com/users/46690/rob-cowie
"""
class ArgumentParser(argparse.ArgumentParser):
    def _get_action_from_name(self, name):
        """Given a name, get the Action instance registered with this parser.
        If only it were made available in the ArgumentError object. It is
        passed as it's first arg...
        """
        container = self._actions
        if name is None:
            return None
        for action in container:
            if '/'.join(action.option_strings) == name:
                return action
            elif action.metavar == name:
                return action
            elif action.dest == name:
                return action

    def error(self, message):
        exc = sys.exc_info()[1]
        if exc:
            exc.argument = self._get_action_from_name(exc.argument_name)
            raise exc
        super(ArgumentParser, self).error(message)

#### EXTCAP FUNCTIONALITY

"""@brief Extcap configuration
This method prints the extcap configuration, which will be picked up by the
interface in Wireshark to present a interface specific configuration for
this extcap plugin
"""
def extcap_config(interface, option):
    args = []
    values = []
    multi_values = []

    #args.append((0, '--delay', 'Time delay', 'Time delay between packages', 'integer', '{range=1,15}{default=5}'))
    args.append((0, '--ts', 'Start Time', 'Capture start time', 'timestamp', '{group=Time / Log}'))
   # args.append((1, '--run_another_script', 'Run SERVER', 'Running another executable', 'boolflag', '{default=no}'))
    args.append((1, '--pcie_filter', 'PCIe Filter', 'Package filter content', 'string', '{required=false}{placeholder=Please enter a filter here ...}'))
    args.append((2, '--pcie_trigger', 'PCIe Trigger', 'Package filter content', 'string', '{required=false}{placeholder=Please enter a trigger here ...}'))

    args.append((3, '--client_ip', 'Client IP Address', 'Use this ip address as client', 'string',
                 '{default=192.168.0.2}{required=false}{placeholder=Please enter a trigger here ...}{save=false}{validation=\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b}'))
    if len(option) <= 0:
        for arg in args:
            print("arg {number=%d}{call=%s}{display=%s}{tooltip=%s}{type=%s}%s" % arg)

        
    for (value, parent) in multi_values:
        sentence = "value {arg=%d}{value=%s}{display=%s}{default=%s}{enabled=%s}" % value
        extra = "{parent=%s}" % parent if parent else ""
        print("".join((sentence, extra)))   
   

def extcap_version():
    print("extcap {version=1.0}{help=https://www.wireshark.org}{display=Example extcap interface}")

def extcap_interfaces():
    print("extcap {version=1.0}{help=https://www.wireshark.org}{display=Example extcap interface}")
    print("interface {value=if1}{display=PCIe interface 1 for extcap}")
    print("control {number=%d}{type=selector}{display=Time delay}{tooltip=Time delay between packages}" % CTRL_ARG_DELAY)
    print("control {number=%d}{type=button}{role=help}{display=Help}{tooltip=Show help}" % CTRL_ARG_HELP)
    print("control {number=%d}{type=button}{role=restore}{display=Restore}{tooltip=Restore default values}" % CTRL_ARG_RESTORE)
    print("control {number=%d}{type=button}{role=logger}{display=Log}{tooltip=Show capture log}" % CTRL_ARG_LOGGER)
    print("value {control=%d}{value=1}{display=1}" % CTRL_ARG_DELAY)
    print("value {control=%d}{value=2}{display=2}" % CTRL_ARG_DELAY)
    print("value {control=%d}{value=3}{display=3}" % CTRL_ARG_DELAY)
    print("value {control=%d}{value=4}{display=4}" % CTRL_ARG_DELAY)
    print("value {control=%d}{value=5}{display=5}{default=true}" % CTRL_ARG_DELAY)
    print("value {control=%d}{value=60}{display=60}" % CTRL_ARG_DELAY)


def extcap_dlts(interface):
    if interface == '1':
        print("dlt {number=147}{name=USER0}{display=Demo Implementation for Extcap}")
    elif interface == '2':
        print("dlt {number=148}{name=USER1}{display=Demo Implementation for Extcap}")

def validate_capture_filter(capture_filter):
    if capture_filter != "filter" and capture_filter != "valid":
        print("Illegal capture filter")

"""
### FAKE DATA GENERATOR
Extcap capture routine
 This routine simulates a capture by any kind of user defined device. The parameters
 are user specified and must be handled by the extcap.
 The data captured inside this routine is fake, so change this routine to present
 your own input data, or call your own capture program via Popen for example. See
 for more details.
"""
def unsigned(n):
    return int(n) & 0xFFFFFFFF

def global_pcap_header():

    header = bytearray()
    header += struct.pack('<L', int('a1b2c3d4', 16))
    header += struct.pack('<H', unsigned(2))  # Pcap Major Version
    header += struct.pack('<H', unsigned(4))  # Pcap Minor Version
    header += struct.pack('<I', int(0))  # Timezone
    header += struct.pack('<I', int(0))  # Accuracy of timestamps
    header += struct.pack('<L', int('0000ffff', 16))  # Max Length of capture frame
    header += struct.pack('<L', unsigned(1))  # Ethernet
    return header




def pcap_package(packet):

    pcap = bytearray()
    caplength = len(packet)
   # timestamp = int(time.time())
    
    
    t = time.time()
    it = int(t)            
    sec = it
    usec = int(round((t - it) *(1000000)))


    pcap += struct.pack('<L', unsigned(sec))  # timestamp seconds
    #pcap += struct.pack('<L', 0x00)  # timestamp microseconds
    pcap += struct.pack('<L', unsigned(usec))  # timestamp microseconds
    pcap += struct.pack('<L', unsigned(caplength))  # length captured
    pcap += struct.pack('<L', unsigned(caplength))  # length in frame
    
    pcap += packet
    return pcap




    
def control_read(fn):
    try:
        header = fn.read(6)
        sp, _, length, arg, typ = struct.unpack('>sBHBB', header)
        if length > 2:
            payload = fn.read(length - 2).decode('utf-8', 'replace')
        else:
            payload = ''
        return arg, typ, payload
    except Exception:
        return None, None, None



def control_read_thread(control_in, fn_out,filter1,client_ip,pcie_trigger):
    global initialized, delay, button, button_disabled
   # hostname = socket.gethostname()
    #local_ip = socket.gethostbyname(hostname)
    proxy = xmlrpc.client.ServerProxy("http://192.168.0.1:8080/")
    proxy.start_server(filter1,client_ip,pcie_trigger)
    with open(control_in, 'rb', 0) as fn:
        arg = 0
        #proxy = xmlrpc.client.ServerProxy("http://192.168.0.1:8080/")
        #proxy.start_server(filter_pcie)    
        while arg is not None:
            arg, typ, payload = control_read(fn)
            log = ''
            if typ == CTRL_CMD_INITIALIZED:
                initialized = True
            elif arg == CTRL_ARG_DELAY:
                delay = float(payload)
                log = "Time delay = " + payload
            elif arg == CTRL_ARG_BUTTON:
                control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_DISABLE, "")
                button_disabled = True
                if button:
                    control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_SET, "Turn on")
                    button = False
                    log = "Button turned off"
                else:
                    control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_SET, "Turn off")
                    button = True
                    log = "Button turned on"

            if len(log) > 0:
                control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_ADD, log + "\n")

def control_write(fn, arg, typ, payload):
    packet = bytearray()
    packet += struct.pack('>sBHBB', b'T', 0, len(payload) + 2, arg, typ)
    if sys.version_info[0] >= 3 and isinstance(payload, str):
        packet += payload.encode('utf-8')
    else:
        packet += payload
    fn.write(packet)

def control_write_defaults(fn_out):
    global initialized, delay

    while not initialized:
        time.sleep(.1)  # Wait for initial control values

    # Write startup configuration to Toolbar controls
  
    control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_SET, str(int(delay)))

    for i in range(1, 16):
        item = '%d\x00%d sec' % (i, i)
        control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_ADD, item)

    control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_REMOVE, str(60))
    
    
    
    


  
def insert_packet(packet):
    # upload packet, using passed arguments
    #fifo_handler.write(pcap_package(raw(packet))) 
    fifo_handler.write(pcap_package(bytes(packet))) 
    
    
    
    
    

def extcap_capture(interface, fifo, control_in, control_out, remote,pcie_filter,client_ip,pcie_trigger):
    
    global   button_disabled ,fifo_handler
    #delay = in_delay if in_delay != 0 else 5
    counter = 1
    fn_out = None
       

 
   
    
    with open(fifo, 'wb', 0) as fh:
        fh.write(global_pcap_header())
        fifo_handler=fh
       
        if control_out is not None:
            fn_out = open(control_out, 'wb', 0)
            control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_SET, "Log started at " + time.strftime("%c") + "\n")

        #thread1 = Thread(target=startserver, args=(pcie_filter))
        #thread1.start()

        if control_in is not None:
            # Start reading thread

            thread = Thread(target=control_read_thread, args=(control_in, fn_out,pcie_filter,client_ip,pcie_trigger))
            thread.start()

        if fn_out is not None:
            control_write_defaults(fn_out)
        
        #proxy = xmlrpc.client.ServerProxy("http://192.168.0.1:8080/")
       # proxy.start_server(pcie_filter) 
        sniff(iface="enp0s8",filter="ip dst 192.168.0.2 and port 21212",prn=insert_packet)
	
       
        
        #sniff(iface="enp0s8",prn=insert_packet)
        #sniff(prn=insert_packet)
       
       
        while True:
            if fn_out is not None:
                log = "Received packet #" + str(counter) + "\n"
                control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_ADD, log)
                counter = counter + 1

                if button_disabled:
                    control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_ENABLE, "")
                    control_write(fn_out, CTRL_ARG_NONE, CTRL_CMD_INFORMATION, "Turn action finished.")
                    button_disabled = False
         
            
         
            #a=sniff(iface="enp5s0",filter="ip dst 132.68.60.170",count=1)
            #for p in a:
             #   fh.write(pcap_fake_package(raw(p))) 
                
            #time.sleep(delay)

    thread.join()
    if fn_out is not None:
        fn_out.close()

def extcap_close_fifo(fifo):
    fh = open(fifo, 'wb', 0)
    fh.close()

####

def usage():
    print("Usage: %s <--extcap-interfaces | --extcap-dlts | --extcap-interface | --extcap-config | --capture | --extcap-capture-filter | --fifo>" % sys.argv[0] )

if __name__ == '__main__':
    
    interface = ""
    option = ""

    # Capture options
    #delay = 0
    pcie_filter = ""
    pcie_trigger = ""
    client_ip = ""
   
    ts = 0

    parser = ArgumentParser(
            prog="Extcap Example",
            description="Extcap example program for python"
            )

    # Extcap Arguments
    parser.add_argument("--capture", help="Start the capture routine", action="store_true" )
    parser.add_argument("--extcap-interfaces", help="Provide a list of interfaces to capture from", action="store_true")
    parser.add_argument("--extcap-interface", help="Provide the interface to capture from")
    parser.add_argument("--extcap-dlts", help="Provide a list of dlts for the given interface", action="store_true")
    parser.add_argument("--extcap-config", help="Provide a list of configurations for the given interface", action="store_true")
    parser.add_argument("--extcap-capture-filter", help="Used together with capture to provide a capture filter")
    parser.add_argument("--fifo", help="Use together with capture to provide the fifo to dump data to")
    parser.add_argument("--extcap-control-in", help="Used to get control messages from toolbar")
    parser.add_argument("--extcap-control-out", help="Used to send control messages to toolbar")
    parser.add_argument("--extcap-version", help="Shows the version of this utility", nargs='?', default="")
    parser.add_argument("--extcap-reload-option", help="Reload elements for the given option")

    # Interface Arguments
  
 
   # parser.add_argument("--delay", help="Demonstrates an integer variable", type=int, default=0, choices=[0, 1, 2, 3, 4, 5, 6] )
    parser.add_argument("--ts", help="Capture start time", action="store_true" )
    #parser.add_argument("--run_another_script", help="Running another executable", action="store_true" )
    parser.add_argument("--pcie_filter", help="Demonstrates string variable", nargs='?', default="" )
    parser.add_argument("--pcie_trigger", help="Demonstrates string variable", nargs='?', default="" )
    parser.add_argument("--client_ip", help="Add a client IP address", nargs='?', default="192.168.0.2")

    try:
        args, unknown = parser.parse_known_args()
    except argparse.ArgumentError as exc:
        print("%s: %s" % (exc.argument.dest, exc.message), file=sys.stderr)
        fifo_found = 0
        fifo = ""
        for arg in sys.argv:
            if arg == "--fifo" or arg == "--extcap-fifo":
                fifo_found = 1
            elif fifo_found == 1:
                fifo = arg
                break
        extcap_close_fifo(fifo)
        sys.exit(ERROR_ARG)

    if len(sys.argv) <= 1:
        parser.exit("No arguments given!")

    if args.extcap_version and not args.extcap_interfaces:
        extcap_version()
        sys.exit(0)

    if not args.extcap_interfaces and args.extcap_interface is None:
        parser.exit("An interface must be provided or the selection must be displayed")
    if args.extcap_capture_filter and not args.capture:
        validate_capture_filter(args.extcap_capture_filter)
        sys.exit(0)

    if args.extcap_interfaces or args.extcap_interface is None:
        extcap_interfaces()
        sys.exit(0)

    if len(unknown) > 1:
        print("Extcap Example %d unknown arguments given" % len(unknown))

    m = re.match('if(\d+)', args.extcap_interface)
    if not m:
        sys.exit(ERROR_INTERFACE)
    interface = m.group(1)

    ts = args.ts
    pcie_filter = args.pcie_filter
    if args.pcie_filter is None or len(args.pcie_filter) == 0:
        pcie_filter = "empty"

    client_ip = args.client_ip
    if args.client_ip is None or len(args.client_ip) < 7 or len(args.client_ip.split('.')) != 4:
        client_ip = "192.168.0.2"

    pcie_trigger = args.pcie_trigger
    if args.pcie_trigger is None or len(args.pcie_trigger) == 0:
        pcie_trigger = "empty"

    #proxy = xmlrpc.client.ServerProxy("http://192.168.0.1:8080/")
    
    #if args.run_another_script:
        
        #os.system("sudo /home/user/temp/TEMP2/PROG")
       # proxy.start_server()

    

    if args.extcap_reload_option and len(args.extcap_reload_option) > 0:
        option = args.extcap_reload_option

    if args.extcap_config:
        extcap_config(interface, option)
    elif args.extcap_dlts:
        extcap_dlts(interface)
    elif args.capture:
    

         


        #thread1 = Thread(target=startserver, args=(pcie_filter))
        #thread1.start()
        #proxy = xmlrpc.client.ServerProxy("http://192.168.0.1:8080/")
        #proxy.start_server(pcie_filter)

        if args.fifo is None:
            sys.exit(ERROR_FIFO)
        # The following code demonstrates error management with extcap
       # if args.delay > 5:
       #    print("Value for delay [%d] too high" % args.delay, file=sys.stderr)
       #   extcap_close_fifo(args.fifo)
       #  sys.exit(ERROR_DELAY)

        try:
            extcap_capture(interface, args.fifo, args.extcap_control_in, args.extcap_control_out, "if1",pcie_filter,client_ip,pcie_trigger)
        except KeyboardInterrupt:
            pass
    else:
        usage()
        sys.exit(ERROR_USAGE)
