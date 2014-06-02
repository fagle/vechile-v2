// File: form-and-join-adapter.h
//
// Description: interface for adapting the form-and-join library to run on
// both an EZSP host processor and on a Zigbee processor such as the EM250.
// Application writers do not need to look at this file.  See
// form-and-join.h for information on using the library.

#ifndef __FORM_AND_JOIN_ADAPTER_H__
#define __FORM_AND_JOIN_ADAPTER_H__

#define MAXNETBUF   (0x08)

// The EmberZigbeeNetwork struct type does not include lqi and rssi, but those
// values are typically desired as well.  Rather than changing the stack API,
// bundle them together in a new struct.
typedef struct 
{
  EmberZigbeeNetwork network;
  int8u lqi;
  int8s rssi;
} NetworkInfo;

typedef struct
{
  int8u lqi;
  int8s rssi;
  int8u index;
  int8u channel;
  int16u panid;
} NetworkInfo2;

// Functions the adapters must implement.
int8u        formAndJoinStackProfile       ( void );
NetworkInfo *formAndJoinGetNetworkPointer  ( int8u index );
void         formAndJoinSetCleanupTimeout  ( void );
int8u       *formAndJoinAllocateBuffer     ( void );
void         formAndJoinReleaseBuffer      ( void );
EmberStatus  formAndJoinSetBufferLength    ( int8u entryCount );

// For use by the node adapter on expiration of the cleanup timer.
void emberFormAndJoinCleanup  ( EmberStatus status );
void scanOpenTimeOutEvent     ( void );
void scanCloseTimeOutEvent    ( void );

#endif  //  __FORM_AND_JOIN_ADAPTER_H__

