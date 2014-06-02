/** 
 * @file app-bootload-utils.h
 * @brief Utilities used for performing application bootloading.
 * See @ref util_bootload for documentation.
 *
 *  <!-- Copyright 2007 by Ember Corporation. All rights reserved.       *80*-->
 *****************************************************************/

 /** 
 * @addtogroup util_app_bootload
 * All functions and variables defined here can be used by applications.
 * See app-bootload-utils.h for source code.
 *
 * Applications can use this library to: 
 * - Load a new (application) image onto its external memory (EEPROM) 
 *   serially through the UART port 1 using the xmodem protocol. 
 * - Load an image from its EEPROM onto a remote node over-the-air (OTA).
 *
 * The library provides support for bootloading one node at a time over 
 * multiple hops away. Additional code is required to support 
 * bootloading multiple nodes simultaneously.
 *
 * Note that if a problem occurs during installing of a new image,
 * the application bootloader will run a recovery image on the failed node.
 * Then a new image needs to be uploaded serially (via RS232)
 * onto the node using xmodem protocol application such as hyperterm.
 *
 * The following diagrams show typical usage of application bootloader and 
 * recovery images.
 *
 * <b>A diagram for loading an image to external memory (EEPROM):</b>
 *    
 * [host pc] --(RS232 or Ethernet)-- {uart1 or port 4901}[node] -- {EEPROM}
 *
 *   
 * <b>A diagram for typical application bootloading:</b>
 *
 * [source node]--(OTA)[relay node(s)]--[target node]
 *
 *     
 * <b>A diagram for bootloading via a recovery image:</b>
 *    
 * [host pc] --(RS232 or Ethernet)-- {uart1 or port 4901}[node] -- {EEPROM}
 *
 * The library also demonstrates Zigbee Cluster Library (ZCL) usage. The library
 * uses only manufacturer-specific commands and attributes.  Its attributes are
 * stored in the zcl utility attribute table under app/ha/gen. Additionally, 
 * the library uses an Ember-assigned application profile value of 0xC00D.
 * The user must change this value to his/her company's application profile 
 * value during deployment. The company can obtain a range of application 
 * profiles from the Zigbee organization if it does not already have them.
 *
 *
 * @note Applications that use the bootload utilities need to 
 * <code> \#define USE_APP_BOOTLOADER_LIB</code>
 * within their <code> CONFIGURATION_HEADER </code> 
 *
 * @{
 */

// *******************************************************************
// Literals needed by the application.


/**@brief Selected endpoint used for application bootloading. 
 *
 * This is used to filtered incoming radio messages.
 */
#define APP_BOOTLOAD_ENDPOINT 240


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**@brief A value that application can check whether bootloading is in 
 * progress. Used in a bootload state machine.
 * 
 * This is necessary because we want the application to be aware that 
 * bootloading is going on and it needs to limit its activities.  For example,
 * the application should not print anything to the serial port when passthru 
 * bootloading is going on because it may violate the XModem protocol. 
 * Try to limit radio activities to a minimum to avoid any interruptions to 
 * the bootload progress.
 * 
 *
 *The following bootload states are supported by the application bootload utility library.
 */
enum bootloadState
#else
typedef int8u bootloadState;
enum
#endif
{
  BOOTLOAD_STATE_NORMAL,                  /*!< initial state (no bootloading) */
  BOOTLOAD_STATE_QUERY,                   /*!< after sending query message */
  BOOTLOAD_STATE_SEND_QUERY_RESP,         /*!< wait to send query response */
  BOOTLOAD_STATE_INITIATE,                /*!< after send initiate message */
  BOOTLOAD_STATE_SEND_INITIATE_RESP,      /*!< wait to send initiate response */
  BOOTLOAD_STATE_RX_IMAGE,                /*!< during receiving OTA data messages */
  BOOTLOAD_STATE_SEND_DATA,               /*!< during sending OTA data messages */
  BOOTLOAD_STATE_WAIT_FOR_DATA_ACK,       /*!< wait for ack on data message */
  BOOTLOAD_STATE_REPORT_ERROR,            /*!< wait to send error message */
  BOOTLOAD_STATE_SEND_COMPLETE,           /*!< after sending complete message */
  BOOTLOAD_STATE_WAIT_FOR_COMPLETE_ACK,   /*!< wait for ack on "complete" msg */
  BOOTLOAD_STATE_VALIDATE,                /*!< after sending validate message */
  BOOTLOAD_STATE_SEND_VALIDATE_RESP,      /*!< wait to send validate response */
  BOOTLOAD_STATE_UPDATE,                  /*!< after sending update message */
  BOOTLOAD_STATE_SEND_UPDATE_RESP         /*!< wait to send update response */
};


// *******************************************************************
// Public functions that are called by the application.

/**@brief A function called in the application's heartbeat or tick function.
 *  The function contains basic bootloading state machine and also
 *  manages the bootload timer.
 *
 */
void appBootloadUtilTick(void);

/**@brief Bootload library initialization. The application needs to define
 *  the ports to be used for printing information and for serial bootload.
 *
 *  Note that the bootload port has to be the RS232 port since it needs to 
 *  receive the image via xmodem protocol.
 *
 * @param appPort       Port used for printing information
 * @param bootloadPort  Port used for serial bootloading
 * @param appProfileId  Application profile id used
 */
void appBootloadUtilInit(int8u appPort, int8u bootloadPort, int16u appProfileId);

/**@brief Starts the bootload process on a target node
 *  that is currently running stack/application.  
 *
 *  The source node sends
 *  bootload initiate message to tell the target that it wants to start 
 *  bootloading.  The source node then enters a state waiting for the target 
 *  node to send an initiate_response, which tells the source node whether the
 *  target node is able to participate in the bootload process, along with the 
 *  error code indicating the reason if it is not able.
 * 
 */
void appBootloadUtilStartXModem(void);

/**@brief Sends a query message to gather information about the node(s) 
 *  with specified address within the EMBER_MAX_HOPS radius.
 *
 *  A query message is sent as a broadcast.  The query is generally used 
 *  to gather information regarding nodes in the network, especially the 
 *  eui64 and short id of the node. 
 *
 * @param address the broadcast address that we want to send to.  The valid
 * addresses are EMBER_BROADCAST_ADDRESS, EMBER_SLEEPY_BROADCAST_ADDRESS,
 * and EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS.
 * 
 * @param isJIT   specify whether the query message is being sent via JIT
 * method or not.  The flag is set to TRUE if parent nodes is sending the 
 * query to the child node when the child polls.
 */
void appBootloadUtilSendQuery(int16u address, boolean isJIT);

/**@brief Starts the bootload process on a target node that is currently 
 *  running stack/application.  
 *
 *  The source node sends a bootload initiate message to tell the target 
 *  that it wants to start bootloading.  
 *  The source node then enters a state waiting for the target 
 *  node to send an initiate_response, which tells the source node whether the
 *  target node is able to participate in the bootload process, along with the 
 *  error code indicating the reason if it is not able.
 *
 * @param target  Node short id to be bootloaded
 * 
 */
void appBootloadUtilStartBootload(EmberNodeId target);

/**@brief Validates the image on the EEPROM on a target node.  
 *
 *  The target node can be a short id of a single node or one of
 *  the broadcast addresses.  After sending the message, the source node enters 
 *  a state waiting for the target node to send a validate_response with status
 *  and image information (timestamp and imageinfo) which tells the source node 
 *  whether the target node has valid image on its EEPROM.
 *
 *  If the target address is its own, then halAppBootloaderGetImageData()
 *  is called directly and no message is sent out.
 *
 * @param target  Node short id to be bootloaded or broadcast address if 
 *  wanting to update multiple nodes at the same time.
 * 
 */
void appBootloadUtilValidate(EmberNodeId target);

/**@brief Installs new image (on EEPROM) on a target node.
 *
 *  The target node can be a short id of a single node or one of
 *  the broadcast addresses.  After sending the message, the source node enters 
 *  a state waiting for the target node to send an update_response with status
 *  which tells the source node whether the target node is able to update the
 *  image or not.
 *
 *  If the target address is its own, then halAppBootloaderInstallNewImage()
 *  is called directly and no message is sent out.
 *
 * @param target  Node short id to be bootloaded or broadcast address if 
 *  wanting to update multiple nodes at the same time.
 * 
 */
void appBootloadUtilUpdate(EmberNodeId target);

/**@brief Called in emberIncomingMessageHander() to pass the application
 *  bootload message to the library for further processing.
 *
 */
void appBootloadUtilIncomingMessageHandler(EmberMessageBuffer message);

/**@brief  Called in emberMessageSentHandler() to pass the sent application bootload 
 *  message and status to the library for further processing.
 *
 */
void appBootloadUtilMessageSent(int16u destination, 
                                EmberApsFrame *apsFrame,
                                EmberMessageBuffer message, 
                                EmberStatus status);

// *******************************************************************
// Callback functions used by the application bootload library.

/** @name Callbacks
 * Callback functions used by the application bootload library.
 *@{
 */

/**@brief A callback function called by the library to get the application
 *  version and id.  MSB returned is the id and LSB is the version.
 *
 */
int16u appBootloadUtilGetAppVersionHandler(void);

/**@brief A callback function called by the library to allow the application
 *  to perform any tasks necessary while the node is validating its external
 *  EEPROM image.  
 *
 *  The validating process can take around ten seconds.  Note
 *  that while the node is validating EEPROM image, it continues to receive
 *  messages over the air.  It is strongly recommended that the application
 *  calls emberTick() within this function in order to service these messages.
 *  Not doing so can lead to the node running out of buffer when it finishes
 *  validating the image.
 *
 */
void appBootloadUtilValidatingEEPROMImageHandler(void);

/** @} END Callbacks */


/** @} END addtogroup */

