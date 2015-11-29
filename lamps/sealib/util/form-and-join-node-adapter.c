
#include "config.h"
#include "ember.h"
#include "form-and-join-adapter.h"
#include "serial.h"

extern void halReboot();

//////////////////////////////////////////////////////////////////////////////////////////
// We use message buffers for caching energy scan results,
// pan id candidates, and joinable beacons.
static EmberMessageBuffer dataCache = EMBER_NULL_MESSAGE_BUFFER;

static EmberEventControl cleanupEvent = {0x00, 0x00};
static EmberEventControl scanTimeOutEvent = {0x00, 0x00};
#define CLEANUP_TIMEOUT_QS    (120)

//////////////////////////////////////////////////////////////////////////////////////////
int8u formAndJoinStackProfile ( void )
{
    return emberStackProfile();
}

///////////////////////////////////////////////////////////////////////////////////////////
// We're relying on the fact that message buffers are a multiple of 16 bytes 
// in size, so that NetworkInfo records do not cross buffer boundaries.
NetworkInfo *formAndJoinGetNetworkPointer ( int8u index )
{
    return (NetworkInfo *)emberGetLinkedBuffersPointer(dataCache, index << 0x04);
}

///////////////////////////////////////////////////////////////////////////////////////////
void formAndJoinSetCleanupTimeout ( void )
{
    emberEventControlSetDelayQS(cleanupEvent, CLEANUP_TIMEOUT_QS);
}

void scanOpenTimeOutEvent( void )
{
    Debug("\r\nscanTimeOutEvent.status = %d,scanTimeOutEvent.timeToExcute = %d",scanTimeOutEvent.status,scanTimeOutEvent.timeToExecute);
    emberEventControlSetDelayQS(scanTimeOutEvent,40);  //10s
    Debug("\r\nscanTimeOutEvent.status = %d,scanTimeOutEvent.timeToExcute = %d",scanTimeOutEvent.status,scanTimeOutEvent.timeToExecute);
}

void scanCloseTimeOutEvent( void )
{
    Debug("\r\nscanTimeOutEvent.status = %d,scanTimeOutEvent.timeToExcute = %d",scanTimeOutEvent.status,scanTimeOutEvent.timeToExecute);
    emberEventControlSetInactive(scanTimeOutEvent);
    Debug("\r\nscanTimeOutEvent.status = %d,scanTimeOutEvent.timeToExcute = %d",scanTimeOutEvent.status,scanTimeOutEvent.timeToExecute);
}

///////////////////////////////////////////////////////////////////////////////////////////
int8u *formAndJoinAllocateBuffer ( void )
{
    dataCache = emberAllocateStackBuffer();
    return (dataCache == EMBER_NULL_MESSAGE_BUFFER ? NULL : emberMessageBufferContents(dataCache));
}

///////////////////////////////////////////////////////////////////////////////////////////
// Set the dataCache length in terms of the number of NetworkInfo entries.
EmberStatus formAndJoinSetBufferLength ( int8u entryCount )
{
    return emberSetLinkedBuffersLength(dataCache, entryCount << 0x04);
}

///////////////////////////////////////////////////////////////////////////////////////////
void formAndJoinReleaseBuffer ( void )
{
    if (dataCache != EMBER_NULL_MESSAGE_BUFFER) 
    {
        emberReleaseMessageBuffer(dataCache);
        dataCache = EMBER_NULL_MESSAGE_BUFFER;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
static void cleanupEventHandler ( void )
{
    emberEventControlSetInactive(cleanupEvent);
    emberFormAndJoinCleanup(EMBER_SUCCESS);
}

static void scanTimeOutEventHandler ( void )
{
    emberEventControlSetInactive(scanTimeOutEvent);
    Debug("\r\n[scan]goto scanTimeOutEventHandler");
    halReboot();
}

static EmberEventData formAndJoinEvents[] =
  { { &cleanupEvent, cleanupEventHandler },
    { &scanTimeOutEvent, scanTimeOutEventHandler },
    { NULL, NULL } };       // terminator

///////////////////////////////////////////////////////////////////////////////////////////
void emberFormAndJoinTick ( void )
{
    emberRunEvents(formAndJoinEvents);
}

///////////////////////////////////////////////////////////////////////////////////////////
