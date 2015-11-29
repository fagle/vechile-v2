// *******************************************************************
// * bootload-utils.c
// * 
// * Utilities used for bootloading.
// * See bootload-utils.h for more complete description
// *
// * Copyright 2006 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include PLATFORM_HEADER     // Micro and compiler specific typedefs and macros

// Ember stack and related utilities
#include "stack/include/ember.h"         // Main stack definitions

// HAL
#include "hal/hal.h" 

// Application Bootloader eeprom APIs 
#ifdef XAP2B
#include "hal/micro/xap2b/em250/eeprom.h"   // EEPROM driver
#endif
#ifdef CORTEXM3
#include "hal/micro/cortexm3/bootloader/eeprom.h"
#endif

// Application utilities
#include "app/util/serial/serial.h"
#include "app/ha/zcl-util-common-include.h"
#include "app/ha/zcl-util-report-table.h"
#include "app/util/bootload/app-bootload-utils-internal.h"
#include "app/util/bootload/app-bootload-utils.h"

#ifdef  USE_APP_BOOTLOADER_LIB

// ***********************************************
// NOTE: applications that use the bootload utilities need to 
// define USE_APP_BOOTLOADER_LIB
// within their CONFIGURATION_HEADER
// ***********************************************

// Initialize bootload state to normal (not currently participate in 
// bootloading)
bootloadState blState = BOOTLOAD_STATE_NORMAL;

// bootload (xmodem) protocols variables
int8u dataBlock[XMODEM_BLOCK_SIZE];
int16u expectedBlockNumber = 1;   // segment to write to download space

// We read data 128 bytes at a time from the EEPROM, however, 
// we can only send it in BOOTLOAD_PAYLOAD_SIZE (64 bytes) at a time
// over the air (OTA).  Hence, data read (from EEPROM) is divided into two
// over-the-air messages. 
int16u currentOTABlock = 0; 

// Short Id of the destination node.  It can either contain the id of the 
// source or target node depending on whether the node is a bootloader or
// being bootloaded.
EmberNodeId destination;

// To keep track of how much time we should wait.  A simple timeout mechanism
int16u actionTimer = TIMEOUT_NONE;

// Port for serial prints and bootloading
int8u appSerial = 0;
int8u bootloadSerial = 1;

// APS frame used
EmberApsFrame apsStruct;

// Node information variable
NodeInfo myNodeInfo;

// Just In Time message parameters
boolean useJIT = FALSE;
EmberNodeId originalQuerySender;

// application global variables
bootloadString globalArray;
header globalHeader;
command globalCommand;
readAttributesCmd globalReadCmd;
int16u globalIds;
int8u globalSeq = 0;
int8u blStatus = APPBL_READY;
          
// Forward declaration of functions used internally by bootload 
// utilities library.  These functions are defined by bootload utility
// library.
static void xmodemSendReady(void);
static int8u xmodemReadSerialByte(int8u timeout, int8u *byte);
static int8u xmodemReceiveBlock(void);
static boolean bootloadSendMessage(int8u cmdId);
static EmberStatus sendRadioMessage(EmberMessageBuffer payload);
static void getReadResponseData(int8u attributeId, 
                                readAttributeStatusRecord *record);
static void displayResponseData(EmberNodeId sender, EmberMessageBuffer buffer,
                                readAttributesResponseCmd *response);
static boolean readEEPROMData(void);
static boolean writeEEPROMData(int16u page);
static void printImageInfo(int8u* info);
static int16u validateEEPROMImage(void);
static void setAllChildrenMessageFlag(void);
// Command parser and builder utility function prototypes
static EmberMessageBuffer buildCompleteCmd(boolean response,
                                           int8u *transactionSequenceNumber, 
                                           int8u commandId,
                                           void *fields);
static int8u reallyBuildCmd(command *cmd, header *h, void *fields);
static void appendHeader(command *cmd, header *h);
static void appendReadAttributePayload(command *cmd, readAttributesCmd* fields);
static void appendReadAttributeResponsePayload(command *cmd, 
                                               readAttributesResponseCmd* fields);
static void appendReportAttributePayload(command *cmd, 
                                         reportAttributesCmd* fields);
static int8u append8BitData(command *cmd, int8u data);
static int8u append16BitData(command *cmd, int16u data);
static int8u append32BitData(command *cmd, int32u data);
static int8u appendBytes(command *cmd, int8u bytesLen, int8u *bytes);
static int8u appendString(command *cmd, int8u *data);
static int8u appendAttribute(command *cmd, int8u attributeType, 
                             void *attributeData);
static int8u parseCompleteCmd(EmberMessageBuffer buffer,
                              int8u *transactionSequenceNumber, 
                              void *fields);
static int8u reallyParseCmd(command *cmd, header *h, void *fields);
static void getHeader(command *cmd, header *h);
static void getReadAttributePayload(command *cmd, 
                                    int8u length, 
                                    readAttributesCmd *fields);
static void getReadAttributeResponsePayload(command *cmd, 
                                             readAttributesResponseCmd *fields);
static void getReportAttributePayload(command *cmd, reportAttributesCmd *fields);
static void getAttributeLocation(command *cmd, int8u type, 
                                 void **attributeLocation);
static int8u getAttributeFromLocation(EmberMessageBuffer buffer, 
                                      void *attributeLocation, 
                                      int8u attributeType, 
                                      void *attributeData);
static int8u attributeTypeLength(int8u type, int8u *datap);
static int8u get8BitData(command *cmd, int8u *data);
static int8u get16BitData(command *cmd, int16u *data);
static int8u get32BitData(command *cmd, int32u *data);
static int8u getBytes(command *cmd, int8u bytesLen, int8u *bytes);
static int8u getString(command *cmd, int8u maxStringLen, int8u *data);
static int8u getAttribute(command *cmd, int8u attributeType, void *attributeData);

// ****************************************************************
// Public functions that can be called from the application to utilize
// bootload features.
// ****************************************************************

// -------------------------------------------------------------------------
// Initialize serial port
void appBootloadUtilInit(int8u appPort, int8u bootloadPort, int16u appProfileId)
{ 
  
  // Initialize serial ports
  appSerial = appPort;
  bootloadSerial = bootloadPort;
  emberSerialInit(bootloadSerial, BOOTLOAD_BAUD_RATE, PARITY_NONE, 1);
  
  // Initialize aps frame
  apsStruct.sourceEndpoint = APP_BOOTLOAD_ENDPOINT;
  apsStruct.destinationEndpoint = APP_BOOTLOAD_ENDPOINT;
  apsStruct.clusterId      = ZCL_APP_BOOTLOAD_CLUSTER_ID;
  apsStruct.profileId      = appProfileId;
  apsStruct.options        = (EMBER_APS_OPTION_RETRY | 
                              EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY);
  
  // Initialize attribute table and write attributes.
  zclUtilAttributeTableInit();
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_EUI64_ATTRIBUTE_ID, 
                        emberGetEui64(),
                        ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);
  emberGetNodeType(&(myNodeInfo.type));
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_NODETYPE_ATTRIBUTE_ID, 
                        &(myNodeInfo.type),
                        ZCL_INT8U_ATTRIBUTE_TYPE);
  // Note that short id is not sent out as part of initiate
  // response since the source node can obtain the information by calling
  // emberGetSender().
  myNodeInfo.shortId = emberGetNodeId();  // not in attribute table
  myNodeInfo.version = appBootloadUtilGetAppVersionHandler();
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_APP_VERSION_ATTRIBUTE_ID, 
                        (int8u*)&(myNodeInfo.version),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
  // Initialize the EEPROM image status to invalid until we are told to validate
  // the content by receiving validate command.
  myNodeInfo.imageStatus = APPBL_IMAGE_INVALID; // not in attribute table
  halCommonGetToken(&(myNodeInfo.mfgId), TOKEN_MFG_MANUF_ID);
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_MFGID_ATTRIBUTE_ID, 
                        (int8u*)&(myNodeInfo.mfgId),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
  halCommonGetToken(&(myNodeInfo.hwTag[0]), TOKEN_MFG_BOARD_NAME);
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_HWTAG_ATTRIBUTE_ID, 
                        (int8u*)&(myNodeInfo.hwTag[0]),
                        ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  myNodeInfo.blVersion = halAppBootloaderGetVersion();
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_APPBL_VERSION_ATTRIBUTE_ID, 
                        (int8u*)&(myNodeInfo.blVersion),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
  myNodeInfo.recVersion = halAppBootloaderGetRecoveryVersion();
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_REC_IMAGE_VERSION_ATTRIBUTE_ID, 
                        (int8u*)&(myNodeInfo.recVersion),
                        ZCL_INT16U_ATTRIBUTE_TYPE);
  myNodeInfo.blType = (int8u)halBootloaderGetType();
  zclUtilWriteAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                        ZCL_APP_BL_BOOTLOADER_TYPES_ATTRIBUTE_ID, 
                        &(myNodeInfo.blType),
                        ZCL_INT8U_ATTRIBUTE_TYPE);
  
  // Initialize zcl header. In this application, we use this header only to
  // build cluster specific commands.
  globalHeader.clusterSpecific = TRUE;         
  globalHeader.manufacturerSpecific = TRUE;    
  globalHeader.serverToClient = FALSE;
  globalHeader.manufacturerCode = myNodeInfo.mfgId;
  
  // Initialize zcl command.
  globalCommand.buffer = EMBER_NULL_MESSAGE_BUFFER; 
  globalCommand.locationIndex = 0;  
  globalCommand.success = TRUE;

}

// -------------------------------------------------------------------------
// Send bootload query.  The function is called by the source node to get
// necessary information about remote nodes.  The message is sent using global
// zcl command to essentially read attributes on the remote nodes.
void appBootloadUtilSendQuery(int16u address, boolean isJIT)
{
  // Able to send query message only when we are not in the middle of the
  // bootloading process, otherwise, this will change the bootload state and 
  // cause the ongoing bootloading process to fail.
  if(blState == BOOTLOAD_STATE_NORMAL) {
    destination = address;
    blState = BOOTLOAD_STATE_QUERY;
    useJIT = isJIT;
    if(bootloadSendMessage(APP_BL_QUERY_COMMAND_ID)) {
      actionTimer = TIMEOUT_GENERIC;
    } else {
      blState = BOOTLOAD_STATE_NORMAL;
    }
  }
}

// -------------------------------------------------------------------------
// Send initiate command to the target node to determine whether the node can
// be bootloaded.  The target node will reply back with its decision whether
// it can be bootloaded. 
void appBootloadUtilStartBootload(EmberNodeId target)
{ 
  destination = target;
  if(bootloadSendMessage(APP_BL_INITIATE_COMMAND_ID)) {
    blState = BOOTLOAD_STATE_INITIATE;
    actionTimer = TIMEOUT_GENERIC;
    // initialize over-the-air block number
    currentOTABlock = 0;
  } 
}

// -------------------------------------------------------------------------
// Send validate command to the target node to determine whether the node has
// valid image on the EEPROM.  Normally, the source node sends this command 
// after it has finished bootloaded the target node.  Upon receipt of the 
// command, the target node will validate the image on its EEPROM (via CRC
// check) and reply back with the status.  If the image is valid, it will also
// include image timestamp and info in the response.
void appBootloadUtilValidate(EmberNodeId target)
{
  int16u pages;
  int8u eepromStatus;
  destination = target;
  myNodeInfo.shortId = emberGetNodeId();
  // If it's our own short id, then display the information directly.
  if(destination == myNodeInfo.shortId) {
    // Reads the app image out of EEPROM, calculates the total file CRC to 
    // verify the image is intact.  The function returns number of pages in 
    // a valid image. 0 for invalid image.
    bl_print("NODE:%2x", destination);
    pages = validateEEPROMImage();
    if(pages > 0) {
      bl_print(" IMGSTATUS:VALID");
      eepromStatus = halAppBootloaderGetImageData(&(myNodeInfo.timestamp), 
          myNodeInfo.imageInfo);
      if(eepromStatus == EEPROM_SUCCESS) {
        bl_print(" TSTAMP:%4x\r\n", myNodeInfo.timestamp);
        printImageInfo(myNodeInfo.imageInfo);
      } else {
        bl_print(" ERR:READ_EEPROM_FAILED, status 0x%x\r\n", eepromStatus);
      }
    } else {
      bl_print(" IMGSTATUS:INVALID\r\n");
    }
  } else {
    blState = BOOTLOAD_STATE_VALIDATE;
    if(bootloadSendMessage(APP_BL_VALIDATE_COMMAND_ID)) {
      // Use longer timeout here because it takes the (target) node about 
      // 10 seconds to read the whole EEPROM to validate the image.
      actionTimer = TIMEOUT_VALIDATE;
    } else {
      blState = BOOTLOAD_STATE_NORMAL;
    }
  } // end checking target id
}

// -------------------------------------------------------------------------
// Send update command to the target node to determine whether the node can
// update its flash image using the image on EEPROM.  The target node will 
// reply back with its status whether it can update its image.
void appBootloadUtilUpdate(EmberNodeId target)
{
  destination = target;
  myNodeInfo.shortId = emberGetNodeId();
  // If it's our own short id, then call the update function directly.
  if(destination == myNodeInfo.shortId) {
    halAppBootloaderInstallNewImage();
  } else {
    if(bootloadSendMessage(APP_BL_UPDATE_COMMAND_ID)) {
      blState = BOOTLOAD_STATE_UPDATE;
      actionTimer = TIMEOUT_GENERIC;
    }
  } // end check short id 
}

// -------------------------------------------------------------------------
// Bootload State Machine
void appBootloadUtilTick(void)
{
  static int16u lastBlinkTime = 0;
  static int8u timeToPoll = 0;
  int16u time;
  int8u pollInterval;

  time = halCommonGetInt16uMillisecondTick();

  // The library performs tasks every TIME_INCREMENT.  The value can be 
  // changed to speed up or slow things down.
  if ((int16u)(time - lastBlinkTime) > TIME_INCREMENT /* ms */) {
    lastBlinkTime = time;
        
    if (actionTimer == 0) {
      // when timer expires ...
      switch (blState) {  
        case BOOTLOAD_STATE_SEND_VALIDATE_RESP:
          // Create and send query or validate response message in response to 
          // read attributes command
          bootloadSendMessage(APP_BL_VALIDATE_RESPONSE_COMMAND_ID);
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        case BOOTLOAD_STATE_SEND_QUERY_RESP:
          // Create and send query or validate response message in response to 
          // read attributes command
          bootloadSendMessage(APP_BL_QUERY_RESPONSE_COMMAND_ID);
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        case BOOTLOAD_STATE_SEND_INITIATE_RESP:
          // Create and send initiate response message in response to initiate
          // command
          bootloadSendMessage(APP_BL_INITIATE_RESPONSE_COMMAND_ID);
          blState = BOOTLOAD_STATE_RX_IMAGE;
          // Set the timer to wait for the source node to validate its image
          // before starting sending data.
          actionTimer = TIMEOUT_VALIDATE;
          break;
        case BOOTLOAD_STATE_SEND_UPDATE_RESP:
          // Create and send update response message in response to update
          // command
          bootloadSendMessage(APP_BL_UPDATE_RESPONSE_COMMAND_ID);
          blState = BOOTLOAD_STATE_NORMAL;
          // Let emberTick() run one more time so the update response message
          // can be sent out.
          break;
        case BOOTLOAD_STATE_VALIDATE:
        case BOOTLOAD_STATE_UPDATE:
        case BOOTLOAD_STATE_QUERY:
          if(!useJIT) {
            // Stop receiving query, validate, update response messages.
            bl_print("\r\nQQ event end QQ\r\n");
          } else {
            useJIT = FALSE;
          }
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        case BOOTLOAD_STATE_INITIATE:  
          // Do not get initiate response from the target node
          bl_print("Error: Do not get response from target %2x\r\n", 
                   destination);
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        case BOOTLOAD_STATE_SEND_DATA:
          if(bootloadSendMessage(APP_BL_WRITE_TO_EEPROM_COMMAND_ID)) {
            blState = BOOTLOAD_STATE_WAIT_FOR_DATA_ACK;
            actionTimer = TIMEOUT_IMAGE_SEND;
          } else if(blState == BOOTLOAD_STATE_SEND_COMPLETE) {
            // bootloadSendMessage() is failed because we have no more message
            // to send, hence, we now have to send complete message.
            return;
          } else {
            // if we failed with other causes, then we stop the bootload
            // process.
            blState = BOOTLOAD_STATE_NORMAL;
          }
          break;
        case BOOTLOAD_STATE_SEND_COMPLETE:
          if(bootloadSendMessage(APP_BL_REPORT_ATTRIBUTES_COMMAND_ID)) {
            blState = BOOTLOAD_STATE_WAIT_FOR_COMPLETE_ACK;
            actionTimer = TIMEOUT_IMAGE_SEND;
          } else {
            blState = BOOTLOAD_STATE_NORMAL;
          }
          break;
        case BOOTLOAD_STATE_WAIT_FOR_COMPLETE_ACK:
        case BOOTLOAD_STATE_WAIT_FOR_DATA_ACK:
          // If the source node doesn't get the ack from the target node in
          // respond to its "complete" packet, it will still consider the 
          // bootloading to be completed and ask the user to issue validate
          // command to the target node to verify that the application is 
          // loaded successfully.  
          if(blState == BOOTLOAD_STATE_WAIT_FOR_COMPLETE_ACK) {
            bl_print("End of bootload signal sent to target node\r\n");
            bl_print("Please validate image on target node\r\n");
          } else {
            bl_print("Error: no ack for data block 0x%2x\r\n", currentOTABlock);
          }
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        case BOOTLOAD_STATE_RX_IMAGE:
          bl_print("Error: too long since last heard from source %2x\r\n",
                    destination);
          blStatus = APPBL_TIMEOUT_SRC;
        case BOOTLOAD_STATE_REPORT_ERROR:
          // Create and send error message in response to data pkt received
          // failure.
          bootloadSendMessage(APP_BL_REPORT_ATTRIBUTES_COMMAND_ID);
          blState = BOOTLOAD_STATE_NORMAL;
          break;
        default:  // BOOTlOAD_STATE_NORMAL
          // Update with new image (on EEPROM)
          if(blStatus == APPBL_UPDATE) {
            halAppBootloaderInstallNewImage();
          }
          break;
      }
    } else {
      // decrement timer
      actionTimer = actionTimer - 1;
    }
    // Handle polling for sleepy node (which includes mobile node) and end 
    // device.  Note that for sleepy nodes, we use two different poll rates 
    // depending on whether the node is involved in the bootloading
    // process or not.  However, for end device, we use the longer poll rate.
    // Polling is needed for end device so that parent will not time it out
    // from the child table.  The node does not need to poll to receive 
    // messages.
    emberGetNodeType(&(myNodeInfo.type));
    if(myNodeInfo.type >= EMBER_END_DEVICE) {
      if(myNodeInfo.type >= EMBER_SLEEPY_END_DEVICE) {
        if(blState == BOOTLOAD_STATE_RX_IMAGE) {
          pollInterval = BL_POLL_INTERVAL;  // 50 ms
        } else {
          pollInterval = POLL_INTERVAL; // 1 second
        }
      } else {
        pollInterval = POLL_INTERVAL; // 1 second
      }
      if(++timeToPoll == pollInterval) {
        emberPollForData();
        timeToPoll = 0;
      }
    } // end check node type
  } //end check elasped time
}

// ****************************************************************
// Handlers and function callback
// ****************************************************************

void appBootloadUtilIncomingMessageHandler(EmberMessageBuffer message)
{
  boolean parseFlag = TRUE;
  PGM_P msg;
  int8u cmdId = commandId(message);
  
  if(zclFrameType(message) == ZCL_PROFILE_WIDE_COMMAND) {
    bl_print("APP BL cluster does not support profile wide command, 0x%x\r\n",
      cmdId);
    return;
  } else {
    // Only process the incoming message when we are in certain states.
    if((blState == BOOTLOAD_STATE_NORMAL) ||
       (blState == BOOTLOAD_STATE_INITIATE) ||
       (blState == BOOTLOAD_STATE_UPDATE) ||
       (blState == BOOTLOAD_STATE_RX_IMAGE) ||
       (blState == BOOTLOAD_STATE_QUERY) ||
       (blState == BOOTLOAD_STATE_VALIDATE)) {
      // Set status to success at the beginning, then check the status
      // later for failure
      globalCommand.success = TRUE;
      // Set the command buffer to the message buffer received in order for it
      // to be parsed.
      globalCommand.buffer = message;
    } else {
      return;
    }
    
    switch(cmdId) {
      case APP_BL_QUERY_COMMAND_ID:
      case APP_BL_VALIDATE_COMMAND_ID:
        msg = "read";
        // We do not reply to the read command when we are in the middle
        // of bootloading another node.
        if(blState == BOOTLOAD_STATE_NORMAL) {
          if(parseReadAttributesCmd(message, &globalSeq, &globalReadCmd)) {
            // get the sender (short) address
            destination = emberGetSender();
            // We send the response right away when appBootloadUtilTick is 
            // called.  We determine which response to send by looking at the
            // number of attributes being read.
            if(cmdId == APP_BL_QUERY_COMMAND_ID) {
              blState = BOOTLOAD_STATE_SEND_QUERY_RESP;
              // Obtained the address of the sender.  Since sleepy node can 
              // not hear the query (broadcast) message directly from the 
              // sender.  It has to obtain the address embedded in the message.
              // For non sleepy node, the address embedded is the same as the
              // value obtained from calling emberGetSender() since they can
              // hear the query message directly.
              {
                int8u length = emberMessageBufferLength(message);
                destination = (int16u)HIGH_LOW_TO_INT(
                  emberGetLinkedBuffersByte(message, length-1),
                  emberGetLinkedBuffersByte(message, length-2));
              }
              // Set message flag for all children to send the query as 
              // unicast the next time they poll.  Also store the sender's
              // short id to be included in the query message to the child.
              emberGetNodeType(&(myNodeInfo.type));
              if((myNodeInfo.type == EMBER_COORDINATOR) || 
                 (myNodeInfo.type == EMBER_ROUTER)) {
                setAllChildrenMessageFlag();
                originalQuerySender = destination;
              }
            } else if(cmdId == APP_BL_VALIDATE_COMMAND_ID) {
              blState = BOOTLOAD_STATE_SEND_VALIDATE_RESP;
            } 
            actionTimer = TIMEOUT_NONE;
          } else {
            parseFlag = FALSE;
          }
        }
        break;

      case APP_BL_QUERY_RESPONSE_COMMAND_ID:
        {
          readAttributeStatusRecord record[MAX_ATTRIBUTE_QUERY];
          readAttributesResponseCmd response;
          msg = "query resp";
                    
          response.recordsCount = MAX_ATTRIBUTE_QUERY;
          response.statusRecords = record;
          // We keep displaying the responses received as long as we receive 
          // them within the timeout value.
          if(blState == BOOTLOAD_STATE_QUERY) {
            if(parseReadAttributesResponseCmd(message, NULL, &response)) {
              displayResponseData(emberGetSender(), message, &response);
            } else {
              parseFlag = FALSE;
            }
          } // end checking state
        }
        break;

      case APP_BL_VALIDATE_RESPONSE_COMMAND_ID:
        {
          readAttributeStatusRecord record[MAX_ATTRIBUTE_VALIDATE];
          readAttributesResponseCmd response;
          msg = "validate resp";
                    
          response.recordsCount = MAX_ATTRIBUTE_VALIDATE;
          response.statusRecords = record;
          // We keep displaying the responses received as long as we receive 
          // them within the timeout value.
          if(blState == BOOTLOAD_STATE_VALIDATE) {
            if(parseReadAttributesResponseCmd(message, NULL, &response)) {
              displayResponseData(emberGetSender(), message, &response);
            } else {
              parseFlag = FALSE;
            }
          } // end checking state
        }
        break;

      case APP_BL_REPORT_ATTRIBUTES_COMMAND_ID:
        {
          reportAttributeRecord record;
          reportAttributesCmd report;
          int8u status;
          msg = "report";
          
          report.recordsCount = MAX_ATTRIBUTE_REPORT;
          report.reportRecords = &record;
          // We display the report message received.
          if(parseReportAttributesCmd(message, &report)) {
            if(record.attributeId == ZCL_APP_BL_STATUS_ATTRIBUTE_ID) {
              // Get the reported status
              getAttributeFromLocation(message, record.attributeLocation, 
                                          ZCL_INT8U_ATTRIBUTE_TYPE, &status);
              bl_print("[Rx Report] %s from node %2x\r\n",
                (status == APPBL_READY? "Node is ready":
                (status == APPBL_WRITE_EEPROM_FAILED? "Failed write to EEPROM":
                (status == APPBL_PARSE_DATA_FAILED? "Message parse failed":
                (status == APPBL_IMAGE_VALID? "Image is valid":
                (status == APPBL_UPDATE? "Node will update image":
                (status == APPBL_COMPLETE? "Bootload Complete!": 
                (status == APPBL_TIMEOUT_SRC? "Bootload Timeout!": 
                "Unknown"))))))), emberGetSender());
              if((status == APPBL_WRITE_EEPROM_FAILED) ||
                 (status == APPBL_PARSE_DATA_FAILED) ||
                 (status == APPBL_TIMEOUT_SRC)) {
                bl_print("Bootload Failed!\r\n");
                blState = BOOTLOAD_STATE_NORMAL;
              } else if(status == APPBL_COMPLETE) {
                // stop waiting for source node.
                actionTimer = TIMEOUT_NONE;
                blState = BOOTLOAD_STATE_NORMAL;
              }
            }
          } else {
            parseFlag = FALSE;
          }
        }
        break;
      
      case APP_BL_INITIATE_COMMAND_ID:
        msg = "initiate";
        // parse cluster specific command
        parseCmd(&globalCommand, &globalHeader);
        // We do not reply to the initiate command when we are in the middle
        // of bootloading.
        if(blState == BOOTLOAD_STATE_NORMAL) {
          // Since this is clusters specific command, we know that the command
          // only contains one attribute id (ZCL_APP_BL_STATUS_ATTRIBUTE_ID)
          get16BitData(&globalCommand, &globalIds);
          if(globalCommand.success) {
            // get the sender (short) address
            destination = emberGetSender();
            // We send the response right away when appBootloadUtilTick is 
            // called.
            blState = BOOTLOAD_STATE_SEND_INITIATE_RESP;
            actionTimer = TIMEOUT_NONE;
          } else {
            parseFlag = FALSE;
          }
        } // end checking state
        break;

      case APP_BL_INITIATE_RESPONSE_COMMAND_ID:
        {
          int8u targetStatus;
          msg = "initiate resp";
          // parse cluster specific command
          parseCmd(&globalCommand, &globalHeader);
          // Only start bootload process if we are still in the initiate
          // state
          if(blState == BOOTLOAD_STATE_INITIATE) {
            // Since this is clusters specific command, we know that the command
            // only contains the node (bootload) status
            get8BitData(&globalCommand, &targetStatus);
            if(globalCommand.success) {
              if(targetStatus == APPBL_READY) {
                bl_print("Start bootloading ...\r\n");
                blState = BOOTLOAD_STATE_SEND_DATA;
                actionTimer = TIMEOUT_NONE;
              } else {
                bl_print("Target node cannot be bootloaded.\r\n");
                blState = BOOTLOAD_STATE_NORMAL;
              }
            } else {
              parseFlag = FALSE;
            }
          } // end checking state
        }
        break;

      case APP_BL_WRITE_TO_EEPROM_COMMAND_ID:
        {
          int16u blockAttributeId;
          int16u block;
          msg = "write_to_eeprom";
          // parse cluster specific command
          parseCmd(&globalCommand, &globalHeader);
          if(blState == BOOTLOAD_STATE_RX_IMAGE) {
            // Get block number attribute id in the data pkt
            getAttribute(&globalCommand, 
                          ZCL_INT16U_ATTRIBUTE_TYPE,
                          &blockAttributeId);
            // Get page number in the data pkt
            getAttribute(&globalCommand, 
                          ZCL_INT16U_ATTRIBUTE_TYPE,
                          &block);
            // Get data attribute id in the data pkt
            getAttribute(&globalCommand, 
                          ZCL_INT16U_ATTRIBUTE_TYPE,
                          &blockAttributeId);
            // Get data in the data pkt, first we set the data length
            getAttribute(&globalCommand, 
                          ZCL_OCTET_STRING_ATTRIBUTE_TYPE,
                          &globalArray);
            // Check command status after parsing
            if(globalCommand.success) {
              // Write received bootload data to EEPROM
              if(!writeEEPROMData(block)) {
                {
                  int8u i;
                  bl_print("corrupted message: ");
                  for(i=0; i< emberMessageBufferLength(message); ++i ) {
                    bl_print("%x ", emberGetLinkedBuffersByte(message, i));
                  }
                }
                blStatus = APPBL_WRITE_EEPROM_FAILED;
                blState = BOOTLOAD_STATE_REPORT_ERROR;
                actionTimer = TIMEOUT_NONE;
              } else {
                // Reset timer after receiving a successful message.  We use
                // longer timeout for nodes that sleep and shorter for nodes
                // that do not.
                emberGetNodeType(&(myNodeInfo.type));
                if(myNodeInfo.type >= EMBER_SLEEPY_END_DEVICE) {
                  actionTimer = (int16u)TIMEOUT_LONG;
                } else {
                  actionTimer = TIMEOUT_GENERIC;
                }
              }
            } else {
              parseFlag = FALSE;
              blStatus = APPBL_PARSE_DATA_FAILED;
              blState = BOOTLOAD_STATE_REPORT_ERROR;
              actionTimer = TIMEOUT_NONE;
            }
          } // check state
        }
        break;
      
      case APP_BL_UPDATE_COMMAND_ID:
        msg = "update";
        // parse cluster specific command
        parseCmd(&globalCommand, &globalHeader);
        // We do not reply to the update command when we are in the middle
        // of bootloading.
        if(blState == BOOTLOAD_STATE_NORMAL) {
          // Since this is clusters specific command, we know that the command
          // only contains one attribute id (ZCL_APP_BL_STATUS_ATTRIBUTE_ID)
          get16BitData(&globalCommand, &globalIds);
          if(globalCommand.success) {
            // get the sender (short) address
            destination = emberGetSender();
            // We send the response right away when appBootloadUtilTick is 
            // called.
            blState = BOOTLOAD_STATE_SEND_UPDATE_RESP;
            actionTimer = TIMEOUT_NONE;
          } else {
            parseFlag = FALSE;
          }
        } // end checking state
        break;

      case APP_BL_UPDATE_RESPONSE_COMMAND_ID:
        {
          int8u targetStatus;
          msg = "update resp";
          // parse cluster specific command
          parseCmd(&globalCommand, &globalHeader);
          // Since this is clusters specific command, we know that the command
          // only contains the node (update) status
          get8BitData(&globalCommand, &targetStatus);
          if(globalCommand.success) {
            bl_print("Target node %2x :%s updating ....\r\n", emberGetSender(),
                (targetStatus == APPBL_UPDATE ? "" : "not"));
          } else {
            parseFlag = FALSE;
          }
        }
        break;

      default:
        msg = "unknown command";
        break;
    } // end switch statement
  } // end checking command type
  if(!parseFlag) {
    bl_print("parse %s cmd failed\r\n", msg);
  } 
}
void appBootloadUtilMessageSent(int16u destination, 
                                EmberApsFrame *apsFrame,
                                EmberMessageBuffer message, 
                                EmberStatus status)
{
  if(status == EMBER_SUCCESS) {
    if((blState == BOOTLOAD_STATE_WAIT_FOR_COMPLETE_ACK) &&
       (commandId(message) == APP_BL_REPORT_ATTRIBUTES_COMMAND_ID)) {
      // we are done bootloading.
      bl_print("Bootload Complete!\r\n");
      blState = BOOTLOAD_STATE_NORMAL;
      // expire the timer so we don't wait any longer in case of failure
      actionTimer = TIMEOUT_NONE;
    } else if((blState == BOOTLOAD_STATE_WAIT_FOR_DATA_ACK) &&
       (commandId(message) == APP_BL_WRITE_TO_EEPROM_COMMAND_ID)) {
      // Prepare to send more data
      blState = BOOTLOAD_STATE_SEND_DATA;
      // expire the timer so we don't wait any longer in case of failure
      actionTimer = TIMEOUT_NONE;
      // increment to next data block.
      ++currentOTABlock;
    } else if((blState == BOOTLOAD_STATE_QUERY) &&
      ((apsFrame->options & EMBER_APS_OPTION_POLL_RESPONSE) 
        == EMBER_APS_OPTION_POLL_RESPONSE)) {
      // We have successfully sent JIT message to the child.  We will now
      // clear the flag.
      emberClearMessageFlag(destination);
      blState = BOOTLOAD_STATE_NORMAL;
      actionTimer = TIMEOUT_NONE;
      useJIT = FALSE;
    }
  } // end check message sent status 
}

// ****************************************************************
// Utilities Functions
// ****************************************************************

// This function is used as a port of XModem communication.  It reads
// bootload serial port (normally uart port 1) one byte at a time and then
// returns appropriate status.
static int8u xmodemReadSerialByte(int8u timeout, int8u *byte)
{
  int16u time = halCommonGetInt16uMillisecondTick();

  while (timeout > 0) {
    halResetWatchdog();
    emberSerialBufferTick();
    if (emberSerialReadAvailable(bootloadSerial) > 0) {
      if(emberSerialReadByte(bootloadSerial, byte) == EMBER_SUCCESS) {
        return SERIAL_DATA_OK;
      }
    } else if ((halCommonGetInt16uMillisecondTick() - time) > 100) {
      timeout -= 1;
    }
  }

  return NO_SERIAL_DATA;
}

// This function is used to read bootload serial port one block at a time.
// XModem protocol specifies its data block length to be 128 bytes plus 
// 2 bytes crc and appropriate header bytes.  The function validates the header
// and the crc and return appropriate value.
static int8u xmodemReceiveBlock(void) {
  int8u header=0, blockNumber=0, blockCheck=0, crcLow=0, crcHigh=0;
  int8u ii, temp;
  int16u crc = 0x0000;
  int8u status = STATUS_NAK;
  
  if ((xmodemReadSerialByte(SERIAL_TIMEOUT_3S, &header) == NO_SERIAL_DATA) 
      || (header == XMODEM_CANCEL)
      || (header == XMODEM_CC)) 
    return STATUS_RESTART;
    
  if ((header == XMODEM_EOT) && 
      (xmodemReadSerialByte(SERIAL_TIMEOUT_3S, &temp) == NO_SERIAL_DATA)) 
    return STATUS_DONE;
      
  if ((header != XMODEM_SOH)
    || (xmodemReadSerialByte(SERIAL_TIMEOUT_1S, &blockNumber) != SERIAL_DATA_OK)
    || (xmodemReadSerialByte(SERIAL_TIMEOUT_1S, &blockCheck)  != SERIAL_DATA_OK)) {
    emberSerialFlushRx(bootloadSerial);
    return STATUS_NAK;
  }

  if (blockNumber + blockCheck == 0xFF) {
    if (blockNumber == LOW_BYTE(expectedBlockNumber)) {
      status = STATUS_OK;
    } else if (blockNumber == LOW_BYTE(expectedBlockNumber - 1)) {
      // Sender probably missed our ack so ack again
      return STATUS_DUPLICATE;  
    }
  } 

  for (ii = 0; ii < XMODEM_BLOCK_SIZE; ii++) {
    if (xmodemReadSerialByte(SERIAL_TIMEOUT_1S, &dataBlock[ii]) != SERIAL_DATA_OK) {
      return STATUS_NAK;
    }
    crc = halCommonCrc16(dataBlock[ii], crc);
  }

  if ((xmodemReadSerialByte(SERIAL_TIMEOUT_1S, &crcHigh) != SERIAL_DATA_OK)
      || (xmodemReadSerialByte(SERIAL_TIMEOUT_1S, &crcLow) != SERIAL_DATA_OK) 
      || (crc != HIGH_LOW_TO_INT(crcHigh, crcLow))) {
    return STATUS_NAK;
  }
  return status;
}

// The function sends XModem ready character 'C' as stated by XModem protocol
// as a signal to the host (PC) to start transmit XModem data.
static void xmodemSendReady(void)
{
  emberSerialFlushRx(bootloadSerial);
  emberSerialWriteByte(bootloadSerial, XMODEM_READY);
}

// The function is called from the sample application.  The function
// signals the PC to transmit data and it reads out the data one XModem block
// at a time.  If all data bytes are read fine and the crc is checked out fine,
// it then writes the data to external storage space (EEPROM).  It repeats the 
// process untill all data is transfered successfully.
void appBootloadUtilStartXModem(void)
{
  int8u eepromStatus;
  boolean xmodemStart = FALSE;
  int8u counter=0, i;
  expectedBlockNumber = 1;
  
  for (;;) {
    int8u status;
    halResetWatchdog();
    xmodemSendReady();
    // Determine if the PC has cancelled the session.  We wait ~ 20 s
    // which is a bit higher than xmodem (retry) timeout which is 10 s.
    ++counter;
    if(xmodemStart && counter > 200) {
      bl_print("Error: PC stops sending us data\r\n");
      blState = BOOTLOAD_STATE_NORMAL;
      return;
    }

    do {
      int8u outByte;
      status = xmodemReceiveBlock();
      switch (status) {
        case STATUS_OK:
          // When receive first good packet, that means we have 'officially'
          // started XModem transfer.
          xmodemStart = TRUE;
          counter = 0;
          // Write data (read from xmodem) to EEPROM.  Note that xmode start
          // block number at value one, however, EEPROM starts the page number
          // at value zero.
          eepromStatus = halAppBootloaderWriteDownloadSpace(
                                          (expectedBlockNumber-1), dataBlock);
          if(eepromStatus != EEPROM_SUCCESS) {
            bl_print("Error: cannot write data to EEPROM (0x%x)\r\n", 
                     eepromStatus);
            // Cancel XModem session with the host (PC)
            outByte = XMODEM_CANCEL;
            emberSerialFlushRx(bootloadSerial);
            for(i=0; i<5; ++i) {
              emberSerialWriteByte(bootloadSerial, outByte);
            }
            return;
          }
          expectedBlockNumber++;
          // fall through: a duplicate means the sender missed our ack.
          // Ack again as if everything's okay.
        case STATUS_DUPLICATE:
          outByte = XMODEM_ACK;
          break;
        case STATUS_NAK:
          outByte = XMODEM_NAK;
          break;
        case STATUS_DONE:
          emberSerialWriteByte(bootloadSerial, XMODEM_ACK);
          bl_print("\r\nXModem (to EEPROM) Complete!\r\n");
          return;
        default:
          continue;
      }
      emberSerialFlushRx(bootloadSerial);
      emberSerialWriteByte(bootloadSerial, outByte);
    } while (status != STATUS_RESTART);
  } // end for loop
}

// The function is called to send various message types to the target node
// over the air.  Note that all messages are cluster specific commands with 
// unique command ids.
static boolean bootloadSendMessage(int8u cmdId)
{
  EmberStatus status;
  EmberMessageBuffer buffer = EMBER_NULL_MESSAGE_BUFFER;
  int8u sender[2];
  PGM_P msg = "";
  
  switch(cmdId) {
    case APP_BL_QUERY_COMMAND_ID:
    case APP_BL_VALIDATE_COMMAND_ID:
      {
        // Create read command
        readAttributesCmd readCmd;
        if(blState == BOOTLOAD_STATE_QUERY) {
          // list of attribute ids to read
          int16u ids[MAX_ATTRIBUTE_QUERY] = 
                                   {ZCL_APP_BL_EUI64_ATTRIBUTE_ID,
                                    ZCL_APP_BL_NODETYPE_ATTRIBUTE_ID,
                                    ZCL_APP_BL_APP_VERSION_ATTRIBUTE_ID,
                                    ZCL_APP_BL_MFGID_ATTRIBUTE_ID,
                                    ZCL_APP_BL_HWTAG_ATTRIBUTE_ID,
                                    ZCL_APP_BL_APPBL_VERSION_ATTRIBUTE_ID,
                                    ZCL_APP_BL_REC_IMAGE_VERSION_ATTRIBUTE_ID,
                                    ZCL_APP_BL_BOOTLOADER_TYPES_ATTRIBUTE_ID};
          readCmd.attributeIdsCount = MAX_ATTRIBUTE_QUERY;
          MEMCOPY(readCmd.attributeIds, ids, (MAX_ATTRIBUTE_QUERY*2));
          msg = "query";
          buffer = buildReadAttributesCmd(APP_BL_QUERY_COMMAND_ID, &readCmd);
          
          // For the query message sent from parent to its children using
          // JIT mechanism.  We attach an address of the original sender so
          // that the children can send the query response to the sender
          // directly.  Otherwise, we attach our own short address.
          if(useJIT) {
            sender[0] = LOW_BYTE(originalQuerySender);
            sender[1] = HIGH_BYTE(originalQuerySender);
          } else {
            // For full functional devices (coordinator and router nodes),
            // also set the message for their children.
            emberGetNodeType(&(myNodeInfo.type));
            if((myNodeInfo.type == EMBER_COORDINATOR) || 
               (myNodeInfo.type == EMBER_ROUTER)) {
              setAllChildrenMessageFlag();
            }
            myNodeInfo.shortId = emberGetNodeId();
            sender[0] = LOW_BYTE(myNodeInfo.shortId);
            sender[1] = HIGH_BYTE(myNodeInfo.shortId);
          }
          emberAppendToLinkedBuffers(buffer, sender, 2);
        } else if(blState == BOOTLOAD_STATE_VALIDATE) {
          // list of attribute ids to read
          int16u ids[MAX_ATTRIBUTE_VALIDATE] = 
                                   {ZCL_APP_BL_STATUS_ATTRIBUTE_ID,
                                    ZCL_APP_BL_TIMESTAMP_ATTRIBUTE_ID,
                                    ZCL_APP_BL_IMAGE_INFO_ATTRIBUTE_ID};
          readCmd.attributeIdsCount = MAX_ATTRIBUTE_VALIDATE;
          MEMCOPY(readCmd.attributeIds, ids, (MAX_ATTRIBUTE_VALIDATE*2));
          msg = "validate";
          buffer = buildReadAttributesCmd(APP_BL_VALIDATE_COMMAND_ID, &readCmd);
        } 
      }
      break;

    case APP_BL_QUERY_RESPONSE_COMMAND_ID:
    case APP_BL_VALIDATE_RESPONSE_COMMAND_ID:
      {
        int8u count;
        // Create read response command
        readAttributesResponseCmd readRespCmd;
        // total numbers of attributes to be read
        int8u attributeCounts = globalReadCmd.attributeIdsCount;
        readRespCmd.recordsCount = attributeCounts;
          
        // crafting query response message
        if(blState == BOOTLOAD_STATE_SEND_QUERY_RESP) {
          // Create read record (to be put in the response)
          readAttributeStatusRecord readRecord[MAX_ATTRIBUTE_QUERY];
          msg = "query response";
          // read the saved read command to determine which attributes
          // to include in the message
          for(count = 0; count < attributeCounts; ++count) {
            // look up the specified attribute id and store neccessary
            // information in the read record
            getReadResponseData(globalReadCmd.attributeIds[count],
                                  &readRecord[count]);
          }
          readRespCmd.statusRecords = readRecord;
          buffer = buildReadAttributesResponseCmd(
                                              APP_BL_QUERY_RESPONSE_COMMAND_ID,
                                              &globalSeq, 
                                              &readRespCmd);
        } else { // crafting validate response message
          // Create read record (to be put in the response)
          readAttributeStatusRecord readRecord[MAX_ATTRIBUTE_VALIDATE];
          msg = "validate response";
          // read the saved read command to determine which attributes
          // to include in the message
          for(count = 0; count < attributeCounts; ++count) {
            // look up the specified attribute id and store neccessary
            // information in the read record
            getReadResponseData(globalReadCmd.attributeIds[count],
                                &readRecord[count]);
          }
          readRespCmd.statusRecords = readRecord;
          buffer = buildReadAttributesResponseCmd(
                                           APP_BL_VALIDATE_RESPONSE_COMMAND_ID,
                                           &globalSeq, 
                                           &readRespCmd);
        }
        bl_print("Send %s\r\n", msg);
      }
      break;

    case APP_BL_REPORT_ATTRIBUTES_COMMAND_ID:
      {
        reportAttributeRecord report;
        reportAttributesCmd reportCmd;
        // Set bootload state to complete if we are sending a complete
        // message.
        if(blState == BOOTLOAD_STATE_SEND_COMPLETE) {
          blStatus = APPBL_COMPLETE;
          msg = "complete";
        } else if(blState == BOOTLOAD_STATE_REPORT_ERROR) { 
          // else it means we are report failure status so we use the  
          // blStatus set in appBootloadUtilIncomingMessageHandler() 
          // when we encounter a failure while trying to write data to EEPROM.
          msg = "error";
        }
        // Create report record
        report.attributeId = ZCL_APP_BL_STATUS_ATTRIBUTE_ID;
        report.attributeLocation = (void *)&blStatus;
        // Create report command
        reportCmd.recordsCount = 1;
        reportCmd.reportRecords = &report;
        buffer = buildReportAttributesCmd(&reportCmd);
      }
      break;
  
    case APP_BL_INITIATE_COMMAND_ID:
      { 
        msg = "initiate";
        // Set global command status to success initially, the status
        // can be changed by append functions if failure happens when appending
        // payload to the command.
        globalCommand.success = TRUE;
        // allocate buffer for the command
        globalCommand.buffer = emberAllocateStackBuffer();
        if (globalCommand.buffer == EMBER_NULL_MESSAGE_BUFFER){
          bl_print("Error: no buffer\r\n");
          return FALSE;
        }
        globalHeader.commandId = APP_BL_INITIATE_COMMAND_ID;
        // build cluster specific command
        buildCmd(&globalCommand, &globalHeader);
        // append command field which in this case is the bootload status
        append16BitData(&globalCommand, ZCL_APP_BL_STATUS_ATTRIBUTE_ID);
        // check command status after appending payload
        if (!globalCommand.success) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          bl_print("Error: append %s cmd failed\r\n", msg);
          return FALSE;
        }
        // The message is sent with force route option in order to make
        // sure that the node has the route needed to send subsequent
        // data messages to the target node.
        apsStruct.options = (EMBER_APS_OPTION_RETRY | 
                            EMBER_APS_OPTION_FORCE_ROUTE_DISCOVERY);
        buffer = globalCommand.buffer;
      }
      break;

    case APP_BL_INITIATE_RESPONSE_COMMAND_ID:
      {
        msg = "initiate response";
        // Set global command status to success initially, the status
        // can be changed by append functions if failure happens when appending
        // payload to the command.
        globalCommand.success = TRUE;
        // allocate buffer for the command
        globalCommand.buffer = emberAllocateStackBuffer();
        if (globalCommand.buffer == EMBER_NULL_MESSAGE_BUFFER){
          bl_print("Error: no buffer\r\n");
          return FALSE;
        }
        globalHeader.commandId = APP_BL_INITIATE_RESPONSE_COMMAND_ID;
        // build cluster specific command
        buildCmd(&globalCommand, &globalHeader); 
        // Check to make sure that the attribute to read is bootload status
        if(globalIds == ZCL_APP_BL_STATUS_ATTRIBUTE_ID) {
          // append bootload status.  Note that application can expand this 
          // step to return other status if the application does not want
          // to be bootloaded.
          append8BitData(&globalCommand, APPBL_READY);
        }
        // check command status after appending payload
        if (!globalCommand.success) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          bl_print("Error: append %s cmd failed\r\n", msg);
          return FALSE;
        }
        buffer = globalCommand.buffer;
        bl_print("Send %s\r\n", msg);
      }
      break;

    case APP_BL_WRITE_TO_EEPROM_COMMAND_ID:
      { 
        msg = "data";
        // Set global command status to success initially, the status
        // can be changed by append functions if failure happens when appending
        // payload to the command.
        globalCommand.success = TRUE;
        // allocate buffer for the command
        globalCommand.buffer = emberAllocateStackBuffer();
        if (globalCommand.buffer == EMBER_NULL_MESSAGE_BUFFER){
          bl_print("Error: no buffer\r\n");
          return FALSE;
        }
        globalHeader.commandId = APP_BL_WRITE_TO_EEPROM_COMMAND_ID;
        globalHeader.transactionSequenceNumber = LOW_BYTE(currentOTABlock);
        // build cluster specific command
        buildCmd(&globalCommand, &globalHeader); 
        // append data block number attribute id
        append16BitData(&globalCommand, ZCL_APP_BL_BLOCK_NUMBER_ATTRIBUTE_ID);
        // append data block number value.  The block number value is
        // ultimately an EEPROM page number.  This is conveniently telling
        // the target node which EEPROM page to write to.
        append16BitData(&globalCommand, currentOTABlock);
        // append data string attribute id
        append16BitData(&globalCommand, ZCL_APP_BL_DATA_ATTRIBUTE_ID);
        // read data from EEPROM and copy it into the global array 
        if(!readEEPROMData()) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          return FALSE;
        }
        // append data string
        appendOctetString(&globalCommand, &globalArray);
        // check command status after appending payload
        if (!globalCommand.success) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          bl_print("Error: append %s cmd failed\r\n", msg);
          return FALSE;
        }
        buffer = globalCommand.buffer;
      }
      break;

    case APP_BL_UPDATE_COMMAND_ID:
      { 
        msg = "update";
        // Set global command status to success initially, the status
        // can be changed by append functions if failure happens when appending
        // payload to the command.
        globalCommand.success = TRUE;
        // allocate buffer for the command
        globalCommand.buffer = emberAllocateStackBuffer();
        if (globalCommand.buffer == EMBER_NULL_MESSAGE_BUFFER){
          bl_print("Error: no buffer\r\n");
          return FALSE;
        }
        globalHeader.commandId = APP_BL_UPDATE_COMMAND_ID;
        // build cluster specific command
        buildCmd(&globalCommand, &globalHeader); 
        // append command field which in this case is the bootload status
        append16BitData(&globalCommand, ZCL_APP_BL_STATUS_ATTRIBUTE_ID);
        // check command status after appending payload
        if (!globalCommand.success) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          bl_print("Error: append %s cmd failed\r\n", msg);
          return FALSE;
        }
        buffer = globalCommand.buffer;
      }
      break;

    case APP_BL_UPDATE_RESPONSE_COMMAND_ID:
      {
        msg = "update response";
        // Set global command status to success initially, the status
        // can be changed by append functions if failure happens when appending
        // payload to the command.
        globalCommand.success = TRUE;
        // allocate buffer for the command
        globalCommand.buffer = emberAllocateStackBuffer();
        if (globalCommand.buffer == EMBER_NULL_MESSAGE_BUFFER){
          bl_print("Error: no buffer\r\n");
          return FALSE;
        }
        globalHeader.commandId = APP_BL_UPDATE_RESPONSE_COMMAND_ID;
        // build cluster specific command
        buildCmd(&globalCommand, &globalHeader); 
        // Check to make sure that the attribute to read is bootload status
        if(globalIds == ZCL_APP_BL_STATUS_ATTRIBUTE_ID) {
          // append bootload status.  Note that application can expand this 
          // step to return other status if the application does not want
          // to install new image from EEPROM onto flash.
          blStatus = APPBL_UPDATE;
          append8BitData(&globalCommand, blStatus);
        }
        // check command status after appending payload
        if (!globalCommand.success) {
          emberReleaseMessageBuffer(globalCommand.buffer);
          bl_print("Error: append %s cmd failed\r\n", msg);
          return FALSE;
        }
        buffer = globalCommand.buffer;
        bl_print("Send %s\r\n", msg);
      }
      break;
  } // end switch statement
  
  // check returned message buffer from zclBuildCmd()
  if(buffer == EMBER_NULL_MESSAGE_BUFFER) {
    bl_print("Error: no buffer\r\n");
    return FALSE;
  }
  status = sendRadioMessage(buffer);
  emberReleaseMessageBuffer(buffer);
  if(status != EMBER_SUCCESS) {
    bl_print("Error: send %s failed (0x%x)\r\n", msg, status);
    return FALSE;
  } else {
    return TRUE;
  }
}

// This function sends out the radio packet using provided payload and 
// destination.  The destination parameter also determines whether the 
// message is a unicast or a broadcast message.
static EmberStatus sendRadioMessage(EmberMessageBuffer payload)
{ 
  EmberStatus status;
  
  switch(destination) {
    case EMBER_BROADCAST_ADDRESS:
    case EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS:
    case EMBER_SLEEPY_BROADCAST_ADDRESS:
      status = emberSendBroadcast(destination,
                               &apsStruct,
                               EMBER_MAX_HOPS,
                               payload);
      break;
    default:
      if(useJIT) {
        apsStruct.options = EMBER_APS_OPTION_POLL_RESPONSE;
      } 
      status = emberSendUnicast(EMBER_OUTGOING_DIRECT,
                             destination,
                             &apsStruct,
                             payload);
      break;
  }
  
  // set the aps option back to retry with enable route.
  apsStruct.options = (EMBER_APS_OPTION_RETRY | 
                       EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY);
  return status;
}

// This function is called to retrieve attribute values
static void getReadResponseData(int8u attributeId, 
                                readAttributeStatusRecord *record)
{
  int8u dataType;
  // same value for all attributes
  record->attributeId = attributeId;
  // default to success.
  record->status = ZCL_SUCCESS;
  
  // look up each attribute id
  switch(attributeId) {
    case ZCL_APP_BL_EUI64_ATTRIBUTE_ID:
      record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             myNodeInfo.eui64,
                             EUI64_SIZE,
                             &dataType);
      record->attributeType = dataType;
      record->attributeLocation = (void *)(myNodeInfo.eui64);
      break;
    case ZCL_APP_BL_NODETYPE_ATTRIBUTE_ID:
      {      
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             &(myNodeInfo.type),
                             1,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.type);
      }
      break;
    case ZCL_APP_BL_APP_VERSION_ATTRIBUTE_ID:
      {
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             (int8u*)&(myNodeInfo.version),
                             2,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.version);
      }
      break;
    case ZCL_APP_BL_STATUS_ATTRIBUTE_ID:
      {
        int16u pages;
        pages = validateEEPROMImage();
        if(pages > 0) {
          if(halAppBootloaderGetImageData(&(myNodeInfo.timestamp), 
                    myNodeInfo.imageInfo) != EEPROM_SUCCESS) {
            myNodeInfo.imageStatus = APPBL_READ_EEPROM_FAILED;
            record->status = ZCL_FAILURE;
          } else {
            myNodeInfo.imageStatus = APPBL_IMAGE_VALID;
            record->attributeType = ZCL_INT8U_ATTRIBUTE_TYPE;
            record->attributeLocation = (void *)&(myNodeInfo.imageStatus);
          }
        } else {
          myNodeInfo.imageStatus = APPBL_IMAGE_INVALID;
          record->status = ZCL_FAILURE;
        }
      }
      break;
    case ZCL_APP_BL_TIMESTAMP_ATTRIBUTE_ID:
      {
        if(myNodeInfo.imageStatus == APPBL_IMAGE_VALID) {
          record->attributeType = ZCL_INT32U_ATTRIBUTE_TYPE;
          record->attributeLocation = (void *)&(myNodeInfo.timestamp);
        } else {
          record->status = ZCL_FAILURE;
        } // end image checking
      }
      break;
    case ZCL_APP_BL_MFGID_ATTRIBUTE_ID:
      {
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             (int8u*)&(myNodeInfo.mfgId),
                             2,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.mfgId);
      }
      break;
    case ZCL_APP_BL_HWTAG_ATTRIBUTE_ID:
      {
        // assign the length value to the first byte
        globalArray.value[0] = HARDWARE_TAG_SIZE;
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             &(globalArray.value[1]),
                             HARDWARE_TAG_SIZE,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)(globalArray.value);
      }
      break;
    case ZCL_APP_BL_APPBL_VERSION_ATTRIBUTE_ID:
      {
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             (int8u*)&(myNodeInfo.blVersion),
                             2,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.blVersion);
      }
      break;
    case ZCL_APP_BL_REC_IMAGE_VERSION_ATTRIBUTE_ID:
      {
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             (int8u*)&(myNodeInfo.recVersion),
                             2,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.recVersion);
      }
      break;
    case ZCL_APP_BL_IMAGE_INFO_ATTRIBUTE_ID:
      {
        if(myNodeInfo.imageStatus == APPBL_IMAGE_VALID) {
          // assign the length value to the first byte
          globalArray.value[0] = EBL_IMAGE_INFO_SIZE;
          MEMCOPY(&(globalArray.value[1]), 
                    myNodeInfo.imageInfo, EBL_IMAGE_INFO_SIZE);
          record->attributeType = ZCL_OCTET_STRING_ATTRIBUTE_TYPE;
          record->attributeLocation = (void *)(globalArray.value);
        } else {
          record->status = ZCL_FAILURE;
        } 
      }
      break;
    case ZCL_APP_BL_BOOTLOADER_TYPES_ATTRIBUTE_ID:
      {
        record->status = zclUtilReadAttribute(ZCL_APP_BOOTLOAD_CLUSTER_ID, 
                             attributeId, 
                             &(myNodeInfo.blType),
                             1,
                             &dataType);
        record->attributeType = dataType;
        record->attributeLocation = (void *)&(myNodeInfo.blType);
      }
      break;
    default:
      record->status = ZCL_UNSUPPORTED_ATTRIBUTE;
      break;
  }
}

// The function is called to print out query or validate responses received.
static void displayResponseData(EmberNodeId sender, EmberMessageBuffer buffer,
                                readAttributesResponseCmd *response)
{
  int8u count = response->recordsCount;
  readAttributeStatusRecord record;
  int8u i;
  
  bl_print("QQ ID:%2x ", sender);
  for(i=0; i<count; ++i) {
    record = response->statusRecords[i];
    if(record.status != ZCL_SUCCESS) {
      bl_print("ATTR(0x%x):ERROR ", record.attributeId);
      // continue to next record
      continue;
    }

    switch(record.attributeId) {
      case ZCL_APP_BL_EUI64_ATTRIBUTE_ID:
        {
          EmberEUI64 senderEui;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, senderEui);
          bl_print("EUI64:%x%x%x%x%x%x%x%x ", 
                       senderEui[0], senderEui[1], senderEui[2], senderEui[3],
                       senderEui[4], senderEui[5], senderEui[6], senderEui[7]);
        }
        break;
      case ZCL_APP_BL_NODETYPE_ATTRIBUTE_ID:
        {
          EmberNodeType tmp;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &tmp);
          bl_print("TYPE:%s ",
            (tmp == EMBER_COORDINATOR ? "C0R": \
            (tmp == EMBER_ROUTER ? "RTR": \
            (tmp == EMBER_END_DEVICE ? "END": \
            (tmp == EMBER_SLEEPY_END_DEVICE ? "SLP": \
            (tmp == EMBER_MOBILE_END_DEVICE ? "MBL": "UNK"))))));
        }
        break;
      case ZCL_APP_BL_APP_VERSION_ATTRIBUTE_ID:
        {
          int16u appVersion;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &appVersion);
          bl_print("APPVER:%2x ", appVersion);
        }
        break;
      case ZCL_APP_BL_TIMESTAMP_ATTRIBUTE_ID:
        {
          int32u timestamp;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &timestamp);
          bl_print("TSTAMP:%4x ", timestamp);
        }
        break;
      case ZCL_APP_BL_MFGID_ATTRIBUTE_ID:
        {
          int16u mfgId;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &mfgId);
          bl_print("MFGID:%2x\r\n", mfgId);
        }
        break;
      case ZCL_APP_BL_HWTAG_ATTRIBUTE_ID:
        {
          // one extra for length byte
          int8u i;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &globalArray);
          bl_print("HWTAG:");
          // skip the length byte
          for(i=1; i<=HARDWARE_TAG_SIZE; ++i) {
            bl_print("%x", globalArray.value[i]);
          }
          bl_print(" ");
        }
        break;
      case ZCL_APP_BL_APPBL_VERSION_ATTRIBUTE_ID:
        {
          int16u blVersion;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &blVersion);
          bl_print("BLVER:%2x ", blVersion);
        }
        break;
      case ZCL_APP_BL_REC_IMAGE_VERSION_ATTRIBUTE_ID:
        {
          int16u recVersion;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &recVersion);
          bl_print("RECVER:%2x ", recVersion);
        }
        break;
      case ZCL_APP_BL_IMAGE_INFO_ATTRIBUTE_ID:
        {
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &globalArray);
          // skip the length byte which is the first byte
          printImageInfo(&(globalArray.value[1]));
        }
        break;
      case ZCL_APP_BL_STATUS_ATTRIBUTE_ID:
        {
          int8u status;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &status);
          bl_print("IMGSTATUS:%s ", 
            (status == APPBL_IMAGE_VALID ? "VALID": "INVALID"));
        }
        break;
      case ZCL_APP_BL_BOOTLOADER_TYPES_ATTRIBUTE_ID:
        {
          int8u type;
          getAttributeFromLocation(buffer, record.attributeLocation, 
              record.attributeType, &type);
          bl_print("BLTYPE:%s ", 
            (type == BL_TYPE_NULL ? "NUL": 
            (type == BL_TYPE_STANDALONE ? "SBL":
            (type == BL_TYPE_APPLICATION ? "ABL" : "UNK"))));
        }
        break;
      default:
        bl_print("ATTR(0x%x):UNKWN ", record.attributeId);
        break;
    } // end switch statement
    emberSerialWaitSend(appSerial);
  } // end for loop
  bl_print("\r\n");
}

// The function is called to read data from EEPROM and stores in dataBlock
// array.
static int16u totalData = 0;

static boolean readEEPROMData()
{
  int8u currentIndex, chunkNumber;
  
  // This is the start of bootload process.  We first check if the image 
  // (stored on EEPROM) is valid, if so then we also know how many pages of 
  // EEPROM we need to read.  
  if(currentOTABlock == 0) {
    totalData = validateEEPROMImage();
    if(totalData == 0) {
      bl_print("Error: invalid image on EEPROM\r\n");
      return FALSE;
    } else {
      bl_print("Valid image on EEPROM, size 0x%2x pages\r\n", 
                totalData);
    }
  } // end checking for the start of bootloading

  // check the current page number we are at.
  if((currentOTABlock/BOOTLOAD_NUM_PKTS) < totalData) {
    // determine which chunk of data we are sending.
    chunkNumber = (int8u)(currentOTABlock & (BOOTLOAD_NUM_PKTS-1));
    if(chunkNumber == 0) {
      // we are sending the first chunk so read data from EEPROM one page 
      // at a time. EEPROM page size is 128 bytes.
      if(halAppBootloaderReadDownloadSpace(
          (int16u)(currentOTABlock/BOOTLOAD_NUM_PKTS), 
          dataBlock) != EEPROM_SUCCESS) {
        bl_print("Error: read EEPROM failed\r\n");
        return FALSE;
      } 
    }
    // Set length byte to bootload payload size;
    globalArray.value[0] = BOOTLOAD_PAYLOAD_SIZE;
    // Calculate the index in the data block to copy from.
    currentIndex = chunkNumber * BOOTLOAD_PAYLOAD_SIZE;
    // Copy data to the zcl global array
    MEMCOPY(&(globalArray.value[1]), 
          &dataBlock[currentIndex], BOOTLOAD_PAYLOAD_SIZE);
  } else {
    // we are done sending data.
    blState = BOOTLOAD_STATE_SEND_COMPLETE;
    return FALSE;
  }
  return TRUE;
}

// The function writes received (bootload) data to its eeprom in order to
// be used for upgraded.
static int8u blockCount = 0;
static boolean writeEEPROMData(int16u block)
{
  int8u chunkNumber, currentIndex;
  int8u status;
  // determine which chunk of data we are writing.
  chunkNumber = (int8u)(block & (BOOTLOAD_NUM_PKTS-1));
  if(chunkNumber == 0) {
    // we are receiving the first block out of total of BOOTLOAD_NUM_PKTS 
    // blocks.
    blockCount = 1;
  } else {
    ++blockCount;
  }
  // Calculate the index in the data block to write to.
  currentIndex = chunkNumber * BOOTLOAD_PAYLOAD_SIZE;
  // Copy data to the zcl global array
  MEMCOPY(&dataBlock[currentIndex],
          &(globalArray.value[1]), 
          BOOTLOAD_PAYLOAD_SIZE);
  // if we receive 128 bytes of data, we write it to EEPROM.  We write data
  // one EEPROM page (128 bytes) at a time.
  if(blockCount == BOOTLOAD_NUM_PKTS) {
    status = halAppBootloaderWriteDownloadSpace((int16u)(block/BOOTLOAD_NUM_PKTS), 
                                                dataBlock);
    if(status != EEPROM_SUCCESS) {
      bl_print("Error: write EEPROM page %2x failed\r\n", 
              (block/BOOTLOAD_NUM_PKTS));
      return FALSE;
    } 
  }
  return TRUE;
}

static void printImageInfo(int8u* info)
{
  int8u i;
  bl_print("IMGINFO:");
  for(i=0; i<EBL_IMAGE_INFO_SIZE; ++i) {
    bl_print("%c", info[i]);
  }
  bl_print("\r\n");
  emberSerialWaitSend(appSerial);
}

static int16u validateEEPROMImage(void)
{
  int16u pages;
  halAppBootloaderImageIsValidReset();
  while ( (pages = halAppBootloaderImageIsValid() ) 
            == BL_IMAGE_IS_VALID_CONTINUE) {
    appBootloadUtilValidatingEEPROMImageHandler();
    // EEPROM driver code is already calling reset watchdog.  Do not need
    // to call it again here.
  }
  return pages;
}

// Set message flag for all children
static void setAllChildrenMessageFlag(void) {
  int8u i;
  EmberNodeId tmp;
  // set all child flag
  for(i=0; i<EMBER_CHILD_TABLE_SIZE; ++i) {
    tmp = emberChildId(i);
    emberSetMessageFlag(tmp);
  }
}

static EmberMessageBuffer buildCompleteCmd(boolean response,
                                           int8u *transactionSequenceNumber, 
                                           int8u commandId,
                                           void *fields)
{
  command cmd;
  header h;
  static int8u sequenceNumber = 0;
  int8u bytesAppended;

  cmd.buffer = emberAllocateStackBuffer();
  if (cmd.buffer == EMBER_NULL_MESSAGE_BUFFER)
    return EMBER_NULL_MESSAGE_BUFFER;

  h.clusterSpecific = TRUE;
  h.manufacturerSpecific = TRUE;
  h.serverToClient = FALSE;
  h.manufacturerCode = myNodeInfo.mfgId;
  if (response == FALSE) {
      h.transactionSequenceNumber = sequenceNumber++;
      if (transactionSequenceNumber != NULL)
        *transactionSequenceNumber = h.transactionSequenceNumber;
  } else
      h.transactionSequenceNumber = *transactionSequenceNumber;
  h.commandId = commandId;

  bytesAppended = reallyBuildCmd(&cmd, &h, fields);

  if (bytesAppended == 0) {
    emberReleaseMessageBuffer(cmd.buffer);
    cmd.buffer = EMBER_NULL_MESSAGE_BUFFER;
  }
  return cmd.buffer;
}

static int8u reallyBuildCmd(command *cmd, header *h, void *fields)
{
  cmd->locationIndex = 0;
  cmd->success = TRUE;
  appendHeader(cmd, h);
  if((h->commandId == APP_BL_QUERY_COMMAND_ID) ||
    (h->commandId == APP_BL_VALIDATE_COMMAND_ID)) {
    appendReadAttributePayload(cmd, (readAttributesCmd*) fields);
  } else if((h->commandId == APP_BL_QUERY_RESPONSE_COMMAND_ID) ||
    (h->commandId == APP_BL_VALIDATE_RESPONSE_COMMAND_ID)) {
    appendReadAttributeResponsePayload(cmd, (readAttributesResponseCmd*) fields);
  } else if(h->commandId == APP_BL_REPORT_ATTRIBUTES_COMMAND_ID) {
    appendReportAttributePayload(cmd, (reportAttributesCmd*) fields);
  } 

  if (!cmd->success) {
    // prune any data that was added prior to the failure
    cmd->locationIndex = 0;
    RESET_BUFFER_LENGTH(cmd, 0);
  }
  return (cmd->locationIndex - 0);
}

static void appendReadAttributePayload(command *cmd, readAttributesCmd* fields)
{
  int8u i;
  int8u attrCount = fields->attributeIdsCount;
  for(i=0; i<attrCount; ++i) {
    append16BitData(cmd, fields->attributeIds[i]);
  }
}

static void appendReadAttributeResponsePayload(command *cmd, 
                                               readAttributesResponseCmd* fields)
{
  int8u count = fields->recordsCount;
  readAttributeStatusRecord *statusRecord = fields->statusRecords;

  for (; count; count--) {
    append16BitData(cmd, statusRecord->attributeId);
    append8BitData(cmd, statusRecord->status);
    if (statusRecord->status == ZCL_SUCCESS) {
      append8BitData(cmd, statusRecord->attributeType);
      appendAttribute(cmd, statusRecord->attributeType,
                         statusRecord->attributeLocation);
    }
    statusRecord++;
  }
}

static void appendReportAttributePayload(command *cmd, 
                                         reportAttributesCmd* fields)
{
  int8u count = fields->recordsCount;
  reportAttributeRecord *attrRecord = fields->reportRecords;
  int8u attributeType;

  for (; count; count--) {
    append16BitData(cmd, attrRecord->attributeId);
    zclUtilGetAttributeDataType(ZCL_APP_BOOTLOAD_CLUSTER_ID,
                                attrRecord->attributeId,
                                &attributeType);
    appendAttribute(cmd, attributeType, attrRecord->attributeLocation);
    attrRecord++;
  }
}

static void appendHeader(command *cmd, header *h)
{
  int8u headerBuffer[ZCL_MAX_HEADER_LENGTH];
  int8u *finger = headerBuffer;

  *finger  = 0; // zero out reserved fields in frame-control
  if (h->clusterSpecific)
    *finger |= ZCL_CLUSTER_SPECIFIC_COMMAND;
  if (h->manufacturerSpecific)
    *finger |= ZCL_MANUFACTURER_SPECIFIC_MASK;
  if (h->serverToClient)
    *finger |= ZCL_DIRECTION_MASK;
  finger++;
  if (h->manufacturerSpecific) {
    *finger++ = LOW_BYTE(h->manufacturerCode);
    *finger++ = HIGH_BYTE(h->manufacturerCode);
  }
  *finger++ = h->transactionSequenceNumber;
  *finger++ = h->commandId;
  appendBytes(cmd, (int8u)(finger - headerBuffer), headerBuffer);
}

static int8u append8BitData(command *cmd, int8u data)
{
  int8u startIndex = cmd->locationIndex;
  if (SET_BUFFER_LENGTH(cmd, startIndex + 1)) {
    cmd->locationIndex += 1;
    SET_BUFFER_BYTE(cmd->buffer, startIndex, data);
  } else {
    debug("can not add 8 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u append16BitData(command *cmd, int16u data)
{
  int8u startIndex = cmd->locationIndex;
  if (SET_BUFFER_LENGTH(cmd, startIndex + 2)) {
    cmd->locationIndex += 2;
    SET_BUFFER_BYTE(cmd->buffer, startIndex, LOW_BYTE(data));
    SET_BUFFER_BYTE(cmd->buffer, startIndex+1, HIGH_BYTE(data));
  } else {
    debug("can not add 16 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u append32BitData(command *cmd, int32u data)
{
  int8u startIndex = cmd->locationIndex;
  int8u i;
  if (SET_BUFFER_LENGTH(cmd, startIndex + 4)) {
    cmd->locationIndex += 4;
    for (i = startIndex; i < cmd->locationIndex; i++) {
      SET_BUFFER_BYTE(cmd->buffer, i, ((int8u)data & 0xFF));
      data = (data >> 8);
    }
  } else {
    debug("can not add 32 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u appendBytes(command *cmd, int8u bytesLen, int8u *bytes)
{
  int8u startIndex = cmd->locationIndex;
  if (SET_BUFFER_LENGTH(cmd, startIndex + bytesLen)) {
    cmd->locationIndex += bytesLen;
    COPY_TO_BUFFER(bytes, cmd->buffer, startIndex, bytesLen);
  } else {
    debug("can not add bytes at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u appendString(command *cmd, int8u *data)
{
  return appendBytes(cmd, *data + 1, data);
}

static int8u appendAttribute(command *cmd, int8u attributeType, 
                             void *attributeData)
{
  int8u startIndex = cmd->locationIndex;
  
  switch (attributeType) {
  case ZCL_INT8U_ATTRIBUTE_TYPE: 
    append8BitData(cmd, *((int8u *)attributeData));
    break;
  case ZCL_INT16U_ATTRIBUTE_TYPE: 
    append16BitData(cmd, *((int16u *)attributeData));
    break;
  case ZCL_INT32U_ATTRIBUTE_TYPE: 
    append32BitData(cmd, *((int32u *)attributeData));
    break;
  case ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE: 
    appendEmberEUI64(cmd, *((EmberEUI64 *)attributeData));
    break;
  case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: 
    appendCharString(cmd, ((bootloadString *)attributeData));
    break;
  case ZCL_OCTET_STRING_ATTRIBUTE_TYPE: 
    appendOctetString(cmd, ((bootloadString *)attributeData));
    break;
  default:
    debug("can not add attribute - unknown type 0x%x\r\n", attributeType);
    cmd->success = FALSE;
    break;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u parseCompleteCmd(EmberMessageBuffer buffer,
                              int8u *transactionSequenceNumber, 
                              void *fields)
{
  command cmd;
  header h;
  int8u bytesObtained;

  cmd.buffer = buffer;
  bytesObtained = reallyParseCmd(&cmd, &h, fields);

  if (bytesObtained) {
    if (transactionSequenceNumber != NULL)
      *transactionSequenceNumber = h.transactionSequenceNumber;
    return (bytesObtained);
  }

  return 0;
}

static int8u reallyParseCmd(command *cmd, header *h, void *fields)
{
  cmd->locationIndex = 0;
  cmd->success = TRUE;
  getHeader(cmd, h);
  if(h->commandId == APP_BL_QUERY_COMMAND_ID) {
    getReadAttributePayload(cmd, MAX_ATTRIBUTE_QUERY, (readAttributesCmd*)fields);
  } else if (h->commandId == APP_BL_VALIDATE_COMMAND_ID){
    getReadAttributePayload(cmd, MAX_ATTRIBUTE_VALIDATE, (readAttributesCmd*)fields);
  } else if ((h->commandId == APP_BL_QUERY_RESPONSE_COMMAND_ID) ||
             (h->commandId == APP_BL_VALIDATE_RESPONSE_COMMAND_ID)) {
    getReadAttributeResponsePayload(cmd, (readAttributesResponseCmd *)fields);
  } else if (h->commandId == APP_BL_REPORT_ATTRIBUTES_COMMAND_ID) {
    getReportAttributePayload(cmd, (reportAttributesCmd *)fields);
  } 

  if (!cmd->success) {
    // reset updates to location that occurred prior to the failure
    cmd->locationIndex = 0;
  }
  return (cmd->locationIndex - 0);
}

static void getReadAttributePayload(command *cmd, 
                                    int8u length, 
                                    readAttributesCmd *fields)
{
  int8u i;
  fields->attributeIdsCount = length;
  for(i=0; i<length; ++i) {
    get16BitData(cmd, &(fields->attributeIds[i]));
  }
}

static void getReadAttributeResponsePayload(command *cmd, 
                                             readAttributesResponseCmd *fields)
{
  int8u count = fields->recordsCount;
  readAttributeStatusRecord *statusRecord = fields->statusRecords;
  int8u bufferLen = GET_BUFFER_LENGTH(cmd);

  fields->recordsCount = 0;
  for (; count && cmd->locationIndex < bufferLen; count--) {
    get16BitData(cmd, &(statusRecord->attributeId));
    get8BitData(cmd, &(statusRecord->status));
    if (statusRecord->status == ZCL_SUCCESS) {
      get8BitData(cmd, &(statusRecord->attributeType));
      getAttributeLocation(cmd, statusRecord->attributeType,
                              &(statusRecord->attributeLocation));
    }
    fields->recordsCount++;
    statusRecord++;
  }
}

static void getReportAttributePayload(command *cmd, reportAttributesCmd *fields)
{
  int8u count = fields->recordsCount;
  reportAttributeRecord *attrRecord = fields->reportRecords;
  int8u attributeType;
  int8u bufferLen = GET_BUFFER_LENGTH(cmd);

  fields->recordsCount = 0;
  for (; count && cmd->locationIndex < bufferLen; count--) {
    get16BitData(cmd, &(attrRecord->attributeId));
    zclUtilGetAttributeDataType(ZCL_APP_BOOTLOAD_CLUSTER_ID,
                                attrRecord->attributeId,
                                &attributeType);
    getAttributeLocation(cmd, attributeType, &(attrRecord->attributeLocation));
    fields->recordsCount++;
    attrRecord++;
  }
}

static void getHeader(command *cmd, header *h)
{
  int8u headerBuffer[ZCL_MAX_HEADER_LENGTH];
  int8u *finger = headerBuffer;

  getBytes(cmd, ZCL_MIN_HEADER_LENGTH, headerBuffer);
  h->clusterSpecific = ((*finger & ZCL_FRAME_TYPE_MASK)
                             == ZCL_CLUSTER_SPECIFIC_COMMAND);
  h->manufacturerSpecific = ((*finger & ZCL_MANUFACTURER_SPECIFIC_MASK)
                                  == ZCL_MANUFACTURER_SPECIFIC_MASK);
  h->serverToClient = ((*finger & ZCL_DIRECTION_MASK) == ZCL_DIRECTION_MASK);
  finger++;
  if (h->manufacturerSpecific) {
    int8u low = *finger++;
    int8u high = *finger++;
    h->manufacturerCode = HIGH_LOW_TO_INT(high, low);
    getBytes(cmd, ZCL_MAX_HEADER_LENGTH - ZCL_MIN_HEADER_LENGTH, finger);
  }
  h->transactionSequenceNumber = *finger++;
  h->commandId = *finger;
}

static void getAttributeLocation(command *cmd, int8u type, 
                                 void **attributeLocation)
{
  int8u attributeLength = attributeTypeLength(type, 
      GET_BUFFER_PTR(cmd->buffer, cmd->locationIndex));
  
  if (GET_BUFFER_LENGTH(cmd) >= cmd->locationIndex + attributeLength) {
    // Since the application won't know how much space to allocate for the 
    // incoming attribute before calling the parse function we need to simply 
    // store the location within the buffer where the attribute is located. 
    // Later when the application actually goes to use the attribute data 
    // they'll need to first extract the data by calling 
    // getAttributeFromLocation()
    *attributeLocation = (void *)((PointerType)cmd->locationIndex);
    cmd->locationIndex += attributeLength;
  } else {
    debug("can not get attribute location at 0x%x\r\n", cmd->locationIndex);
    cmd->success = FALSE;
  }
}

static int8u getAttributeFromLocation(EmberMessageBuffer buffer, 
                                      void *attributeLocation, 
                                      int8u attributeType, 
                                      void *attributeData)
{
  command cmd;
  cmd.buffer = buffer;
  cmd.locationIndex = (PointerType)attributeLocation;
  return getAttribute(&cmd, attributeType, attributeData);
}

static int8u attributeTypeLength(int8u type, int8u *datap)
{
  if ((type == ZCL_OCTET_STRING_ATTRIBUTE_TYPE) ||
      (type == ZCL_CHAR_STRING_ATTRIBUTE_TYPE)) {
    return (1 + *datap);
  } else if (type == ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE) {
    return EUI64_SIZE;
  } else if (type == ZCL_INT8U_ATTRIBUTE_TYPE) {
    return 1;
  } else if (type == ZCL_INT16U_ATTRIBUTE_TYPE) {
    return 2;
  } else if (type == ZCL_INT32U_ATTRIBUTE_TYPE) {
    return 4;
  } else if (type == ZCL_UNKNOWN_ATTRIBUTE_TYPE) {
    return 0;
  } else {
    return 0;
  }
}

static int8u get8BitData(command *cmd, int8u *data)
{
  int8u startIndex = cmd->locationIndex;
  if (GET_BUFFER_LENGTH(cmd) >= startIndex + 1) {
    cmd->locationIndex += 1;
    *data = GET_BUFFER_BYTE(cmd->buffer, startIndex);
  } else {
    debug("can not get 8 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u get16BitData(command *cmd, int16u *data)
{
  int8u startIndex = cmd->locationIndex;
  if (GET_BUFFER_LENGTH(cmd) >= startIndex + 2) {
    cmd->locationIndex += 2;
    *data = HIGH_LOW_TO_INT(GET_BUFFER_BYTE(cmd->buffer, startIndex + 1),
                            GET_BUFFER_BYTE(cmd->buffer, startIndex));
  } else {
    debug("can not get 16 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u get32BitData(command *cmd, int32u *data)
{
  int8u startIndex = cmd->locationIndex;
  int8u i;
  if (GET_BUFFER_LENGTH(cmd) >= startIndex + 4) {
    cmd->locationIndex += 4;
    for (i = cmd->locationIndex; i > startIndex ; i--)
      *data = (*data << 8) + GET_BUFFER_BYTE(cmd->buffer, i - 1);
  } else {
    debug("can not get 32 bit data at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u getBytes(command *cmd, int8u bytesLen, int8u *bytes)
{
  int8u startIndex = cmd->locationIndex;
  if (GET_BUFFER_LENGTH(cmd) >= startIndex + bytesLen) {
    cmd->locationIndex += bytesLen;
    COPY_FROM_BUFFER(bytes, cmd->buffer, startIndex, bytesLen);
  } else {
    debug("can not get bytes at location 0x%x\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u getString(command *cmd, int8u maxStringLen, int8u *data)
{
  int8u startIndex = cmd->locationIndex;
  int8u len;
  if ((GET_BUFFER_LENGTH(cmd) >= startIndex + 1) &&
      ((len = GET_BUFFER_BYTE(cmd->buffer, startIndex)) <= maxStringLen)) {
    getBytes(cmd, len + 1, data);
  } else {
    debug("can not get string at location 0x%x, len too big\r\n", startIndex);
    cmd->success = FALSE;
  }
  return (cmd->locationIndex - startIndex);
}

static int8u getAttribute(command *cmd, int8u attributeType, void *attributeData)
{
  int8u startIndex = cmd->locationIndex;
  
  switch (attributeType) {
  case ZCL_INT8U_ATTRIBUTE_TYPE: 
    get8BitData(cmd, (int8u *)attributeData);
    break;
  case ZCL_INT16U_ATTRIBUTE_TYPE: 
    get16BitData(cmd, (int16u *)attributeData);
    break;
  case ZCL_INT32U_ATTRIBUTE_TYPE: 
    get32BitData(cmd, (int32u *)attributeData);
    break;
  case ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE: 
    getEmberEUI64(cmd, (EmberEUI64 *)attributeData);
    break;
  case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    getCharString(cmd, (bootloadString *)attributeData);
    break;
  case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    getOctetString(cmd, (bootloadString *)attributeData);
    break;
  default:
    debug("can not get attribute - unknown type 0x%x\r\n", attributeType);
    cmd->success = FALSE;
    break;
  }
  return (cmd->locationIndex - startIndex);
}

// Stub functions to keep zcl-utils happy (bug 10081)
void zclUtilReportTableTick(void) {}
void zclUtilReportTableClear(void) {}

zclReportEntry* zclUtilFindReportEntry(int16u clusterId, 
                                       int16u attributeId, 
                                       int16u shortId,
                                       int8u  direction) {return NULL;}
int8u zclUtilAddTxEntry(int16u clusterId,
                        int16u attributeId,
                        int8u  dataType,
                        int16u minInterval,
                        int16u maxInterval,
                        int32u reportableChange,
                        int16u shortId) {return 0;}
int8u zclUtilAddRxEntry(int16u clusterId,
                        int16u attributeId,
                        int32u reportableChange,
                        int16u timeout, 
                        int16u shortId) {return 0;}
boolean zclSetMyTimeBasedOnNextReadAttrResp = FALSE;
EmberStatus zaAppSendUnicast(EmberOutgoingMessageType type,
                             int16u indexOrDestination,
                             EmberApsFrame *apsFrame,
                             int8u messageLength,
                             int8u* message) {return 0;}
EmberStatus zaAppSendInterpanUnicast(int16u destinationShortAddress,
                                     int16u destinationPanId,
                                     int16u destinationAppProfileId,
                                     int16u destinationClusterId,
                                     int16u multicastIdOrZero,
                                     EmberEUI64* longAddressOrNull,
                                     int8u messageLength,
                                     int8u* messageBytes) {return 0;}
void zclUtilUpdateReportEntryStatus(int16u sourceOrDest, 
                                    EmberStatus messageSentStatus) {}
void addFakeSignature(void) {}
                                     
#else //Define stubbed app bootload util functions

void appBootloadUtilStartXModem() {}

void appBootloadUtilSendQuery(int16u address, boolean isJIT) {}

void appBootloadUtilStartBootload(EmberNodeId target) {}

void appBootloadUtilValidate(EmberNodeId target) {}

void appBootloadUtilUpdate(EmberNodeId target) {}

void appBootloadUtilIncomingMessageHandler(EmberMessageBuffer message) {}

void appBootloadUtilMessageSent(int16u destination, 
                                EmberApsFrame *apsFrame,
                                EmberMessageBuffer message, 
                                EmberStatus status) {}

void appBootloadUtilTick(void) {}

void appBootloadUtilInit(int8u appPort, 
                         int8u bootloadPort, 
                         int16u appProfileId) {}

#endif//USE_APP_BOOTLOADER_LIB
