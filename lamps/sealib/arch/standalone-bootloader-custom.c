/*
 * File: standalone-bootloader-custom.c
 * Description: Customizeable portions of the standalone bootloader
 *
 *
 * Copyright 2009 by Ember Corporation. All rights reserved.                *80*
 */
//[[ Author(s): David Iacobone, diacobone@ember.com
//              Lee Taylor, lee@ember.com
//]]

#include PLATFORM_HEADER
#include "bootloader-gpio.h"
#include "bootloader-common.h"
#include "bootloader-serial.h"
#include "hal/micro/cortexm3/memmap.h"
#include "standalone-bootloader.h"


#ifdef CORTEXM3_EM351
  #define MY_EM3XX "EM351 "
#elif defined CORTEXM3_EM357
  #define MY_EM3XX "EM357 "
#elif defined CORTEXM3_STM32W108
  #define MY_EM3XX "STM32W "
#endif

#ifdef NO_RADIO
  #define MY_RADIO "Serial "
#else
  #define MY_RADIO ""
#endif

const char *bootName = MY_EM3XX MY_RADIO "Bootloader";

#define PRINT_STRING(str) serPutStr(str)

static void printImageInfo(void)
{
  int8u i = 0;
  int8u ch;

  PRINT_STRING("\r\n");

  if(halAppAddressTable.baseTable.type != APP_ADDRESS_TABLE_TYPE) {
    PRINT_STRING(" ** no app **\r\n"); 
    return;
  }

  serPutChar('"');
  while ((i < 32) && ( (ch = halAppAddressTable.imageInfo[i++]) != 0) )
    serPutChar(ch);
  serPutChar('"');

  PRINT_STRING("\r\n");
}

// print the menu
static void printMenu(void)
{
  static const char *menu_string = 
          "\r\n\r\n"
          "1. upload ebl\r\n"
          "2. run\r\n"
          "3. ebl info\r\n"
          "BL > ";

  PRINT_STRING("\r\n");
  PRINT_STRING(bootName);
  PRINT_STRING(" v");
  serPutHex(HIGH_BYTE(halBootloaderAddressTable.bootloaderVersion));
  PRINT_STRING(" b");
  serPutHex(LOW_BYTE(halBootloaderAddressTable.bootloaderVersion));
 
  PRINT_STRING(menu_string);
}

void bootloaderMenu(void)
{
  int8u ch;
  int16u status;

  halResetWatchdog();
  serGetFlush();

  BL_STATE_UP();   // indicate bootloader is up

  while(1) {
    // loop polling the serial and radio channels for bootload activity
    halResetWatchdog();
    BL_STATE_POLLING_LOOP();   // indicate we're polling for input

    #ifndef NO_RADIO  // Note: RADIO is not yet supported
      status = checkOtaStart();
      if (status == BL_SUCCESS) { 
        status = receiveOtaImage();
        #ifdef BL_DEBUG
        // only print to uart in debug mode

          PRINT_STRING("\r\nOTA upload ");
          if (status == BL_SUCCESS) {
            BL_STATE_DOWNLOAD_SUCCESS();   // indicate successful download
            PRINT_STRING("complete\r\n");
            BL_STATE_DOWN();   // going down
            return; // return will reset to newly downloaded application
          } else {
            BL_STATE_DOWNLOAD_FAILURE();   // indicate download failure
            PRINT_STRING("aborted: stat=0x\r\n");
            serPutHexInt(status);
          }
        #endif
      }
    #endif
    
    // check serial interface for a command entry
    status = serGetChar(&ch);
    if(status == BL_SUCCESS) {
      serPutChar(ch); // echo
      serGetFlush();  // flush other incoming chars

      switch(ch) {
        case '1': // upload app
          PRINT_STRING("\r\nbegin upload\r\n");
          status = receiveSerialImage();

          PRINT_STRING("\r\nSerial upload ");
          if(status == BL_SUCCESS) {
            BL_STATE_DOWNLOAD_SUCCESS();   // indicate successful download
            PRINT_STRING("complete\r\n");
          } else {
            BL_STATE_DOWNLOAD_FAILURE();   // indicate download failure
            PRINT_STRING("aborted\r\n");
            if (status & BLOCKERR_MASK) {
              PRINT_STRING("\r\nXModem block error 0x");
            } else if (status & BL_ERR_MASK) {
              PRINT_STRING("\r\ndownload file error 0x");
            } else {
              PRINT_STRING("\r\nerror 0x");
            }
            serPutHexInt(status);
            PRINT_STRING("\r\n");
          }
          break;
        case '2': // jump to app
          BL_STATE_DOWN();
          return;  // return will reset to the application
        case '3': // image info
          printImageInfo();
          break;
        case '\r':
        case '\n':
          printMenu();
          break;
        default:
          // allow the main bootloader code to check for debug menu options
          //  normal builds do not have any debug options, so this call
          //  is optional
          checkDebugMenuOption(ch);
          break;
      }
    }
  }
}

