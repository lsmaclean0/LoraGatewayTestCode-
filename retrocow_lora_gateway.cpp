//
// Created by Zhijiang Chen on 7/22/18.
//

// Include the SX1272
#include "SX1272.h"

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// please uncomment only 1 choice
//#define BAND868
#define BAND900
//#define BAND433
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// For a Raspberry-based gateway the distribution uses a radio.makefile file that can define MAX_DBM
//
#ifndef MAX_DBM
    #define MAX_DBM 14
#endif

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define PRINTLN                   printf("\n")
#define PRINT_CSTSTR(fmt, param)   printf(fmt,param)
#define PRINT_STR(fmt, param)      PRINT_CSTSTR(fmt,param)
#define PRINT_VALUE(fmt, param)    PRINT_CSTSTR(fmt,param)
#define PRINT_HEX(fmt, param)      PRINT_VALUE(fmt,param)
#define FLUSHOUTPUT               fflush(stdout);

///////////////////////////////////////////////////////////////////
// DEFAULT LORA MODE
#define LORAMODE 1
// the special mode to test BW=125MHz, CR=4/5, SF=12
// on the 868.1MHz channel
//#define LORAMODE 11
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// GATEWAY HAS ADDRESS 1
#define LORA_ADDR 1
///////////////////////////////////////////////////////////////////

// set loraMode to default LORAMODE
uint8_t loraMode = LORAMODE;

uint8_t loraAddr = LORA_ADDR;

//#define SHOW_FREEMEMORY
//#define GW_RELAY
//#define RECEIVE_ALL
//#define CAD_TEST
//#define LORA_LAS
//#define WINPUT
//#define ENABLE_REMOTE

#ifdef BAND868
#define MAX_NB_CHANNEL 15
#define STARTING_CHANNEL 4
#define ENDING_CHANNEL 18
#ifdef SENEGAL_REGULATION
uint8_t loraChannelIndex=0;
#else
uint8_t loraChannelIndex=6;
#endif
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_04_868,CH_05_868,CH_06_868,CH_07_868,CH_08_868,CH_09_868,
                                            CH_10_868,CH_11_868,CH_12_868,CH_13_868,CH_14_868,CH_15_868,CH_16_868,CH_17_868,CH_18_868};

#elif defined BAND900
#define MAX_NB_CHANNEL 13
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 12
uint8_t loraChannelIndex = 5;
uint32_t loraChannelArray[MAX_NB_CHANNEL] = {CH_00_900, CH_01_900, CH_02_900, CH_03_900, CH_04_900, CH_05_900,
                                             CH_06_900, CH_07_900, CH_08_900,
                                             CH_09_900, CH_10_900, CH_11_900, CH_12_900};

#elif defined BAND433
#define MAX_NB_CHANNEL 4
#define STARTING_CHANNEL 0
#define ENDING_CHANNEL 3
uint8_t loraChannelIndex=0;
uint32_t loraChannelArray[MAX_NB_CHANNEL]={CH_00_433,CH_01_433,CH_02_433,CH_03_433};
#endif

uint32_t loraChannel = loraChannelArray[loraChannelIndex];

bool optAESgw = false;
bool optRAW = false;
bool radioON = false;
uint16_t optBW = 0;
uint8_t optCR = 0;
uint8_t optSF = 0;
uint8_t optCH = 0;
double optFQ = -1.0;
uint8_t optSW = 0x12;



/**
 * Lora Module Configuraiton
 */
void startConfig(){

    int e;

    // Set transmission mode and print the result
    PRINT_CSTSTR("%s", "^$LoRa mode ");
    PRINT_VALUE("%d", loraMode);
    PRINTLN;

    e = sx1272.setMode(loraMode);
    PRINT_CSTSTR("%s", "^$Setting mode: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    // Select frequency channel
    if (loraMode == 11) {
        // if we start with mode 11, then switch to 868.1MHz for LoRaWAN test
        // Note: if you change to mode 11 later using command /@M11# for instance, you have to use /@C18# to change to the correct channel
        e = sx1272.setChannel(CH_18_868);
        PRINT_CSTSTR("%s", "^$Channel CH_18_868: state ");
    } else {
        e = sx1272.setChannel(loraChannel);

        if (optFQ > 0.0) {
            PRINT_CSTSTR("%s", "^$Frequency ");
            PRINT_VALUE("%f", optFQ);
            PRINT_CSTSTR("%s", ": state ");
        } else {
            #ifdef BAND868
                if (loraChannelIndex>5) {
                  PRINT_CSTSTR("%s","^$Channel CH_1");
                  PRINT_VALUE("%d", loraChannelIndex-6);
                }
                else {
                  PRINT_CSTSTR("%s","^$Channel CH_0");
                  PRINT_VALUE("%d", loraChannelIndex+STARTING_CHANNEL);
                }
                PRINT_CSTSTR("%s","_868: state ");
            #elif defined BAND900
                PRINT_CSTSTR("%s", "^$Channel CH_");
                PRINT_VALUE("%d", loraChannelIndex);
                PRINT_CSTSTR("%s", "_900: state ");
            #elif defined BAND433
                //e = sx1272.setChannel(0x6C4000);
                PRINT_CSTSTR("%s","^$Channel CH_");
                PRINT_VALUE("%d", loraChannelIndex);
                PRINT_CSTSTR("%s","_433: state ");
            #endif
        }
    }

    PRINT_VALUE("%d", e);
    PRINTLN;

    // Select amplifier line; PABOOST or RFO
    #ifdef PABOOST
        sx1272._needPABOOST=true;
        // previous way for setting output power
        // loraPower='x';
        PRINT_CSTSTR("%s","^$Use PA_BOOST amplifier line");
        PRINTLN;
    #else
        // previous way for setting output power
        // loraPower='M';
    #endif

    // Select output power in dBm
    e = sx1272.setPowerDBM((uint8_t) MAX_DBM);

    PRINT_CSTSTR("%s", "^$Set LoRa power dBm to ");
    PRINT_VALUE("%d", (uint8_t) MAX_DBM);
    PRINTLN;

    PRINT_CSTSTR("%s", "^$Power: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    // get preamble length
    e = sx1272.getPreambleLength();
    PRINT_CSTSTR("%s", "^$Get Preamble Length: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;
    PRINT_CSTSTR("%s", "^$Preamble Length: ");
    PRINT_VALUE("%d", sx1272._preamblelength);
    PRINTLN;

    if (sx1272._preamblelength != 8) {
        PRINT_CSTSTR("%s", "^$Bad Preamble Length: set back to 8");
        sx1272.setPreambleLength(8);
        e = sx1272.getPreambleLength();
        PRINT_CSTSTR("%s", "^$Get Preamble Length: state ");
        PRINT_VALUE("%d", e);
        PRINTLN;
        PRINT_CSTSTR("%s", "^$Preamble Length: ");
        PRINT_VALUE("%d", sx1272._preamblelength);
        PRINTLN;
    }

    // Set the node address and print the result
    //e = sx1272.setNodeAddress(loraAddr);
    sx1272._nodeAddress = loraAddr;
    e = 0;
    PRINT_CSTSTR("%s", "^$LoRa addr ");
    PRINT_VALUE("%d", loraAddr);
    PRINT_CSTSTR("%s", ": state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    if (optAESgw)
        PRINT_CSTSTR("%s", "^$Handle AES encrypted data\n");

    if (optRAW) {
        PRINT_CSTSTR("%s", "^$Raw format, not assuming any header in reception\n");
        // when operating n raw format, the SX1272 library do not decode the packet header but will pass all the payload to stdout
        // note that in this case, the gateway may process packet that are not addressed explicitly to it as the dst field is not checked at all
        // this would be similar to a promiscuous sniffer, but most of real LoRa gateway works this way
        sx1272._rawFormat = true;
    }

    // Print a success message
    PRINT_CSTSTR("%s", "^$SX1272/76 configured ");
    PRINT_CSTSTR("%s", "as LR-BS. Waiting RF input for transparent RF-serial bridge\n");
    #if defined ARDUINO && defined GW_RELAY
        PRINT_CSTSTR("%s","^$Act as a simple relay gateway\n");
    #endif

}

/**
 * Setup Lora Device
 */
void setup(){

    int e;

    // Power ON the module
    e = sx1272.ON();
    PRINT_CSTSTR("%s", "^$**********  RetroCow Lora Gateway Power ON: state ");
    PRINT_VALUE("%d", e);
    PRINTLN;

    e = sx1272.getSyncWord();

    if(!e){
        PRINT_CSTSTR("%s", "^$Default sync word: 0x");
        PRINT_HEX("%X", sx1272._syncWord);
        PRINTLN;
    }

    if (optSW != 0x12){
        e = sx1272.setSyncWord(optSW);

        PRINT_CSTSTR("%s", "^$Set sync word to 0x");
        PRINT_HEX("%X", optSW);
        PRINTLN;
        PRINT_CSTSTR("%s", "^$LoRa sync word: state ");
        PRINT_VALUE("%d", e);
        PRINTLN;
    }

    if (!e){
        radioON = true;
        // Start configuration
        startConfig();
    }

    FLUSHOUTPUT;
    delay(1000);
}

/**
 * Application Entry
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]){

    int opt = 0;

    static struct option long_options[] = {
            {"mode", required_argument, 0, 'a'},
            {"aes", no_argument, 0, 'b'},
            {"bw", required_argument, 0, 'c'},
            {"cr", required_argument, 0, 'd'},
            {"sf", required_argument, 0, 'e'},
            {"raw", no_argument, 0, 'f'},
            {"freq", required_argument, 0, 'g'},
            {"ch", required_argument, 0, 'h'},
            {"sw", required_argument, 0, 'i'},
            {0, 0, 0, 0}
    };

    int long_index = 0;

    while((opt = getopt_long(argc, argv, "a:bc:d:e:fg:h:i",
            long_options, &long_index)) !=-1){

        switch(opt){
            case 'a':
                loraMode = atoi(optarg);
                break;
            case 'b':
                optAESgw = true;    // not used at the moment
                break;
            case 'c':
                optBW = atoi(optarg);   // 125, 250 or 500, setBW() will correct the optBW value
                break;
            case 'd':
                optCR = atoi(optarg);
                // 5, 6, 7 or 8
                // setCR() will correct the optCR value
                break;
            case 'e':
                optSF = atoi(optarg);
                // 6, 7, 8, 9, 10, 11 or 12
                break;
            case 'f':
                optRAW = true;
                break;
            case 'g':
                optFQ = atoi(optarg);   // in MHz, eg. 868.1
                loraChannel = optFQ * 1000000.0 * RH_LORA_FCONVERT;
            case 'h':
                optCH = true;
                loraChannelIndex = atoi(optarg);
                if (loraChannelIndex < STARTING_CHANNEL || loraChannelIndex > ENDING_CHANNEL)
                    loraChannelIndex = STARTING_CHANNEL;
                loraChannelIndex = loraChannelIndex - STARTING_CHANNEL;
                loraChannel = loraChannelArray[loraChannelIndex];
                break;
            case 'i':
                uint8_t sw = atoi(optarg);
                optSW = (sw/10) * 16 + (sw%10);     // assume that sw is expressed in hex value
                break;
        }
    }

    // Init
    setup();
}