#include "serial.h"
#include "ember.h"
#include "mem.h"
#include "security-common.h"


/////////////////////////////////////////////////////////////////////////////////////////////
//
boolean nodeSecurityInit ( EmberKeyData * preconfiguredKey )
{
    EmberInitialSecurityState state;
  
    sea_memset(&state, 0x00, sizeof(state));
    if (preconfiguredKey) 
    {
        sea_memcpy(emberKeyContents(&(state.preconfiguredKey)), emberKeyContents(preconfiguredKey), EMBER_ENCRYPTION_KEY_SIZE);
    }
    state.bitmask = (EMBER_STANDARD_SECURITY_MODE | ( preconfiguredKey ? (EMBER_HAVE_PRECONFIGURED_KEY |
                     EMBER_REQUIRE_ENCRYPTED_KEY) : EMBER_GET_LINK_KEY_WHEN_JOINING));
    //return (EMBER_SUCCESS == emberSetInitialSecurityState(&state));
    
    EmberStatus tempstatus;
    tempstatus = emberSetInitialSecurityState(&state);
    Debug("\r\nemberSetInitialSecurityState is %d",tempstatus);
    return (EMBER_SUCCESS == tempstatus);
}
