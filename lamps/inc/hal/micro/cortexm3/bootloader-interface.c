/*
 * File: bootloader-interface.c
 * Description: EM3XX common bootloader HAL functions
 *
 * Copyright 2008-2009 by Ember Corporation. All rights reserved.           *80*
 */

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "hal/micro/bootloader-interface.h"
#include "hal/micro/cortexm3/memmap.h"
#include "hal/micro/cortexm3/bootloader/bootloader-common.h"

//////////////////////
// Generic bootloader functionality
///////////

/** @description Returns the bootloader type the application was
 *  built for.
 *
 * @return BL_TYPE_NULL, BL_TYPE_STANDALONE or BL_TYPE_APPLICATION
 */
BlBaseType halBootloaderGetType(void)
{
  // Important distinction:
  //   this returns what bootloader the app was built for
  #ifdef NULL_BTL
    BLDEBUG_PRINT("built for NULL bootloader\r\n");
    return BL_TYPE_NULL;
  #elif defined APP_BTL
    // Deprecated
    BLDEBUG_PRINT("built for APP bootloader\r\n");
    return BL_TYPE_APPLICATION;
  #elif defined SERIAL_UART_BTL
    // Deprecated
    BLDEBUG_PRINT("built for SERIAL UART standalone bootloader\r\n");
    return BL_TYPE_STANDALONE;
  #elif defined SERIAL_OTA_BTL
    // Deprecated
    BLDEBUG_PRINT("built for SERIAL OTA standalone bootloader\r\n");
    return BL_TYPE_STANDALONE;
  #else
    BLDEBUG_PRINT("built for 8k bootloader\r\n");
    return BL_TYPE_BOOTLOADER;
  #endif
}


// Returns the extended type of 
BlExtendedType halBootloaderGetInstalledType(void)
{
  // Important distinction:
  //   this returns what bootloader is actually present on the chip
  return halBootloaderAddressTable.bootloaderType;
}

int16u halGetBootloaderVersion(void)
{
  return halBootloaderAddressTable.bootloaderVersion;
}
