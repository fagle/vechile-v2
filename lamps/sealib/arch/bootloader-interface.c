
#include "config.h"
#include "ember-types.h"
#include "bootloader-interface.h"
#include "memmap.h"
#include "bootloader-common.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Generic bootloader functionality
//

///////////////////////////////////////////////////////////////////////////////////////////
/** @description Returns the bootloader type the application was
 *  built for.
 *
 * @return BL_TYPE_NULL, BL_TYPE_STANDALONE or BL_TYPE_APPLICATION
 */
BlBaseType halBootloaderGetType ( void )
{
#ifdef NULL_BTL      // Important distinction: this returns what bootloader the app was built for
    BLDEBUG_PRINT("built for NULL bootloader\r\n");
    return BL_TYPE_NULL;
#elif defined APP_BTL
    BLDEBUG_PRINT("built for APP bootloader\r\n");
    return BL_TYPE_APPLICATION;
#elif defined SERIAL_UART_BTL
    BLDEBUG_PRINT("built for SERIAL UART bootloader\r\n");
    return BL_TYPE_APPLICATION;
#else
    BLDEBUG_PRINT("built for Standalone bootloader\r\n");
    return BL_TYPE_STANDALONE;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
// Returns the extended type of 
BlExtendedType halBootloaderGetInstalledType ( void )
{
    // Important distinction:   this returns what bootloader is actually present on the chip
    return halBootloaderAddressTable.bootloaderType;
}

///////////////////////////////////////////////////////////////////////////////////////////
int16u halGetBootloaderVersion ( void )
{
    return halBootloaderAddressTable.bootloaderVersion;
}
