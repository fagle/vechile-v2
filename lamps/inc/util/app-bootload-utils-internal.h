// *******************************************************************
// * app-bootload-utils-internal.h
// * 
// * Utilities used by bootload library.  Functions and variables defined
// * in this file are used internally by the bootload library.  Applications
// * should not have to worry about using/calling any functions or variables
// * defined here.
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#ifdef XAP2B
#include "hal/micro/xap2b/em250/eeprom.h"   // EEPROM driver
#endif
#ifdef CORTEXM3
#include "hal/micro/cortexm3/bootloader/eeprom.h"
#endif
#include "hal/micro/crc.h"

#if (EEPROM_PAGE_SIZE != 128)
#error demo code is optimized for 128 byte EEPROM_PAGE_SIZE. Please verify code.
#endif

// Default bootload baudrate over serial interface to ensure maximum
// performance.
#if defined(XAP2B) || defined(CORTEXM3)
  #define BOOTLOAD_BAUD_RATE  BAUD_115200      // 115200 kbps for em250 and em35x
#else
  #define BOOTLOAD_BAUD_RATE  BAUD_38400      // 38400 kbps for avr/em2420
#endif


// Various time out values used in bootload state machine
#define TIME_INCREMENT         50   // ms
#define TIMEOUT_NONE            0   // Act on immediately
#define TIMEOUT_IMAGE_SEND     (3000/TIME_INCREMENT)   // 3 seconds.
#define TIMEOUT_GENERIC        (5000/TIME_INCREMENT)   // 5 seconds.
#define TIMEOUT_VALIDATE       (15000/TIME_INCREMENT)  // 15 seconds.
#define TIMEOUT_LONG           (60000L/TIME_INCREMENT)  // 60 seconds.

// Poll interval for sleepy and mobile node types.  Note that if the poll
// interval is longer than the value of EMBER_MOBILE_NODE_POLL_TIMEOUT then
// the poll will trigger a rejoin.  The default value of 
// EMBER_MOBILE_NODE_POLL_TIMEOUT is 5 s.
//
// The normal poll interval used is 1 second.  However, during bootloading,
// the node will poll at a much faster rate of 50 ms.
#define POLL_INTERVAL          20   // 1 second
#define BL_POLL_INTERVAL       1    // 50 ms

// Maximum data size according to XModem protocol over serial line.
#define XMODEM_BLOCK_SIZE   128   // bytes

// XModem protocol message types
#define XMODEM_SOH    0x01  // Start of Header
#define XMODEM_EOT    0x04  // End of Transmission
#define XMODEM_ACK    0x06  // Acknowledge
#define XMODEM_NAK    0x15  // Negative Acknowledge
#define XMODEM_CANCEL 0x18  // Cancel (from sender)
#define XMODEM_READY  0x43  // ASCII 'C'
#define XMODEM_CC     0x03  // Cancel (from sender, user)

// State and timeout values used when dealing with serial port.
#define SERIAL_DATA_OK        0x00
#define SERIAL_TIMEOUT_3S     30    // In 0.1 second increments
#define SERIAL_TIMEOUT_1S     10    // In 0.1 second increments
#define NO_SERIAL_DATA        0x01

// Variables used in XModem state machine.
#define STATUS_OK           0   // Receive good data packet
#define STATUS_NAK          1   // Receive nack packet
#define STATUS_ACK          2   // Receive ack packet
#define STATUS_RESTART      3   // Waiti for serial input or recieve bad packet
#define STATUS_DUPLICATE    4   // Receive duplicate packet
#define STATUS_DONE         5   // Finish XModem transfer
#define STATUS_ABORT        6   // Abort XModem transfer

// Maximum size of data (read from eeprom) that will be send over the air
#define BOOTLOAD_PAYLOAD_SIZE   64    // bytes

// Number of bootload data packets that need to be sent which is two in
// this case because we are using BOOTLOAD_PAYLOAD_SIZE of 64 bytes.
#define BOOTLOAD_NUM_PKTS   (EEPROM_PAGE_SIZE/BOOTLOAD_PAYLOAD_SIZE)

// Size of hardware tag which is an array of int8u.
#define HARDWARE_TAG_SIZE       16    // bytes

// Image info size
#define EBL_IMAGE_INFO_SIZE     32    // bytes

//----------------------------------------------------------------
// Application Bootloader Cluster Id's Commands and Attributes

// Application Bootloader status
enum {
  APPBL_READY,
  APPBL_WRITE_EEPROM_FAILED,
  APPBL_READ_EEPROM_FAILED,
  APPBL_PARSE_DATA_FAILED,
  APPBL_IMAGE_VALID,
  APPBL_IMAGE_INVALID,
  APPBL_TIMEOUT_SRC,
  APPBL_UPDATE,
  APPBL_COMPLETE
};

// Application Bootloader specific command ids (ZCL_CLUSTER_SPECIFIC_COMMAND)
enum {
  APP_BL_QUERY_COMMAND_ID,
  APP_BL_QUERY_RESPONSE_COMMAND_ID,
  APP_BL_VALIDATE_COMMAND_ID,
  APP_BL_VALIDATE_RESPONSE_COMMAND_ID,
  APP_BL_INITIATE_COMMAND_ID,
  APP_BL_INITIATE_RESPONSE_COMMAND_ID,
  APP_BL_WRITE_TO_EEPROM_COMMAND_ID,
  APP_BL_UPDATE_COMMAND_ID,
  APP_BL_UPDATE_RESPONSE_COMMAND_ID,
  APP_BL_REPORT_ATTRIBUTES_COMMAND_ID
};

// Maximum number of attributes defined for each command used in the library
#define MAX_READ_ATTRIBUTE_COUNT    9 //MAX_ATTRIBUTE_QUERY + sender address
#define MAX_ATTRIBUTE_QUERY         8
#define MAX_ATTRIBUTE_VALIDATE      3
#define MAX_ATTRIBUTE_REPORT        1

// node information storage struct
typedef struct {
  EmberEUI64 eui64;
  EmberNodeType type;
  EmberNodeId shortId;
  int16u version;
  int32u timestamp;
  int8u imageInfo[32];
  int8u imageStatus;
  int16u mfgId;
  int16u hwTag[HARDWARE_TAG_SIZE];
  int16u blVersion;
  int16u recVersion;
  int8u blType;
} NodeInfo;


//----------------------------------------------------------------
// Bootload utility library printing options

// Debug Print: this should come in handy during development
// To enable debug prints, #define ENABLE_DEBUG.  The printing is off by default.  
//#define ENABLE_DEBUG  
#ifdef ENABLE_DEBUG
  #define debug(...)                                  \
  do {                                                \
    emberSerialPrintf(appSerial,__VA_ARGS__);         \
    emberSerialWaitSend(appSerial);                   \
  } while (0)
#else
  #define debug(...)                                  \
  do {                                                \
    ;                                                 \
  } while (0)
#endif

// Bootload Print: enable more verbose feedback.
// To enable debug prints, #define ENABLE_BOOTLOAD_PRINT.  
// The printing is off by default.  
//#define ENABLE_BOOTLOAD_PRINT  
#ifdef ENABLE_BOOTLOAD_PRINT
  #define bl_print(...)                               \
  do {                                                \
    emberSerialPrintf(appSerial,__VA_ARGS__);         \
    emberSerialWaitSend(appSerial);                   \
  } while (0)
#else
  #define bl_print(...)                               \
  do {                                                \
    ;                                                 \
  } while (0)
#endif

//----------------------------------------------------------------
// Typedef anad Macro Utility 
typedef struct {
  EmberMessageBuffer buffer; // Location where ZCL command is stored
  int8u locationIndex;  // Indicates the current location within "buffer"
  boolean success;      // Indicates whether an append or get failed.
                        // Will be set to FALSE if the function fails. Left 
                        // unchanged if the function succeeds.  An
                        // application to check the status of a series 
                        // of appends or gets by initially setting this
                        // this field to TRUE then checking after the
                        // series of fuinction calls.
} command;

typedef struct {
  int8u attributeIdsCount;
  int16u attributeIds[MAX_READ_ATTRIBUTE_COUNT];
} readAttributesCmd;

typedef struct {
  int16u attributeId;
  int8u status;
  int8u attributeType;
  void* attributeLocation;
} readAttributeStatusRecord;

typedef struct {
  int8u recordsCount;
  readAttributeStatusRecord *statusRecords;
} readAttributesResponseCmd;

typedef struct {
  int16u attributeId;
  int8u attributeType;
  void* attributeLocation;
} reportAttributeRecord;

typedef struct {
  int8u recordsCount;
  reportAttributeRecord *reportRecords;
} reportAttributesCmd;

typedef struct {
  int8u value[1 + BOOTLOAD_PAYLOAD_SIZE];
} bootloadString;

typedef struct {
  boolean clusterSpecific;         // Is this a global command or a cluster
                                   // specific command?
  boolean manufacturerSpecific;    // Is this a proprietary command or a ZCL
                                   // defined command?
  boolean serverToClient;          // Is the direction of this command toward
                                   // the client or toward the server?
  int16u manufacturerCode;         // If this command is a proprietary 
                                   // command then this field will be 
                                   // populated with a manufacturer specific 
                                   // value.
  int8u transactionSequenceNumber; // Sequence number of the command.
  int8u commandId;                 // The command identifier
} header;

// ZCL command frame header
#define ZCL_COMMAND_ID_OFFSET_CLUSTER_SPECIFIC 4
#define ZCL_MAX_HEADER_LENGTH 5
#define ZCL_MIN_HEADER_LENGTH 3

// ZCL Frame Utility Functions
#define zclFrameType(buffer)                                                   \
  (emberGetLinkedBuffersByte((buffer), 0) & ZCL_FRAME_TYPE_MASK)

#define commandId(buffer) \
  (emberGetLinkedBuffersByte(message, ZCL_COMMAND_ID_OFFSET_CLUSTER_SPECIFIC))
  
#define SET_BUFFER_LENGTH(cmd, len)                                            \
  ((emberSetLinkedBuffersLength((cmd)->buffer, (len))) == EMBER_SUCCESS)

#define SET_BUFFER_BYTE(buffer, index, byte)                                   \
  emberSetLinkedBuffersByte((buffer), (index), (byte))

#define GET_BUFFER_LENGTH(cmd)                                                 \
  emberMessageBufferLength((cmd)->buffer)

#define GET_BUFFER_PTR(buffer, index)                                          \
  emberGetLinkedBuffersPointer((buffer), (index))

#define GET_BUFFER_BYTE(buffer, index)                                         \
  emberGetLinkedBuffersByte((buffer), (index))

#define COPY_TO_BUFFER(loc, buffer, index, len)                                \
  emberCopyToLinkedBuffers((loc), (buffer), (index), (len))

#define COPY_FROM_BUFFER(loc, buffer, index, len)                              \
  emberCopyFromLinkedBuffers((buffer), (index), (loc), (len))

#define RESET_BUFFER_LENGTH(cmd, len)                                          \
  (emberSetLinkedBuffersLength((cmd)->buffer, (len)))

#define appendEmberEUI64(cmd, data) \
  appendBytes((cmd), EUI64_SIZE, (data))

#define appendCharString(cmd, data) \
  appendString((cmd), (data)->value)

#define appendOctetString(cmd, data) \
  appendString((cmd), (data)->value)

#define getEmberEUI64(cmd, data) \
  getBytes((cmd), EUI64_SIZE, (int8u *)(data))

#define getCharString(cmd, data)                              \
  (getString((cmd), BOOTLOAD_PAYLOAD_SIZE, (data)->value))

#define getOctetString(cmnd, data)                             \
  (getString((cmd), BOOTLOAD_PAYLOAD_SIZE, (data)->value))

#define buildReadAttributesCmd(id, fields) \
    buildCompleteCmd(FALSE, NULL, (id), (fields))

#define buildReadAttributesResponseCmd(id, transactionSequenceNumber, fields) \
    buildCompleteCmd(TRUE, (transactionSequenceNumber), (id), (fields))

#define buildReportAttributesCmd(fields) \
    buildCompleteCmd(FALSE, NULL, APP_BL_REPORT_ATTRIBUTES_COMMAND_ID, (fields))

#define buildCmd(cmd, header)         \
    reallyBuildCmd((cmd), (header), NULL)

#define parseReadAttributesCmd(buffer, transactionSequenceNumber, fields) \
    parseCompleteCmd((buffer), (transactionSequenceNumber), (fields))

#define parseReadAttributesResponseCmd(buffer, \
    transactionSequenceNumber, fields) \
    parseCompleteCmd((buffer), (transactionSequenceNumber), (fields))

#define parseReportAttributesCmd(buffer, fields) \
    parseCompleteCmd((buffer), NULL, (fields))

#define parseCmd(cmd, header)      \
    reallyParseCmd((cmd), (header), NULL)

