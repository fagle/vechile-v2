/**
 * @file ami-inter-pan.h
 * @brief Utilities for sending and receiving ZigBee AMI InterPAN messages.
 * See @ref message for documentation.
 *
 * <!--Copyright 2008 by Ember Corporation. All rights reserved.         *80*-->
 */
 
/** @addtogroup message 
 *
 * See also ami-inter-pan.h for source code.
 * @{
 */
#ifndef __AMI_INTER_PAN_H__
#define __AMI_INTER_PAN_H__

// The three types of inter-PAN messages.  The values are actually the
// corresponding APS frame controls.
//
// 0x03 is the special interPAN message type.  Unicast mode is 0x00,
// broadcast mode is 0x08, and multicast mode is 0x0C.
//

#define INTER_PAN_UNICAST   0x03
#define INTER_PAN_BROADCAST 0x0B
#define INTER_PAN_MULTICAST 0x0F

// A struct for keeping track of all of the header info.

typedef struct {
  int8u messageType;            // one of the INTER_PAN_...CAST values

  // MAC addressing
  // For outgoing messages this is the destination.  For incoming messages
  // it is the source, which always has a long address.
  int16u panId;
  boolean hasLongAddress;       // always TRUE for incoming messages
  EmberNodeId shortAddress;
  EmberEUI64 longAddress;

  // APS data
  int16u profileId;
  int16u clusterId;
  int16u groupId;               // only used for INTER_PAN_MULTICAST
} InterPanHeader;

// This returns a message suitable for passing to emberSendRawMessage().
EmberMessageBuffer makeInterPanMessage(InterPanHeader *headerData,
                                       EmberMessageBuffer payload);

// This is meant to be called on the message and offset values passed
// to emberMacPassthroughMessageHandler(...).  The header is parsed and
// the various fields are written to the InterPanHeader.  The returned
// value is the offset of the payload in the message, or 0 if the
// message is not a correctly formed AMI interPAN message.
int8u parseInterPanMessage(EmberMessageBuffer message,
                           int8u startOffset,
                           InterPanHeader *headerData);

#endif