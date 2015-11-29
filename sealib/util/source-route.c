
#include "config.h"

#ifdef SINK_APP

#include "ember.h"
#include "security.h"
#include "source-route-common.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// AppBuilder includes this file and uses the define below to turn off source routing. This doesnt affect non-AppBuilder applications.
#ifndef ZA_NO_SOURCE_ROUTING

/////////////////////////////////////////////////////////////////////////////////////////////
// ZigBee protocol limitations effectively constrain us to a 11-hop source route in the worst case scenario (for a 12-hop 
// delivery in total), so we enforce a threshold value on the relayCount and ignore source route operations beyond
// this threshold.  See explanatory comment in emberAppendSourceRouteHandler() for more details about this.
// This value could be reduced at the designer's discretion, but not increased beyond 11 without causing routing problems.
#define MAX_RELAY_COUNT    (11)
   
#ifndef EXTERNAL_TABLE
static SourceRouteTableEntry table[EMBER_SOURCE_ROUTE_TABLE_SIZE];
int8u sourceRouteTableSize = EMBER_SOURCE_ROUTE_TABLE_SIZE;
SourceRouteTableEntry *sourceRouteTable = table;
#endif

void emberIncomingRouteRecordHandler ( EmberNodeId source,
                                       EmberEUI64 sourceEui,
                                       int8u relayCount,
                                       EmberMessageBuffer header,
                                       int8u relayListIndex )
{
    int8u previous;
    int8u i;

    if (sourceRouteTableSize == 0x00 || relayCount > MAX_RELAY_COUNT) 
        return;                                          // Ignore over-sized source routes, since we can't reuse them reliably anyway.

    previous = sourceRouteAddEntry(source, NULL_INDEX);  // The source of the route record is furthest from the gateway. We start there and work closer.
    for (i = 0x00; i < relayCount; i ++)                 // Go through the relay list and add them to the source route table.
    {
        int8u    index = relayListIndex + (i << 0x01);
        EmberNodeId id = emberGetLinkedBuffersLowHighInt16u(header, index);
        previous = sourceRouteAddEntry(id, previous);    // We pass the index of the previous entry to link the route together.
    }
  
    securityAddToAddressCache(source, sourceEui);        // If the following message has APS Encryption, our node will need to know
                                                         // the IEEE Address of the source in order to properly decrypt.
}



void emberAppendSourceRouteHandler ( EmberNodeId destination, EmberMessageBuffer header )
{
    int8u foundIndex   = sourceRouteFindIndex(destination);
    int8u relayCount   = 0x00;
    int8u bufferLength = emberMessageBufferLength(header);
    int8u i;

    if (foundIndex == NULL_INDEX) 
        return;

    i = foundIndex;             // Find out what the relay count is.
    while (sourceRouteTable[i].closerIndex != NULL_INDEX) 
    {
        i = sourceRouteTable[i].closerIndex;
        relayCount++;
    }

    // Make room for relay count (1 byte), relay index (1 byte), relay list (2 bytes per relay).
    if (emberSetLinkedBuffersLength(header, bufferLength + 0x02 + (relayCount << 0x01)) != EMBER_SUCCESS) 
        return;

    emberSetLinkedBuffersByte(header, bufferLength, relayCount);  // Set the relay count and relay index.
    emberSetLinkedBuffersByte(header, bufferLength + 1, (relayCount > 0 ? relayCount - 1 : 0));

    relayCount = 0x00;    // Fill in the relay list. The first relay in the list is the closest to the destination (furthest from the gateway).
    i = foundIndex;
    while (sourceRouteTable[i].closerIndex != NULL_INDEX) 
    {
        i = sourceRouteTable[i].closerIndex;
        emberSetLinkedBuffersLowHighInt16u(header, bufferLength + 2 + (relayCount << 1), sourceRouteTable[i].destination);
        relayCount++;
    }

    // Per Ember Case 10096, we need to protect against oversized source routes that will overflow the PHY packet length 
    // (127).  Since the worst-case packet overhead is the APS-encrypted, NWK-encrypted, tunneled key delivery sent by 
    // the Trust Center during authentication of new devices, and this leaves enough only room for an 11-hop source route 
    // frame, we abort our route-appending operation if the resulting source route contains more than 11 relays. Unless 
    // a route already exists to the destination, this will likely result in the stack not sending the packet at all, but
    // this is better than forcing too much data into the packet (which, as of this writing, will trigger an assert 
    // in mac.c and cause a reset).
    if ( relayCount > MAX_RELAY_COUNT) // Setting the buffer length back to its value at the start of this callback effectively chops off the appended source route data.
        emberSetLinkedBuffersLength(header, bufferLength);
}
#endif // ZA_NO_SOURCE_ROUTING

#endif // SINK_APP