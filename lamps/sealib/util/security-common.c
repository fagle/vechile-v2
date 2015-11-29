
#include "config.h"
#include "ember.h"

#if defined EZSP_HOST
boolean emberHaveLinkKey(EmberEUI64 remoteDevice)
{
  EmberKeyStruct keyStruct;

  // Check and see if the Trust Center is the remote device first.
  if (EMBER_SUCCESS == emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &keyStruct)) {
    if (0 == MEMCOMPARE(keyStruct.partnerEUI64, remoteDevice, EUI64_SIZE)) {
      return TRUE;
    }
  }

  return (0xFF != emberFindKeyTableEntry(remoteDevice, TRUE));        // look for link keys?
}
#endif
