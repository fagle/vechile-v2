// *******************************************************************
// * bootload-utils-internal.h
// * 
// * Utilities used by bootload library.  Functions and variables defined
// * in this file are used internally by the bootload library.  Applications
// * should not have to worry about using/calling any functions or variables
// * defined here.
// *
// * Copyright 2006 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "hal/micro/crc.h"

// Default bootload baudrate over serial interface to ensure maximum
// performance.
#if defined(XAP2B) 
  #define BOOTLOAD_BAUD_RATE  BAUD_115200      // 115200 kbps for em250
#else
  #define BOOTLOAD_BAUD_RATE  BAUD_38400      // 38400 kbps for avr/em2420
#endif

// The channel that a device uses if it has started a bootload (destroyed
// the application) and then reset while bootloading. This cannot be 
// changed as it is coded into the standalone-bootloader binary.
#define BOOTLOADER_DEFAULT_CHANNEL   13

// To be used in bootloader-demo-v2.c.
extern EmberEUI64 broadcastEui64;

// Important address information needed when doing clone bootloading.  The
// application needs to know where to locate both header and application.
// Note that these are word addresses.
#if defined(XAP2B)
  // For EM250, PROG_STK_INFO_BASEW represents the end of the bootloader, 
  // hence, the beginning of the application space.  Header data contains 
  // useful information regarding the image, refer to <micro>-ebl.h for more 
  // details.  The header locates 64 bytes (or 0x20 words) after the beginning 
  // of application space.
  #define APP_ADDR          PROG_STK_INFO_BASEW
  #define HEADER_ADDR       (APP_ADDR+0x20)
#elif defined(AVR_ATMEGA_128)
  // For EM2420, the application locates at the beginning while the bootloader
  // locates at the end of flash.
  #define APP_ADDR          0x0080
  #define HEADER_ADDR       0xEF80
  #define LAST_DATA_PAGE    0x77  // last page before bootloader
  #define LAST_PAGE_SIZE    (0x0400 - 0x0100) // excludes ebl header offset
#endif

#define TAG_TYPE            0
#define DATA_TYPE           1

// Ebl (file formate) page size is 1024 bytes for both EM250 and AVR128.
// The actual page size for AVR128 is 256 bytes.  However, since the ebl
// format only allocates 8 bit for page number (maximum is 256 pages) which
// is not sufficient for total flash (128 KB) of AVR128.
#define PAGE_SIZE_BYTE    0x0400      // 1024 bytes 

// Various time out values used in bootload state machine
#define TIMEOUT_IMAGE_SEND     15   // In 0.2 second increments = 3 seconds.
#define TIMEOUT_REQUEST         7   // In 0.2 second increments = 1.4 seconds.
#define TIMEOUT_AUTH_CHALLENGE 15   // In 0.2 second increments = 3 seconds.
#define TIMEOUT_AUTH_RESPONSE  15   // In 0.2 second increments = 3 seconds.
#define TIMEOUT_START          25   // In 0.2 second increments = 5 seconds.
#define TIMEOUT_QUERY          25   // In 0.2 second increments = 5 seconds.
#define TIMEOUT_GENERIC         7   // In 0.2 second increments = 1.4 seconds.

// State and timeout values used when dealing with serial port.
#define SERIAL_DATA_OK        0x00
#define SERIAL_TIMEOUT_3S     30    // In 0.1 second increments
#define SERIAL_TIMEOUT_1S     10    // In 0.1 second increments
#ifdef XAP2B
  // Only define for EM250.  Avr128 already defines this in 
  // hal/micro/avr-atmega/128/bootloader.h
  #define NO_SERIAL_DATA        0x01
#endif

// Application-level retry for added robustness.  Query, data and EOT messages
// are each retried NUM_PKT_RETRIES times if response is not received.  
// Note that this is on top of the mac retries.  For every application message 
// sent, there are three mac retries in case of failure.
#define NUM_PKT_RETRIES         10   

// Over the air bootload message types extended from XModem message types.
// When transmitting bootload messages over the air, we also follow basic
// XModem protocol.
#define XMODEM_SOH    0x01  // Start of Header
#define XMODEM_EOT    0x04  // End of Transmission
#define XMODEM_ACK    0x06  // Acknowledge
#define XMODEM_NAK    0x15  // Negative Acknowledge
#define XMODEM_CANCEL 0x18  // Cancel (from sender)
#define XMODEM_READY  0x43  // ASCII 'C'
#define XMODEM_QUERY  0x51  // ASCII 'Q'
#define XMODEM_QRESP  0x52  // ASCII 'R'
#define XMODEM_CC     0x03  // Cancel (from sender, user)
#define XMODEM_LAUNCH_REQUEST 0x4c // ASCII 'L' (Launch Bootloader)
#define XMODEM_AUTH_CHALLENGE 0x63 // ASCII 'c' (Authentication Challenge)
#define XMODEM_AUTH_RESPONSE  0x72 // ASCII 'r' (Auth. Response to Challenge)

// Variables used in XModem state machine.
#define STATUS_OK           0   // Receive good data packet
#define STATUS_NAK          1   // Receive nack packet
#define STATUS_ACK          2   // Receive ack packet
#define STATUS_RESTART      3   // Waiti for serial input or recieve bad packet
#define STATUS_DUPLICATE    4   // Receive duplicate packet
#define STATUS_DONE         5   // Finish XModem transfer
#define STATUS_ABORT        6   // Abort XModem transfer

// Maximum data size according to XModem protocol over serial line.
#define XMODEM_BLOCK_SIZE   128   // bytes

// Maximum data size for bootload message that goes over the air.
#define BOOTLOAD_OTA_SIZE             64    // bytes

// Maximum bootload data packet size which is SOH (data) message.  The message 
// contains BOOTLOAD_OTA_SIZE bytes data and 6 bytes header.
#define MAX_BOOTLOAD_MESSAGE_SIZE     (BOOTLOAD_OTA_SIZE + 6) 

// Current bootloader protocol version
#define BOOTLOAD_PROTOCOL_VERSION 1  

//offset into bootload header and bootloader payload
#define OFFSET_VERSION            0
#define OFFSET_MESSAGE_TYPE       1
#define OFFSET_DEVICE_TYPE        2
#define OFFSET_BLOCK_NUMBER       2
#define OFFSET_ERROR_TYPE         2
#define OFFSET_BLOCK_NUMBER_CHECK 3
#define OFFSET_ERROR_BLOCK        3
#define OFFSET_IMAGE_CONTENT      4

// offsets into bootloader payload for query response message.
#define QRESP_OFFSET_BL_ACTIVE             2 // 1 byte long
#define QRESP_OFFSET_MFG_ID                3 // 2 bytes long
#define QRESP_OFFSET_HARDWARE_TAG          5 // 16 bytes long
#define QRESP_OFFSET_BL_CAPS              21 // 1 byte long
#define QRESP_OFFSET_PLATFORM             22 // 1 byte long
#define QRESP_OFFSET_MICRO                23 // 1 byte long
#define QRESP_OFFSET_PHY                  24 // 1 byte long
#define QRESP_OFFSET_BL_VERSION           25 // 2 bytes long

// offsets into bootloader payload for launch request message.
#define OFFSET_MFG_ID             2 // 2 bytes long
#define OFFSET_HARDWARE_TAG       4 // 8 bytes long 

// offsets into bootloader payload for authentication challenge message.
#define OFFSET_AUTH_CHALLENGE     2 // 16 bytes long

// offsets into bootloader payload for authentication response message.
#define OFFSET_AUTH_RESPONSE      2 // 16 bytes long

// Variables used in bootV1State state machine.
#define BOOT_V1_STATE_IDLE        0   // No v1 bootloading active
#define BOOT_V1_STATE_QRESP       1   // Waiting for QRESP
#define BOOT_V1_STATE_XMODEM      2   // Waiting for xmodem data
#define BOOT_V1_STATE_DATA_ACK    3   // Waiting for data ACK
#define BOOT_V1_STATE_EOT_ACK     4   // Waiting for EOT ACK

//raw offset into bootload header and bootloader payload
#define RAW_OFFSET_SEG_BOOTLOAD   9
#define RAW_OFFSET_SIZE           10
#define RAW_OFFSET_MESSAGE_TYPE   11
#define RAW_OFFSET_DEST_EUI       12
#define RAW_OFFSET_SRC_EUI        20
#define RAW_OFFSET_SEQ_NUMBER     28
#define RAW_OFFSET_SRC_NODEID     29

#define V1_RAW_MAC_OVERHEAD_SIZE  9

#define V1_SEG_BOOTLOAD           0x05

#define V1_CONTROL_PACKET_SIZE (1 + EUI64_SIZE + EUI64_SIZE + 1 + 2)
#define V1_DATA_PACKET_SIZE (1 + EUI64_SIZE + 2 + BOOTLOAD_OTA_SIZE)

//----------------------------------------------------------------
// Bootload utility library printing options

#ifndef BOOTLOAD_PRINTF
#define BOOTLOAD_PRINTF(...) emberSerialPrintf(__VA_ARGS__)
#endif
#ifndef BOOTLOAD_WAITSEND
#define BOOTLOAD_WAITSEND(x) emberSerialWaitSend(x)
#endif

// Debug Print: this should come in handy during development
// To enable debug prints, #define ENABLE_DEBUG.  The printing is off by default.  
//#define ENABLE_DEBUG  
#ifdef ENABLE_DEBUG
  #define debug(...)                                  \
  do {                                                \
    BOOTLOAD_PRINTF(appSerial,__VA_ARGS__);         \
    BOOTLOAD_WAITSEND(appSerial);                   \
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
    BOOTLOAD_PRINTF(appSerial,__VA_ARGS__);         \
    BOOTLOAD_WAITSEND(appSerial);                   \
  } while (0)
#else
  #define bl_print(...)                               \
  do {                                                \
    ;                                                 \
  } while (0)
#endif

/**@description A function is called to send V1 data packet.  This routine
 * will normally not be used by the app.  Is listed here for debug/test.
 *
 * Note that the bootload utility library will call this function automatically
 * if bootloading a V1 node.
 *
 * @param toEui: the node we want to send data to.
 * @param expectedBlock: the expected block number.
 * @param data: the 64 bytes of data to send.
 * 
 */
void sendV1DataPacket(EmberEUI64 toEui, int8u expectedBlock, int8u data[64]);

/**@description A function is called to send V1 control packet.  This routine
 * will normally not be used by the app.  Is listed here for debug/test.
 *
 * Note that the bootload utility library will call this function automatically
 * if bootloading a V1 node.
 *
 * @param toEui: the node we want to send data to.
 * @param type: the type of control message this is.
 * @param sequenceNumber: the expected block number.
 * 
 */
void sendV1ControlPacket(EmberEUI64 toEui, int8u type, int8u sequenceNumber);
