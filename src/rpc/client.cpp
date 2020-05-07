//
// Created by Alexey on 06/03/2020.
//
/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include <memory.h> /* for memset */
#include "pci_sniff.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "common.h"
/*****************************************************************************/
/**                                Namespace                                **/
/*****************************************************************************/
using namespace std;

/*****************************************************************************/
/**                            Type definitions                             **/
/*****************************************************************************/
/**
 * CLI command definitions
 */
typedef enum parserInteractiveMenu_e{
    RPC_START=0,
    RPC_PAUSE,
    RPC_CHANGE_INTERVAL,
    RPC_STOP
}parserInteractiveMenu;

/*****************************************************************************/
/**                            Static variables                             **/
/*****************************************************************************/
/**
 * Main parser context instance
 */
static parserContext_t parserContext;
static CLIENT *cl;

/**
 * Aux interactive handler for interval value parsing
 * @param menuPtr RPC message
 * @return
 */
static int interactive_menu_parse_interval(){
    int result = 0;
    int erval=0;

    PARSER_PRINT("Enter new time interval in usec for packets streaming\n");
    result = scanf("%d", (int*)(&erval));
    if(result <= 0){
        PARSER_DBG("Failed to parse interval\n");
        scanf("%*s");
        return -1;
    }

    return erval;
}

/**
 * Clients entry point to main loop after successful connection
 * Main process handles RPC message flow to server and nested thread handles
 * CLI user interface
 * @param context parser context instance
 * @return PARSER_SUCCESS on success, PARSER_ERROR otherwise
 */
static int main_loop(){
    pthread_t cliThread;
    int result;
    int *rpc_res;
    int erval = 1;

    PARSER_DBG("Starting main loop\n");
    while (1){
        parserInteractiveMenu menuItem = RPC_START;
        PARSER_PRINT("============================================\n"
                     " Enter next command for RPC:\n"
                     "    %d - Start streaming meta-data packets\n"
                     "    %d - Pause streaming process\n"
                     "    %d - Change streaming interval\n"
                     "    %d - Stop streaming process and exit\n"
                     "============================================\n"
        , RPC_START, RPC_PAUSE, RPC_CHANGE_INTERVAL, RPC_STOP);


        result = scanf("%d", (int*)&menuItem);
        if(result <= 0){
            PARSER_DBG("Failed to parse selection\n");
            scanf("%*s"); // clear invalid char(s) from stdin
            return 0;
        }

        switch (menuItem){
            case RPC_START:
                PARSER_PRINT("Sending RPC start command\n");
                rpc_res = start_1(NULL, cl);
                break;
            case RPC_PAUSE:
                PARSER_PRINT("Sending RPC pause command\n");
                rpc_res = pause_1(NULL, cl);
                break;
            case RPC_CHANGE_INTERVAL:
                // Pause the streaming until user enters new value
                erval = interactive_menu_parse_interval();
                PARSER_PRINT("Sending RPC change interval to %d. \n", erval);
                if(erval < 0){
                    return PARSER_SUCCESS;
                }
                rpc_res = interval_1(&erval, cl);
                break;
            case RPC_STOP:
                PARSER_PRINT("Sending RPC stop command\n");
                stop_1(NULL, cl);
                return 1;
            default:
                PARSER_DBG("Invalid RPC command\n");
        }

        if (rpc_res == NULL){
            return PARSER_ERROR; //TODO: change to timeout
        }
        if (*rpc_res<0){
            return PARSER_ERROR;
        }

        PARSER_DBG("Send completed successfully\n");
    }
}


/*****************************************************************************/
/**                                  Main                                   **/
/*****************************************************************************/
int main(int argc, char **argv) {
    int result = PARSER_ERROR;
    memset(&parserContext, 0, sizeof(parserContext_t));

    cl = clnt_create(argv[1], PCISNIFF, PCISNIFF_V1, "tcp");
    if (cl == NULL) {
        printf("error: could not connect to server.\n");
        return 1;
    }
    print_header_common();
    PARSER_PRINT("Starting in client mode\n");
    result = main_loop();
    //return result;
    return result;
}


