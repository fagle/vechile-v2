#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// ray added:共用的函数、变量
// ++++++++++++++++++++++++++++++++++++++++++++++++++
//++common globle and varibles
boolean buttonZeroPress = FALSE;  // for button push (see halButtonIsr)
boolean buttonOnePress  = FALSE;  // for button push (see halButtonIsr)

/////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void halButtonIsr ( void )
//* 功能        :  Callback from the HAL when a button state changes
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : WARNING: this callback is an ISR so the best approach is to set a
//                flag here when an action should be taken and then perform the action
//                somewhere else. In this case the actions are serviced in the 
//                OnNetTick function
//*------------------------------------------------*/
void halButtonIsr ( int8u button, int8u state )
{
    if (button == BUTTON0 && state == BUTTON_PRESSED)  // button 0 (button 0 on the dev board) was pushed down
        buttonZeroPress = TRUE;

    if (button == BUTTON1 && state == BUTTON_PRESSED)  // button 1 (button 1 on the dev board) was pushed down
        buttonOnePress = TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// 结束：ray added :共用的变量、函数 
// ++++++++++++++++++++++++++++++++++++++++++++++++++
// utility function, used below in printing the security keys that are set
// 
void sensorPrintKeyOptions ( EmberKeyStruct * key )
{
    Debug( "\r\n    seqNum [");  // sequence number
    if ((key->bitmask) & EMBER_KEY_HAS_SEQUENCE_NUMBER) 
    {
        Debug( "%x] ", key->sequenceNumber);
    } 
    else 
    {
        Debug( "none] ");
    }

    Debug( "\r\n    outFC [");  // outgoing frame counter
    if (key->bitmask & EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER ) 
    {
        Debug( "%4x] ", key->outgoingFrameCounter);
    } 
    else 
    {
        Debug( "none] ");
    }

    Debug( "\r\n    inFC [");  // incoming frame counter
    if (key->bitmask & EMBER_KEY_HAS_INCOMING_FRAME_COUNTER ) 
    {
        Debug( "%4x] ", key->incomingFrameCounter);
    } 
    else 
    {
        Debug( "none] ");
    }

    Debug( "\r\n    partner [");  // partner EUI
    if (key->bitmask & EMBER_KEY_HAS_PARTNER_EUI64) 
    {
        printEUI64((EmberEUI64*) key->partnerEUI64);
        Debug( "]");
    } 
    else 
    {
        Debug( "None] ");
    }

    Debug( "\r\n    auth [");  // is key authorized
    if (key->bitmask & EMBER_KEY_IS_AUTHORIZED) 
    {
        Debug( "Y] ");
    } 
    else 
    {
        Debug( "N] ");
    }
    Debug( "\r\n"); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
// utility function, used below in printing the security keys that are set
// 
void sensorCommonPrint16ByteKey ( int8u * key ) 
{
    for (u8 i = 0x00; i < EMBER_ENCRYPTION_KEY_SIZE; i ++)  // To save on buffers, we reduce the number of calls to 
        Debug( "%02x ", key[i]); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Set the security keys and the security state - specific to this 
// application, all variants of this application (sink, sensor, 
// sleepy-sensor, mobile-sensor) need to use the same security setup.
// 
void sensorCommonSetupSecurity ( void )
{
    //static boolean securityInitStatus = FALSE;
   boolean securityInitStatus = FALSE;  
  
    if (!securityInitStatus)
    {
#ifdef SINK_APP
        securityInitStatus = trustCenterInit((EmberKeyData *)sys_info.key.link, (EmberKeyData *)sys_info.key.network);    
        if (securityInitStatus)     
            Debug( "\r\nINFO : nwk key set to: %s", sys_info.key.network);
#else
        securityInitStatus = nodeSecurityInit((EmberKeyData *)sys_info.key.link);    
        if (!securityInitStatus)
            Debug( "\r\nERROR: initializing security.", securityInitStatus);
        else
            Debug( "\r\nINFO : link key set to: %s", sys_info.key.link);
#endif        
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// print info about this node
// 
void printNodeInfo ( void ) 
{
    int8u  extendedPanId[EXTENDED_PAN_ID_SIZE];

    emberGetExtendedPanId(extendedPanId);

    if (network_info.type == EMBER_COORDINATOR)
        Debug("sink ");
    else
        Debug("sensor ");
    
    Debug( "eui [");
    printEUI64((EmberEUI64*)emberGetEui64());
    Debug( "] short ID [%2x]\r\n", emberGetNodeId());

    if (emberNetworkState() == EMBER_JOINED_NETWORK) 
    {
        Debug("channel [0x%x], power [0x%x], panId [0x%2x]\r\n", emberGetRadioChannel(), emberGetRadioPower(), emberGetPanId());
        //printExtendedPanId(extendedPanId);
    } 
    else 
    {
        Debug("channel [N/A], power [N/A], panId [N/A] \r\n");
        Debug( "ExtendedPanId [N/A]");
    }
    Debug( ", stack [%2x], app [2.60]\r\n", SOFTWARE_VERSION);
    Debug( "security level [%x]\r\n", EMBER_SECURITY_LEVEL);
}

#if EMBER_SECURITY_LEVEL == 5
/******************************************************************************
// Print the Keys
*******************************************************************************/
void sensorCommonPrintKeys ( void ) 
{
    EmberKeyStruct keyStruct;
    EmberKeyType keyType = EMBER_TRUST_CENTER_LINK_KEY;
    EmberNetworkStatus networkStatus; 
    int8u keyTypesMax = 2;
    int8u i;

    networkStatus = emberNetworkState();
  
    if (networkStatus == EMBER_JOINED_NETWORK || networkStatus == EMBER_JOINED_NETWORK_NO_PARENT) 
    {
        for (i = 0x01; i <= keyTypesMax; i ++) 
        {
            emberGetKey(keyType, &keyStruct);     // Get the key data
      
            if (keyType == EMBER_TRUST_CENTER_LINK_KEY) 
            {
                Debug("EMBER_TRUST_CENTER_LINK_KEY: ");
            } 
            else 
            {
                Debug("EMBER_CURRENT_NETWORK_KEY:  ");
            }

            sensorCommonPrint16ByteKey(emberKeyContents(&keyStruct.key));    // print the key
            sensorPrintKeyOptions(&keyStruct);

            keyType = EMBER_CURRENT_NETWORK_KEY;          // change key type
        }

    // print the link key table - sensor app doesnt use link keys
    // so comment this code out
    /*
	{
	    EmberStatus status;

    Debug( "Link Key Table, size = 0x%x", 
                      EMBER_KEY_TABLE_SIZE);
    for (i=0; i<EMBER_KEY_TABLE_SIZE; i++) {
      status = emberGetKeyTableEntry(i, &keyStruct);
      Debug( "\r\n%x:", i);

      switch (status) {
      case EMBER_KEY_INVALID:
        Debug( "invalid key");
        break;
      case EMBER_INDEX_OUT_OF_RANGE:
        Debug( "index out of range");
        break;
      case EMBER_SUCCESS:
        sensorCommonPrint16ByteKey(emberKeyContents(&keyStruct.key));
        sensorPrintKeyOptions(&keyStruct);
        break;
      case EMBER_LIBRARY_NOT_PRESENT:
        Debug( "link key library not present");
        break;
      default:
        Debug( "unknown status 0x%x", status);
      }
    }
}    */
    } 
    else 
        Debug("\r\nINFO : not a valid network state to query the keys.");
}
#endif //EMBER_SECURITY_LEVEL == 5

// *********************************
// utility for printing EUI64 addresses          
// *********************************
void printEUI64 ( EmberEUI64 * eui ) 
{
    int8u * ptr = (int8u*)eui; 
    for (int8u i = EUI64_SIZE; i > 0x00; i --) 
        DBG("%02x", ptr[i - 0x01]);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void delay ( u16 times )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void delay ( u16 times )
{
    while (times --)
        for (vu16 i = 0x00; i < 0x3ff; i ++) 
            i = i;
}

// *********************************************************************
// the next set of variables are needed to support EM250 bootloader.
#ifdef USE_BOOTLOADER_LIB

EmberNodeId bootloadInProgressChildId;
EmberEUI64  bootloadInProgressEui;
void sendBootloaderLaunchMessage(EmberEUI64 targetEui);
boolean parentLaunchBootload = FALSE;

#endif // USE_BOOTLOADER_LIB


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// 下面是使用Poll/Jit/child的情况
// ++++++++++++++++++++++++++++++++++++++++++++++++++

// bitmasks to tell which children have or have not been sent a
// partcular JIT message - one bitmask per message. This can support
// up to 16 children
int16u jitMaskMulticastHello;

// Message Buffers are allocated for each message type
EmberMessageBuffer jitMessageMulticastHello = EMBER_NULL_MESSAGE_BUFFER;

// all JIT messages in this app use the same APS frame setup except
// for the clusterID. The APS header is setup before it is needed to
// save time. Only the clusterID is modified before a message is sent
EmberApsFrame jitMessageApsFrame;


// Called from appAddJitForAllChildren which is called when the parent
// receives a message that it must turn into a JIT message.
//
// This fills in the Message Buffer for the JIT message type passed in
// if it has not already been filled in. It also updates the data in the
// MessageBuffer to the data passed in.
void createAndStoreJitMessage ( int8u messageType,
                                EmberMessageBuffer* globalJitBuffer,
                                int8u* data,
                                int8u dataLength )
{
    if ((int16u)dataLength  > PACKET_BUFFER_SIZE)    // make sure we don't put in too much data and overrun the packet buffer
        assert(FALSE);

  // if this is the first time we are creating the JIT message then
  // setup the aps header. Also allocate a buffer
  if ((*globalJitBuffer) == EMBER_NULL_MESSAGE_BUFFER)
  {
    Debug( "EVENT: creating JIT msg 0x%x\r\n", 
                      messageType);

    jitMessageApsFrame.sourceEndpoint      = network_info.endpoint;
    jitMessageApsFrame.destinationEndpoint = network_info.endpoint;
    // the cluster ID will change based on the message
    //jitMessageApsFrame.clusterId      = clusterId;
    jitMessageApsFrame.profileId           = sys_info.channel.profile;
    jitMessageApsFrame.options             = EMBER_APS_OPTION_POLL_RESPONSE;

    (*globalJitBuffer) = emberFillLinkedBuffers(data,
                                             dataLength);
    if ((*globalJitBuffer) == EMBER_NULL_MESSAGE_BUFFER) {
      Debug( "ERROR: no buffers to construct JIT message!\r\n");
      return;
    }
  }

}

// Called when the parent receives a message that it must turn into
// a JIT message.
//
// this sets the JIT message flag for all children, and sets
// the bitmask that the application uses to make sure it doesn't
// send the same JIT message to a child more than once.
void appAddJitForAllChildren(int8u msgType, 
                             int8u* data, 
                             int8u dataLength)
{
  int8u i;
  EmberStatus status;

  if (msgType == ICHP_MSG_HELLO) {
    jitMaskMulticastHello = 0xFFFF;
    createAndStoreJitMessage(ICHP_MSG_HELLO,
                             &jitMessageMulticastHello,
                             data, dataLength);
  }

  // set the message flag for all children
  Debug( "JIT: setting flag, for all children\r\n");

  for (i=0; i<emberMaxChildCount(); i++)
  {
    EmberNodeId childId = emberChildId(i);
    if (childId != EMBER_NULL_NODE_ID) {
      status = emberSetMessageFlag(childId);
      if (status != EMBER_SUCCESS) {
        Debug(
                          "ERROR: set flag, child %2x, status %x\r\n",
                          emberChildId(i), status);
      }
    }
  }
}

// This is called from emberPollHandler when the polling node has it's
// message flag set.
//
// This sends the JIT message to the child using an APS message. The
// child should be awake because it has just polled and the parent should
// have sent a mac ack with frame pending set to true
void appSendJitToChild(EmberNodeId childId)
{
  EmberStatus status;
  // used for debugging (see below)
  //   boolean sentMcastHello = FALSE;
  //   EmberStatus mcastHelloStatus;

  // get the child index from child ID. The index is used by the
  // application to determine if the child has already polled for
  // that message
  int8u childIndex = emberChildIndex(childId);

  // This application only sends 1 message per poll. If the sleepy child gets
  // data from a poll it will nap and poll again (not hibernate) until it
  // doesn't get any data from a poll. In order to send more than one message
  // in response to a single poll, the parent needs to be sure to clear the
  // message flag (emberClearMessageFlag) after the first message has been
  // put into the message queue. If the flag is cleared before that, then
  // the first message will have framePending=false and the child will not
  // be awake to hear the second message. The best way to handle this is
  // to keep track of the number of messages sent to each child, decrement
  // this for each emberMessageSentHandler callback, and then clear the message
  // flag (in emberMessageSentHandler) when the number of outstanding messages go
  // to zero. This application doesn't do this and instead sends one
  // JIT message at a time. If there is a second JIT message on the parent
  // it will be picked up on the next nap cycle, set for 1 second.

  // check if this child has already polled for the message
  if (BIT(childIndex) & jitMaskMulticastHello) {

    jitMessageApsFrame.clusterId = ICHP_MSG_HELLO;
    /*mcastHelloStatus = */ emberSendUnicast(EMBER_OUTGOING_DIRECT,
                                             childId,
                                             &jitMessageApsFrame,
                                             jitMessageMulticastHello);
    // sentMcastHello = TRUE;
    // clear the bit - invert the number that has the child index'th bit set (xor)
    // and then bitwise AND to clear from the jitMask
    jitMaskMulticastHello = jitMaskMulticastHello & (BIT(childIndex) ^ 0xFFFF);
  }

  // The next section of code is used for debugging and is commented out.
  // Serial characters are dropped when there are a bunch of these messages
  // going out at the same time so it's not very useful with more than one
  // or two children, but can be helpful when initially testing modifications
  // to this code (for instance, adding another JIT type)
  // 
  //  if (sentMcastHello) {
  //    Debug( "TX JIT [hello]:%2x(%x)\r\n", 
  //                      childId, mcastHelloStatus);
  //  }


  // clear the message flag if there are no msgs waiting for this node
  if (!(BIT(childIndex) & jitMaskMulticastHello))
  {
    status = emberClearMessageFlag(childId);
    if (status != EMBER_SUCCESS) {
      Debug( "ERROR: %x, clear flag %2x\r\n", 
                        status, childId);
    }
    // The next three lines are commented out but may be useful when
    // modifying and then debugging this code.
    //else {
    //  Debug( "JIT: clear flag %2x\r\n", childId);
    //}
  }
}

// This is called by the stack when a device polls. When
// transmitExpected is true, this means the message flag is set for
// this child.
//
void emberPollHandler(EmberNodeId childId, boolean transmitExpected)
{
  // functions called from within this should not contain actions that
  // could take a long time. The messages sent in response to the poll
  // have to be within 15ms. Doing a in here, for
  // example would cause the sleepy end device to go back to sleep
  // before the parent was able to send any messages at all.

  if (transmitExpected)
  {
	  // Check if we need to send the initial bootload launch message.
    // In addition don't call appSendJitToChild if we have bootload process
    // pending since it could confuse the bootload by sending other JIT message 
    // and also could clear the message flag
    #ifdef USE_BOOTLOADER_LIB
      // Check if the child that polls is the node to be bootloaded.
      if(bootloadInProgressChildId == childId) {
        if (parentLaunchBootload) {
          sendBootloaderLaunchMessage(bootloadInProgressEui);
          parentLaunchBootload = FALSE;
          // Do not try to send any other JIT message when starting 
          // bootload process.
          return;
        } else if (blState == BOOTLOAD_STATE_DELAY_BEFORE_START) {
          // We have received the authentication challenge.
          bootloadUtilSendAuthResponse(bootloadInProgressEui);
          emberClearMessageFlag(childId);
          // Do not try to send any other JIT message when starting 
          // bootload process.
          return;
        }
      }
      if(!IS_BOOTLOADING) {
        // take care of all the sending of JIT messages to the child
        appSendJitToChild(childId);	
      }
    #else
      appSendJitToChild(childId);	
    #endif
  }
}


// this function is called from jitMessageStatus to print the contents 
// of a JIT buffer. Useful in making sure the correct data is being
// copied into the JIT buffer.
void jitMessageStatusPrintMessageData(EmberMessageBuffer bufferToPrint)
{
  int8u i;
  if (bufferToPrint != EMBER_NULL_MESSAGE_BUFFER) {
    Debug( "   data: ");
    for (i=0; i< emberMessageBufferLength(bufferToPrint); i++) {
      Debug( "%x ",
                        emberGetLinkedBuffersByte(bufferToPrint, i));
    }
    Debug( "\r\n");
  }

}

// This is called to print the status of the JIT storage on a parent node
// (sink or line powered sensor)
void jitMessageStatus(void)
{
  halResetWatchdog();

  // bitmasks
  Debug( "bitmask for Multicast_Hello message: %2x\r\n",
                    jitMaskMulticastHello);

  // multicast hello packet buffer
  Debug( "packet buffer for Multicast_Hello: %x\r\n",
                    jitMessageMulticastHello);
  jitMessageStatusPrintMessageData(jitMessageMulticastHello);
}


// this is called to print the child table
void printChildTable(void)
{
  int8u i;
  EmberStatus status;
  EmberEUI64 eui;
  EmberNodeType type;

  for (i=0; i<emberMaxChildCount(); i++)
  {
    status = emberGetChildData(i, eui, &type);
    Debug( "%x:", i);
    if (status == EMBER_SUCCESS)
    {
      printEUI64((EmberEUI64*)eui);
      switch (type){
      case EMBER_SLEEPY_END_DEVICE:
        Debug( " sleepy");
        break;
      case EMBER_MOBILE_END_DEVICE:
        Debug( " mobile");
        break;
      case EMBER_END_DEVICE:
        Debug( " non-sleepy");
        break;
      default:
        Debug( " ??????");
        break;
      }
    }
    Debug( "\r\n");
  }
}

void emberChildJoinHandler(int8u index, boolean joining)
{
  EmberStatus status;
  EmberEUI64 eui;
  EmberNodeType type;

  // if the node is joining...
  if (joining == TRUE)
  {
    status = emberGetChildData(index, eui, &type);
    // ...and the type is mobile...
    if ((status == EMBER_SUCCESS) &&
        (type == EMBER_MOBILE_END_DEVICE))
    {
      // ...set the message flag so it gets any messages right
      // away, since the node will not be in the child table
      // (and not have it's message flag set) whenever it
      // hibernates
      emberSetMessageFlag(emberChildId(index));
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberSwitchNetworkKeyHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        :// this is called when the stack status changes
//*------------------------------------------------*/
void emberSwitchNetworkKeyHandler ( u8 sequenceNumber )
{
    Debug( "\r\nEVENT: network key switched, new seq %x", sequenceNumber);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++
// 下面是使用Bootloader_lib的情况
// ++++++++++++++++++++++++++++++++++++++++++++++++++

// **************************************************
// utility calls to support the EM250 Bootloader
//
// **************************************************
#ifdef USE_BOOTLOADER_LIB

// this is called by the sensor or sink -- applications that act
// as parents. This is called with the EUI of the child that the
// parents wants to bootload. Since the sleepy device may not be
// awake (likely not), this function sets the message flag of the
// child and remembers which child needs to be bootloaded. When the
// child wakes up and polls, the initial bootloader launch message 
// is sent -- see emberPollHandler
void bootloadMySleepyChild(EmberEUI64 targetEui)
{
  int8u index;
  
  // message for the user
  Debug( "INFO: attempt child bootload, eui ");
  printEUI64((EmberEUI64*)targetEui);
  Debug( "\r\n");
 
  // make sure the EUI is a child of this parent
  if (!(isMyChild(targetEui, &index))) {
	return;
  }
  
  // set the message flag for the child
  emberSetMessageFlag(emberChildId(index));

  // remember the EUI and shortId for the child we are bootloading
  bootloadInProgressChildId = emberChildId(index);
  MEMCOPY(bootloadInProgressEui, targetEui, EUI64_SIZE);
  
  // the bootload util library tells us if a bootload is in progres
  // with the IS_BOOTLOADING macro. We need to set a flag to send the
  // bootload launch message. Only want to do this once.
  parentLaunchBootload = TRUE;

  // print a message explaining that passthru is initiating 
  // and there is a few seconds before the xmodem ('C' characters
  // printing) starts.
  Debug( 
                   "If a bootload response is seen, xmodem ");
  Debug( "will start in 30 seconds\r\n");
  Debug( 
                    "If xmodem does not start, the bootload failed\r\n");
}


// called by a sensor or sink when bootloading a neighbor device is
// desired. This function just prints a bunch of information for the
// user's benefit and calls a function that sends the initial bootload
// launch message.
void bootloadMyNeighborRouter(EmberEUI64 targetEui)
{ 
  // message for the user
  Debug( "INFO: attempt neighbor bootload, eui ");	
  printEUI64((EmberEUI64*)targetEui);
  Debug( "\r\n");

  // print a message explaining that passthru is initiating 
  // and there is a few seconds before the xmodem ('C' characters
  // printing) starts.
  Debug( 
                   "If a bootload response is seen, xmodem ");
  Debug( "will start in 10 seconds\r\n");
  Debug( 
                    "If xmodem does not start, the bootload failed\r\n");
  
  sendBootloaderLaunchMessage(targetEui);
}


// does the sending of the bootloader launch message. This is really
// just setting up the parameters and making a call to bootloadUtilSendRequest
void sendBootloaderLaunchMessage(EmberEUI64 targetEui)
{
  // encryptKey has to match the token TOKEN_MFG_BOOTLOADER_AES_KEY 
  // on the target device or the bootload will not start
  int8u encryptKey[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

  // mfgId and hardwareTag have to match what the target device is
  // checking for in bootloadUtilRequestHandler. These values are meant
  // to be stored in the manufacturing tokens TOKEN_MFG_MANUF_ID (mfgId)
  // and TOKEN_MFG_BOARD_NAME (hardwareTag).
  int16u mfgId = 0xE250;
  int8u hardwareTag[8] = {'D', 'E', 'V', ' ', '0', '4', '5', '5'};
  
  bootloadUtilSendRequest(targetEui,
                          mfgId,
                          hardwareTag,
                          encryptKey,
                          BOOTLOAD_MODE_PASSTHRU); 	
}

// determines if the EUI passed in is in this device's child table
// return of true means the EUI is in the child table
// return of false means the EUI is not in the child table
// if the EUI is in the child table, then childIndex is set to the
// index in the child table of the EUI.
boolean isMyChild(EmberEUI64 candidateEui, int8u* childIndex)
{
  int8u i;
  EmberEUI64 childEui;
  EmberNodeType type;
  EmberStatus status;
  
  // go through whole child table  
  for (i=0; i<emberMaxChildCount(); i++) {
    status = emberGetChildData(i, childEui, &type);
	if ((status == EMBER_SUCCESS) && 
		(MEMCOMPARE(candidateEui, childEui, EUI64_SIZE)==0)) {
      // we found a match
      (*childIndex) = i;
      return TRUE;
    }
  }
  // didn't match any entry in my child table
  return FALSE;	
}

// no check for now. If an attempt is made to bootload a device 
// that is not a neighbor the initial launch will fail.
boolean isMyNeighbor(EmberEUI64 eui)
{
    return TRUE;		
}

////////////////////////////////////////////////////////////////////////////////////
//
#endif //USE_BOOTLOADER_LIB


