/** 
 * @file token-stack.h
 * @brief Definitions for stack tokens.
 * See @ref token_stack for documentation.
 *
 * The file token-stack.h should not be included directly. 
 * It is accessed by the other token files. 
 *
 * <!--Brooks Barrett-->
 * <!-- Copyright 2005 by Ember Corporation. All rights reserved. -->
 */

/**
 * @addtogroup token_stack
 *
 * The tokens listed here are divided into three sections (the three main
 * types of tokens mentioned in token.h):
 * - manufacturing
 * - stack
 * - application
 *
 * For a full explanation of the tokens, see hal/micro/token.h. 
 * See token-stack.h for source code. 
 *
 * There is a set of tokens predefined in the APPLICATION DATA section at the
 * end of token-stack.h because these tokens are required by the stack, 
 * but they are classified as application tokens since they are sized by the 
 * application via its CONFIGURATION_HEADER.
 * 
 * The user application can include its own tokens in a header file similar
 * to this one. The macro ::APPLICATION_TOKEN_HEADER should be defined to equal
 * the name of the header file in which application tokens are defined. 
 * See the APPLICATION DATA section at the end of token-stack.h 
 * for examples of token definitions.
 * 
 * Since token-stack.h contains both the typedefs and the token defs, there are
 * two \#defines used to select which one is needed when this file is included.
 * \#define DEFINETYPES is used to select the type definitions and
 * \#define DEFINETOKENS is used to select the token definitions.
 * Refer to token.h and token.c to see how these are used.
 *
 * @{
 */

#ifndef DEFINEADDRESSES
/** 
 * @brief By default, tokens are automatically located after the previous token.
 *
 * If a token needs to be placed at a specific location,
 * one of the DEFINE_FIXED_* definitions should be used.  This macro is
 * inherently used in the DEFINE_FIXED_* definition to locate a token, and
 * under special circumstances (such as manufacturing tokens) it may be
 * explicitely used.
 * 
 * @param region   A name for the next region being located.
 * @param address  The address of the beginning of the next region.
 */
  #define TOKEN_NEXT_ADDRESS(region, address)
#endif


// The basic TOKEN_DEF macro should not be used directly since the simplified
//  definitions are safer to use.  For completeness of information, the basic
//  macro has the following format:
//
//  TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...)
//  name - The root name used for the token
//  creator - a "creator code" used to uniquely identify the token
//  iscnt - a boolean flag that is set to identify a counter token
//  isidx - a boolean flag that is set to identify an indexed token
//  type - the basic type or typdef of the token
//  arraysize - the number of elements making up an indexed token
//  ... - initializers used when reseting the tokens to default values
//
//
// The following convenience macros are used to simplify the definition
//  process for commonly specified parameters to the basic TOKEN_DEF macro
//  DEFINE_BASIC_TOKEN(name, type, ...)
//  DEFINE_INDEXED_TOKEN(name, type, arraysize, ...)
//  DEFINE_COUNTER_TOKEN(name, type, ...)
//  DEFINE_FIXED_BASIC_TOKEN(name, type, address, ...)
//  DEFINE_FIXED_INDEXED_TOKEN(name, type, arraysize, address, ...)
//  DEFINE_FIXED_COUNTER_TOKEN(name, type, address, ...)
//  DEFINE_MFG_TOKEN(name, type, address, ...)
//  

/**
 * @name Convenience Macros
 * @brief The following convenience macros are used to simplify the definition
 * process for commonly specified parameters to the basic TOKEN_DEF macro.
 * Please see hal/micro/token.h for a more complete explanation.
 *@{
 */
#define DEFINE_BASIC_TOKEN(name, type, ...)  \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1,  __VA_ARGS__)

#define DEFINE_COUNTER_TOKEN(name, type, ...)  \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1,  __VA_ARGS__)

#define DEFINE_INDEXED_TOKEN(name, type, arraysize, ...)  \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize),  __VA_ARGS__)

#define DEFINE_FIXED_BASIC_TOKEN(name, type, address, ...)  \
  TOKEN_NEXT_ADDRESS(name,(address))                          \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1,  __VA_ARGS__)

#define DEFINE_FIXED_COUNTER_TOKEN(name, type, address, ...)  \
  TOKEN_NEXT_ADDRESS(name,(address))                            \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1,  __VA_ARGS__)

#define DEFINE_FIXED_INDEXED_TOKEN(name, type, arraysize, address, ...)  \
  TOKEN_NEXT_ADDRESS(name,(address))                                       \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize),  __VA_ARGS__)

#define DEFINE_MFG_TOKEN(name, type, address, ...)  \
  TOKEN_NEXT_ADDRESS(name,(address))                  \
  TOKEN_MFG(name, CREATOR_##name, 0, 0, type, 1,  __VA_ARGS__)
  
/** @} END Convenience Macros */


// The eeprom test uses its own, permanent token definitions.
#ifndef SIM_EEPROM_TEST

// The creator codes are here in one list instead of next to their token
// definitions so comparision of the codes is easier.  The only requirement
// on these creator definitions is that they all must be unique.  A favorite
// method for picking creator codes is to use two ASCII characters inorder
// to make the codes more memorable.

/**
 * @name Creator Codes
 * @brief The CREATOR is used as a distinct identifier tag for the
 * token.  
 *
 * The CREATOR is necessary because the token name is defined
 * differently depending on the hardware platform, therefore the CREATOR makes
 * sure that token definitions and data stay tagged and known.  The only
 * requirement is that each creator definition must be unique.  Please
 * see hal/micro/token.h for a more complete explanation.
 *@{
 */
 
// STACK CREATORS
#define CREATOR_STACK_NVDATA_VERSION             0xFF01
#define CREATOR_STACK_BOOT_COUNTER               0xE263
#define CREATOR_STACK_NONCE_COUNTER              0xE563
#define CREATOR_STACK_ANALYSIS_REBOOT            0xE162
#define CREATOR_STACK_KEYS                       0xEB79
#define CREATOR_STACK_NODE_DATA                  0xEE64
#define CREATOR_STACK_CLASSIC_DATA               0xE364
#define CREATOR_STACK_ALTERNATE_KEY              0xE475
#define CREATOR_STACK_APS_FRAME_COUNTER          0xE123
#define CREATOR_STACK_TRUST_CENTER               0xE124
#define CREATOR_STACK_NETWORK_MANAGEMENT         0xE125
// APP CREATORS
#define CREATOR_STACK_BINDING_TABLE              0xE274
#define CREATOR_STACK_CHILD_TABLE                0xFF0D
#define CREATOR_STACK_KEY_TABLE                  0xE456
#define CREATOR_STACK_CERTIFICATE_TABLE          0xE500

/** @} END Creator Codes  */


//////////////////////////////////////////////////////////////////////////////
// MANUFACTURING DATA
// Since the manufacturing data is platform specific, we pull in the proper
// file here.
#if defined(AVR_ATMEGA)
  #include "hal/micro/avr-atmega/token-manufacturing.h"
#elif defined(MSP430)
  #include "hal/micro/msp430/token-manufacturing.h"
#elif defined(XAP2B)
  #include "hal/micro/xap2b/token-manufacturing.h"
#elif defined(CORTEXM3)
  #include "token-manufacturing.h"
#elif defined(EMBER_TEST)
  #include "hal/micro/avr-atmega/token-manufacturing.h"
#else
  #error no platform defined
#endif


//////////////////////////////////////////////////////////////////////////////
// STACK DATA
// *the addresses of these tokens must not change*

/**
 * @brief The current version number of the stack tokens.
 * MSB is the version, LSB is a complement.  
 *
 * Please see hal/micro/token.h for a more complete explanation.
 */
#define CURRENT_STACK_TOKEN_VERSION 0x03FC //MSB is version, LSB is complement

#ifdef DEFINETYPES
typedef int16u tokTypeStackNvdataVersion;
typedef int16u tokTypeStackBootCounter;
typedef int16u tokTypeStackAnalysisReboot;
typedef int32u tokTypeStackNonceCounter;
typedef struct 
{
  int8u networkKey[16];
  int8u activeKeySeqNum;
} tokTypeStackKeys;

typedef struct 
{
  int16u panId;
  int8s radioTxPower;
  int8u radioFreqChannel;
  int8u stackProfile;
  int8u nodeType;
  int16u zigbeeNodeId;
  int8u extendedPanId[8];
} tokTypeStackNodeData;

typedef struct 
{
  int8u gradEnableRelay;
  int8u encryptionKey[16];
  int8u idAssignmentEnabled;
  int16u gradNodeId;
  int8u idAuthorityTag[6];
} tokTypeStackClassicData;

typedef struct 
{
  int16u mode;
  int8u eui64[8];
  int8u key[16];
} tokTypeStackTrustCenter;

typedef struct 
{
  int32u activeChannels;
  int16u managerNodeId;
  int8u updateId;
} tokTypeStackNetworkManagement;
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// The Stack tokens also need to be stored at well-defined locations
//  None of these addresses should ever change without extremely great care
#define STACK_VERSION_LOCATION    128 //  2   bytes
#define STACK_APS_NONCE_LOCATION  130 //  4   bytes
#define STACK_ALT_NWK_KEY_LOCATION 134 // 17  bytes (key + sequence number)
// reserved                       151     1   bytes
#define STACK_BOOT_COUNT_LOCATION 152 //  2   bytes
// reserved                       154     2   bytes
#define STACK_NONCE_LOCATION      156 //  4   bytes
// reserved                       160     1   bytes
#define STACK_REBOOT_LOCATION     161 //  2   bytes
// reserved                       163     7   bytes
#define STACK_KEYS_LOCATION       170 //  17  bytes
// reserved                       187     5   bytes
#define STACK_NODE_DATA_LOCATION  192 //  16  bytes
#define STACK_CLASSIC_LOCATION    208 //  26  bytes
#define STACK_TRUST_CENTER_LOCATION 234 //26  bytes
// reserved                       260     8   bytes
#define STACK_NETWORK_MANAGEMENT_LOCATION 268
                                      //  7   bytes
// reserved                       275     109 bytes

DEFINE_FIXED_BASIC_TOKEN(STACK_NVDATA_VERSION, 
                         tokTypeStackNvdataVersion,
                         STACK_VERSION_LOCATION,
                         CURRENT_STACK_TOKEN_VERSION)
DEFINE_FIXED_COUNTER_TOKEN(STACK_APS_FRAME_COUNTER,
                           tokTypeStackNonceCounter,
                           STACK_APS_NONCE_LOCATION,
                           0x00000000)
DEFINE_FIXED_BASIC_TOKEN(STACK_ALTERNATE_KEY,
                         tokTypeStackKeys,
                         STACK_ALT_NWK_KEY_LOCATION,
                         {0,})
DEFINE_FIXED_COUNTER_TOKEN(STACK_BOOT_COUNTER,
                           tokTypeStackBootCounter,
                           STACK_BOOT_COUNT_LOCATION,
                           0x0000)
DEFINE_FIXED_COUNTER_TOKEN(STACK_NONCE_COUNTER,
                           tokTypeStackNonceCounter,
                           STACK_NONCE_LOCATION,
                           0x00000000)
DEFINE_FIXED_BASIC_TOKEN(STACK_ANALYSIS_REBOOT,
                         tokTypeStackAnalysisReboot,
                         STACK_REBOOT_LOCATION,
                         0x0000)
DEFINE_FIXED_BASIC_TOKEN(STACK_KEYS,
                         tokTypeStackKeys,
                         STACK_KEYS_LOCATION,
                         {0,})
DEFINE_FIXED_BASIC_TOKEN(STACK_NODE_DATA,
                         tokTypeStackNodeData,
                         STACK_NODE_DATA_LOCATION,
                         {0xFFFF,-1,0,0x00,0x00,0x0000})
DEFINE_FIXED_BASIC_TOKEN(STACK_CLASSIC_DATA,
                         tokTypeStackClassicData,
                         STACK_CLASSIC_LOCATION,
                         {TRUE,{0,0,0,0,0,0,0,0},TRUE,0xFFFF,{0,0,0,0,0,0}})
DEFINE_FIXED_BASIC_TOKEN(STACK_TRUST_CENTER,
                         tokTypeStackTrustCenter,
                         STACK_TRUST_CENTER_LOCATION,
                         {0,})
DEFINE_FIXED_BASIC_TOKEN(STACK_NETWORK_MANAGEMENT,
                         tokTypeStackNetworkManagement,
                         STACK_NETWORK_MANAGEMENT_LOCATION,
                         {0, 0xFFFF, 0})
#endif //DEFINETOKENS


//////////////////////////////////////////////////////////////////////////////
// APPLICATION DATA
// *If a fixed application token is desired, its address must be above 384.*

#ifdef DEFINETYPES
typedef int8u tokTypeStackBindingTable[13];
typedef int8u tokTypeStackChildTable[11];
typedef int8u tokTypeStackKeyTable[25];
// Certificate Table Entry
//   Certificate:    48-bytes
//   CA Public Key:  22-bytes
//   Private Key:    21-bytes
//   Flags:          1-byte
#define TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE (48 + 22 + 21 + 1)
#define TOKEN_CERTIFICATE_TABLE_ENTRY_FLAGS_INDEX (TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE - 1)
typedef int8u tokTypeStackCertificateTable[TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE];
#endif //DEFINETYPES

// The following application tokens are required by the stack, but are sized by 
//  the application via its CONFIGURATION_HEADER, which is why they are present
//  within the application data section.  Any special application defined
//  tokens will follow.  
// NOTE: changing the size of these tokens within the CONFIGURATION_HEADER
//  WILL move automatically move any custom application tokens that are defined
//  in the APPLICATION_TOKEN_HEADER
#ifdef DEFINETOKENS
// Application tokens start at location 384 and are automatically positioned.
TOKEN_NEXT_ADDRESS(APP,384)
DEFINE_INDEXED_TOKEN(STACK_BINDING_TABLE,
                     tokTypeStackBindingTable,
                     EMBER_BINDING_TABLE_TOKEN_SIZE,
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_CHILD_TABLE,
                     tokTypeStackChildTable,
                     EMBER_CHILD_TABLE_TOKEN_SIZE,
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_KEY_TABLE,
                     tokTypeStackKeyTable,
                     EMBER_KEY_TABLE_TOKEN_SIZE,
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_CERTIFICATE_TABLE,
                     tokTypeStackCertificateTable,
                     EMBER_CERTIFICATE_TABLE_SIZE,
                     {0,})
#endif //DEFINETOKENS


#ifdef APPLICATION_TOKEN_HEADER
  #include APPLICATION_TOKEN_HEADER
#endif


//The tokens defined below are test tokens.  They are normally not used by
//anything but are left here as a convenience so test tokens do not have to
//be recreated.  If test code needs temporary, non-volatile storage, simply
//uncomment and alter the set below as needed.
//#define CREATOR_TT01 1
//#define CREATOR_TT02 2
//#define CREATOR_TT03 3
//#define CREATOR_TT04 4
//#define CREATOR_TT05 5
//#define CREATOR_TT06 6
//#ifdef DEFINETYPES
//typedef int32u tokTypeTT01;
//typedef int32u tokTypeTT02;
//typedef int32u tokTypeTT03;
//typedef int32u tokTypeTT04;
//typedef int16u tokTypeTT05;
//typedef int16u tokTypeTT06;
//#endif //DEFINETYPES
//#ifdef DEFINETOKENS
//#define TT01_LOCATION 1
//#define TT02_LOCATION 2
//#define TT03_LOCATION 3
//#define TT04_LOCATION 4
//#define TT05_LOCATION 5
//#define TT06_LOCATION 6
//DEFINE_FIXED_BASIC_TOKEN(TT01, tokTypeTT01, TT01_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT02, tokTypeTT02, TT02_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT03, tokTypeTT03, TT03_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT04, tokTypeTT04, TT04_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT05, tokTypeTT05, TT05_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT06, tokTypeTT06, TT06_LOCATION, 0x0000)
//#endif //DEFINETOKENS

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////
//token_ledlamp
#define CREATOR_LEDLAMP_PARAMETERS  0x5354
#define CREATOR_LEDLAMP_STAGE       0x5350

#ifdef DEFINETYPES
typedef struct
{
  int16u roadId;
  int16u lampNum;
  int8u  rptPeriod;  
  int32u openHold;
} tokTypeLedLampParameters;

typedef struct
{
  int16u hold;
  int16u open;
} sea_stage_t;

typedef struct
{
  int32u stage[MAXSTAGE];
} tokTypeLedLampStage;
#endif


#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(LEDLAMP_PARAMETERS,tokTypeLedLampParameters,{ROADID,DEFALUTLAMPNUM,DEFALUTPERIOD})
DEFINE_INDEXED_TOKEN(LEDLAMP_STAGE,tokTypeLedLampStage,MAXSTAGE,{0,})
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
#if 0

#define CREATOR_SENSOR_NAME        0x5354
#define CREATOR_SENSOR_PARAMETERS  0x5350

#ifdef DEFINETYPES
typedef int8u tokTypeSensorName[10];
typedef struct 
{
  int8u initValues[5];
  int8u reportInterval;
  int16u calibrationValue;
} tokTypeSensorParameters;
#endif 

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(SENSOR_NAME,tokTypeSensorName,{'U','N','A','M','E','D',' ',' ',' ',' '})
DEFINE_BASIC_TOKEN(SENSOR_PARAMETERS,tokTypeSensorParameters,{{0x01,0x02,0x03,0x04,0x05},5,0x0000})
#endif 

#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//

#else // ifndef SIM_EEPROM_TEST

//****************************************************************
// THE REST OF THIS FILE IS ONLY USED FOR TESTING SIMULATED EEPROM.
// MAKE CHANGES ONLY IF YOU ARE CHANGING THOSE TESTS.
//****************************************************************

// The eeprom test uses its own, permanent token definitions.

// STACK CREATORS
#define CREATOR_STACK_NVDATA_VERSION             0xFF01
#define CREATOR_STACK_BOOT_COUNTER               0xE263
#define CREATOR_STACK_NONCE                      0xE564
#define CREATOR_STACK_ANALYSIS_REBOOT            0xE162
#define CREATOR_STACK_KEYS                       0xEB79
#define CREATOR_STACK_NODE_DATA                  0xEE64
#define CREATOR_STACK_CLASSIC_DATA               0xE364
// APP CREATORS
#define CREATOR_STACK_BINDING_TABLE              0xE274
#define CREATOR_STACK_CHILD_TABLE                0xFF0D
#define CREATOR_STACK_DISCOVERY_CACHE            0xE463
#define CREATOR_STACK_APS_INDIRECT_BINDING_TABLE 0xE474

#if defined(AVR_ATMEGA)
  #include "hal/micro/avr-atmega/token-manufacturing.h"
#elif defined(MSP430)
  #include "hal/micro/msp430/token-manufacturing.h"
#elif defined(XAP2B)
  #include "hal/micro/xap2b/token-manufacturing.h"
#elif defined(EMBER_TEST)
  #include "hal/micro/avr-atmega/token-manufacturing.h"
#else
  #error no platform defined
#endif

#define CURRENT_STACK_TOKEN_VERSION 0x03FC //MSB is version, LSB is complement

#ifdef DEFINETYPES

typedef int16u tokTypeStackNvdataVersion;
typedef int16u tokTypeStackBootCounter;
typedef int16u tokTypeStackAnalysisReboot;
typedef int32u tokTypeStackNonce;
typedef struct 
{
  int8u networkKey[16];
  int8u activeKeySeqNum;
} tokTypeStackKeys;

typedef struct 
{
  int16u panId;
  int8s radioTxPower;
  int8u radioFreqChannel;
  int8u stackProfile;
  int8u nodeType;
  int16u zigbeeNodeId;
} tokTypeStackNodeData;

typedef struct 
{
  int8u gradEnableRelay;
  int8u encryptionKey[16];
  int8u idAssignmentEnabled;
  int16u gradNodeId;
  int8u idAuthorityTag[6];
} tokTypeStackClassicData;
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// The Stack tokens also need to be stored at well-defined locations
//  None of these addresses should ever change without extremely great care
#define STACK_VERSION_LOCATION    128 //  2   bytes
// reserved                       130     22  bytes
#define STACK_BOOT_COUNT_LOCATION 152 //  2   bytes
// reserved                       154     2   bytes
#define STACK_NONCE_LOCATION      156 //  4   bytes
// reserved                       160     1   bytes
#define STACK_REBOOT_LOCATION     161 //  2   bytes
// reserved                       163     7   bytes
#define STACK_KEYS_LOCATION       170 //  17  bytes
// reserved                       187     5   bytes
#define STACK_NODE_DATA_LOCATION  192 //  8   bytes
// reserved                       200     8   bytes
#define STACK_CLASSIC_LOCATION    208 //  26  bytes
// reserved                       234     150 bytes

DEFINE_FIXED_BASIC_TOKEN(STACK_NVDATA_VERSION, 
                         tokTypeStackNvdataVersion,
                         STACK_VERSION_LOCATION,
                         CURRENT_STACK_TOKEN_VERSION)
DEFINE_FIXED_COUNTER_TOKEN(STACK_BOOT_COUNTER,
                           tokTypeStackBootCounter,
                           STACK_BOOT_COUNT_LOCATION,
                           0x0000)
DEFINE_FIXED_BASIC_TOKEN(STACK_NONCE,
                         tokTypeStackNonce,
                         STACK_NONCE_LOCATION,
                         0x00000000)
DEFINE_FIXED_BASIC_TOKEN(STACK_ANALYSIS_REBOOT,
                         tokTypeStackAnalysisReboot,
                         STACK_REBOOT_LOCATION,
                         0x0000)
DEFINE_FIXED_BASIC_TOKEN(STACK_KEYS,
                         tokTypeStackKeys,
                         STACK_KEYS_LOCATION,
                         {0,})
DEFINE_FIXED_BASIC_TOKEN(STACK_NODE_DATA,
                         tokTypeStackNodeData,
                         STACK_NODE_DATA_LOCATION,
                         {0xFFFF,-1,0,0x00,0x00,0x0000})
DEFINE_FIXED_BASIC_TOKEN(STACK_CLASSIC_DATA,
                         tokTypeStackClassicData,
                         STACK_CLASSIC_LOCATION,
                         {TRUE,{0,0,0,0,0,0,0,0},TRUE,0xFFFF,{0,0,0,0,0,0}})
#endif //DEFINETOKENS

#ifdef DEFINETYPES
typedef int8u tokTypeStackBindingTable[12];
typedef int8u tokTypeStackChildTable[10];
typedef int8u tokTypeStackDiscoveryCache[EMBER_DISCOVERY_CACHE_ENTRY_SIZE];
typedef int8u tokTypeStackApsIndirectBinding[19];
#endif //DEFINETYPES

#ifdef DEFINETOKENS
// Application tokens start at location 384 and are automatically positioned.
TOKEN_NEXT_ADDRESS(APP,384)
DEFINE_INDEXED_TOKEN(STACK_BINDING_TABLE,
                     tokTypeStackBindingTable,
                     (EMBER_BINDING_TABLE_SIZE-EMBER_TEMPORARY_BINDING_ENTRIES),
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_CHILD_TABLE,
                     tokTypeStackChildTable,
                     EMBER_CHILD_TABLE_SIZE,
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_DISCOVERY_CACHE,
                     tokTypeStackDiscoveryCache,
                     EMBER_DISCOVERY_CACHE_SIZE,
                     {0,})
DEFINE_INDEXED_TOKEN(STACK_APS_INDIRECT_BINDING_TABLE,
                     tokTypeStackApsIndirectBinding,
                     EMBER_APS_INDIRECT_BINDING_TABLE_SIZE,
                     {0,})
#endif //DEFINETOKENS

#endif //SIM_EEPROM_TEST

#ifndef DEFINEADDRESSES
  #undef TOKEN_NEXT_ADDRESS
#endif

///////////////////////////////////////////////////////////////////////////////////////////


