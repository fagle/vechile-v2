
#include "config.h"
#include "ember.h"
#include "ami-inter-pan.h"

static int8u* pushInt16u ( int8u *finger, int16u value );
static int8u* pushEui64  ( int8u *finger, int8u *value );

// Defined in the MAC code.  We use this to keep our sequence numbers
// in step with other transmissions.
extern int8u emMacDataSequenceNumber;

#define SHORT_DEST_FRAME_CONTROL   (0xC821)                // The two possible MAC frame controsl.
#define LONG_DEST_FRAME_CONTROL    (0xCC21)
#define MAX_INTER_PAN_MAC_SIZE     (2 + 1 + 2 + 8 + 2 + 8) // Frame control, sequence, dest PAN ID, dest, source PAN ID, source.
#define STUB_NWK_SIZE              (0x02)                  // NWK stub frame has two control bytes.
#define STUB_NWK_FRAME_CONTROL     (0x000B)
#define MAX_STUB_APS_SIZE          (1 + 2 + 2 + 2)         // APS frame control, group ID, cluster ID, profile ID

#define MAX_INTER_PAN_HEADER_SIZE  (MAX_INTER_PAN_MAC_SIZE + STUB_NWK_SIZE + MAX_STUB_APS_SIZE)

EmberMessageBuffer makeInterPanMessage ( InterPanHeader *headerData, EmberMessageBuffer payload )
{
    int8u header[MAX_INTER_PAN_HEADER_SIZE];
    int8u *finger = header;
    int16u macFrameControl =  headerData->hasLongAddress ? LONG_DEST_FRAME_CONTROL : SHORT_DEST_FRAME_CONTROL;

    finger    = pushInt16u(finger, macFrameControl);
    *finger++ = ++emMacDataSequenceNumber;

    finger = pushInt16u(finger, headerData->panId);
    if (headerData->hasLongAddress)
        finger = pushEui64(finger, headerData->longAddress);
    else
        finger = pushInt16u(finger, headerData->shortAddress);

    finger = pushInt16u(finger, emberGetPanId());
    finger = pushEui64(finger, emberGetEui64());

    finger = pushInt16u(finger, STUB_NWK_FRAME_CONTROL);

    *finger++ = headerData->messageType;
    if (headerData->messageType == INTER_PAN_MULTICAST)
        finger = pushInt16u(finger, headerData->groupId);
    finger = pushInt16u(finger, headerData->clusterId);
    finger = pushInt16u(finger, headerData->profileId);

    {
        int8u headerLength = finger - header;
        int8u payloadLength = emberMessageBufferLength(payload);
        EmberMessageBuffer message = emberFillLinkedBuffers(NULL, headerLength + payloadLength);
        if (message != EMBER_NULL_MESSAGE_BUFFER) 
        {
            emberCopyToLinkedBuffers(header,  message, 0x00, headerLength);
            emberCopyBufferBytes(message, headerLength, payload, 0x00, payloadLength);
        }
        return message;
    }
}                                

// Returns the index of the start of the payload, or zero if the message is not correctly formed.
int8u parseInterPanMessage ( EmberMessageBuffer message,  int8u startOffset, InterPanHeader *headerData )
{
    int8u header[MAX_INTER_PAN_HEADER_SIZE];
    int8u length = sizeof(header);
    int8u haveBytes = emberMessageBufferLength(message) - startOffset;
    int8u *finger = header;
    int16u macFrameControl;

    if (haveBytes < length)
        length = haveBytes;

    emberCopyFromLinkedBuffers(message, startOffset, header, length);
  
    macFrameControl = HIGH_LOW_TO_INT(finger[1], finger[0]);

    // verify the MAC Frame Control - ignore the Ack required bit as the stack changes the value to 0 during processing. 
    // Increment past the FC, seq, dest PAN ID and dest addr
    if ((macFrameControl & 0xFFDF) == (LONG_DEST_FRAME_CONTROL & 0xFFDF)) 
    {
        finger += 2 + 1 + 2 + 8;      // control, sequence, dest PAN ID, long dest
    }
    else if ((macFrameControl & 0xFFDF) == (SHORT_DEST_FRAME_CONTROL & 0xFFDF)) 
    {
        finger += 2 + 1 + 2 + 2;      // control, sequence, dest PAN ID, short dest
    }
    else 
    {
    //fprintf(stderr, "bad MAC frame control %04X\n", macFrameControl);
    //emberSerialPrintf(1, "bad MAC frame control %2x\r\n", macFrameControl);
    return 0;
  }

  // after the dest PAN ID and dest addr is the source PAN ID
  headerData->panId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

  // after the source PAN ID is the long source address
  headerData->hasLongAddress = TRUE;
  MEMCOPY(headerData->longAddress, finger, 8);
  finger += 8;

  // verify the stub network frame control
  if (HIGH_LOW_TO_INT(finger[1], finger[0]) != STUB_NWK_FRAME_CONTROL) {
    //fprintf(stderr, "bad NWK frame control %02X\n", *(finger - 1));
    //emberSerialPrintf(1, "bad NWK frame control %x%x\r\n", *(finger - 1), *finger);
    return 0;
  }
  finger += 2;
  
  // message type - this is the APS frame control
  headerData->messageType = *finger++;
  
  if (headerData->messageType == INTER_PAN_MULTICAST) {
    headerData->groupId = HIGH_LOW_TO_INT(finger[1], finger[0]);
    finger += 2;
  } else if (! (headerData->messageType == INTER_PAN_UNICAST
                || headerData->messageType == INTER_PAN_BROADCAST)) {
    //fprintf(stderr, "bad APS frame control %02X\n", headerData->messageType);
    //emberSerialPrintf(1, "bad APS frame control %2x\r\n", headerData->messageType);
    return 0;
  }
  
  headerData->clusterId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;
  headerData->profileId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

//  fprintf(stderr, "[used %d out of %d]\n",
//          finger - header,
//          haveBytes);

  if (finger - header <= haveBytes) {
    //emberSerialPrintf(1, "ret %x\r\n", startOffset + finger - header);
    return startOffset + finger - header;
    
  } else {
    //emberSerialPrintf(1, "ret zero\r\n");
    return 0;
  }
}

//----------------------------------------------------------------
// Utilities

static int8u* pushInt16u(int8u *finger, int16u value)
{
  *finger++ = LOW_BYTE(value);
  *finger++ = HIGH_BYTE(value);
  return finger;
}

static int8u* pushEui64(int8u *finger, int8u *value)
{
  MEMCOPY(finger, value, 8);
  return finger + 8;
}

