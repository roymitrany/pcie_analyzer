/* pci_sniff.x - Specification of remote command to start and stop pci sniffing.
This file is the input to rpcgen */
program PCISNIFF { /* remote program name (not used)*/
    version PCISNIFF_V1 { /* declaration of program version number*/
        int START(void) = 1; /* procedure number = 1 */
        int PAUSE(void) = 2; /* procedure number = 1 */
        int INTERVAL(int) = 3; /* procedure number = 1 */
        int STOP (void) = 4; /* procedure number = 1 */
    } = 1; /* definition of program version = 1*/
} = 0x3012225; /* remote program number (must be unique)*/