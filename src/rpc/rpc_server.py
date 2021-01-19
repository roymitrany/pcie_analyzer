#! /usr/bin/python3

from xmlrpc.server import *
import xmlrpc.client
import io
import sys
import re
import argparse
import time
import struct
import os




def start_server(pcie_filter,client_ip,pcie_trigger,packet_number):
    
    command= "sudo ./server " + pcie_filter +" "+client_ip  +" " +pcie_trigger + " "+packet_number+" &"
    os.system(command)
    return 1   
   
server = SimpleXMLRPCServer(('192.168.0.1',8080))
print("server is listening on port 8080")


server.register_function(start_server,"start_server")

server.serve_forever()   
