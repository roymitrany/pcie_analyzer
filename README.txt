Compilation instructions:
1. mkdir ./build
2. cd ./build
3. cmake ..
4. make


Usage instructions:
server side:
1. run rpc_server.py (this is the listener)
 note: pay attention that this file is located in the same dir with server exe file 
 
 client side:
 2. copy pcie_extcap.py to /usr/lib/x86_64-linux-gnu/wireshark/extcap/ by:
    sudo cp pcie_extcap.py /usr/lib/x86_64-linux-gnu/wireshark/extcap/
 
 3. run wireshark and choose "PCIe interface 1 for extcap: if1"
 4. optional: choose Filter and Trigger
 5. run.











