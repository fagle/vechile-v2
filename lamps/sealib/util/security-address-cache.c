
#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"
#include "security.h"

//------------------------------------------------------------------------------

int8u addressCacheSize = 0;             
static int8u addressCacheStartIndex;
static int8u nextIndex;                 // relative to addressCacheStartIndex

//------------------------------------------------------------------------------

void securityAddressCacheInit(int8u securityAddressCacheStartIndex,
                              int8u securityAddressCacheSize)
{
  addressCacheStartIndex = securityAddressCacheStartIndex;
  addressCacheSize = securityAddressCacheSize;
  nextIndex = 0;
}

//------------------------------------------------------------------------------
// The Trust Center Address cache keeps track of EUI64s for the purpose of
// sending APS Encrypted commands.  During joining the Trust Center
// will send an APS Encrypted command to the parent of the device and needs to 
// know their long address for creating the Nonce used in encryption.

// We simply loop through our cache and overwrite the last one received.

void securityAddToAddressCache(EmberNodeId nodeId, EmberEUI64 nodeEui64)
{
  int8u index = nextIndex;
  int8u i;

  if (addressCacheSize == 0) {
    return;
  }

  // Search through our cache for an existing IEEE with the same info.
  // If it exists update that.
  for (i = 0; i < addressCacheSize; i++) {
    EmberEUI64 eui64;
    emberGetAddressTableRemoteEui64(addressCacheStartIndex + i, eui64);
    if (MEMCOMPARE(eui64, nodeEui64, EUI64_SIZE) == 0) {
      index = i;
      break;
    }
  }

  if (index == nextIndex) {
    nextIndex += 1;
    if (nextIndex == addressCacheSize)
      nextIndex = 0;
  }

  index += addressCacheStartIndex;
  if (emberSetAddressTableRemoteEui64(index, nodeEui64)
      == EMBER_SUCCESS) {
    emberSetAddressTableRemoteNodeId(index, nodeId);
  }
}
