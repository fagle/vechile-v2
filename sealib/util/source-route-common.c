
#include "config.h"

#ifdef SINK_APP

#include "ember-types.h"
#include "source-route-common.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// AppBuilder includes this file and uses the define below to turn off source
// routing. This doesnt affect non-AppBuilder applications.
#ifndef ZA_NO_SOURCE_ROUTING

/////////////////////////////////////////////////////////////////////////////////////////////
// The number of entries in use.
static int8u entryCount = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
// The index of the most recently added entry.
static int8u newestIndex = NULL_INDEX;

/////////////////////////////////////////////////////////////////////////////////////////////
// Return the index of the entry with the specified destination.
int8u sourceRouteFindIndex ( EmberNodeId id )
{
    int8u i;
    for (i = 0x00; i < entryCount; i ++) 
    {
        if (sourceRouteTable[i].destination == id)
            return i;
    }
    return NULL_INDEX;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Create an entry with the given id or update an existing entry. furtherIndex
// is the entry one hop further from the gateway.
int8u sourceRouteAddEntry ( EmberNodeId id, int8u furtherIndex )
{
    int8u index = sourceRouteFindIndex(id);  // See if the id already exists in the table.
    int8u i;

    if (index == NULL_INDEX) 
    {
        if (entryCount < sourceRouteTableSize)    // No existing entry. Table is not full. Add new entry.
        {
            index = entryCount;
            entryCount += 0x01;
        } 
        else                                      // No existing entry. Table is full. Replace oldest entry.
        {
            index = newestIndex;
            while (sourceRouteTable[index].olderIndex != NULL_INDEX) 
                index = sourceRouteTable[index].olderIndex;
        }
    }

    if (index != newestIndex)                     // Update the pointers (only) if something has changed.
    {
        for (i = 0x00; i < entryCount; i ++) 
        {
            if (sourceRouteTable[i].olderIndex == index) 
            {
                sourceRouteTable[i].olderIndex = sourceRouteTable[index].olderIndex;
                break;
            }
        }
        sourceRouteTable[index].olderIndex = newestIndex;
        newestIndex = index;
    }

    sourceRouteTable[index].destination = id;    // Add the entry.
    sourceRouteTable[index].closerIndex = NULL_INDEX;

    if (furtherIndex != NULL_INDEX)              // The current index is one hop closer to the gateway than furtherIndex.
        sourceRouteTable[furtherIndex].closerIndex = index;

    return index;                                // Return the current index to save the caller having to look it up. 
}  

#endif //ZA_NO_SOURCE_ROUTING

#endif //#ifdef SINK_APP