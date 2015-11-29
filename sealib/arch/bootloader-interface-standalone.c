
#include "config.h"
#include "ember-types.h"
#include "error.h"
#include "hal.h"
#include "memmap.h"

EmberStatus halLaunchStandaloneBootloader(int8u mode)
{
  if(BOOTLOADER_BASE_TYPE(halBootloaderAddressTable.bootloaderType) 
     == BL_TYPE_STANDALONE) {
    // should never return
    halInternalSysReset(RESET_BOOTLOADER_BOOTLOAD);
  }
  return EMBER_ERR_FATAL;
}

int16u halGetStandaloneBootloaderVersion(void)
{
  if(BOOTLOADER_BASE_TYPE(halBootloaderAddressTable.bootloaderType) 
     == BL_TYPE_STANDALONE) {
    return halGetBootloaderVersion();
  } else {
    return BOOTLOADER_INVALID_VERSION;
  }
}
