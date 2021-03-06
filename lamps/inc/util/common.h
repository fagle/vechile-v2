// File: common.h
// 
// Description: declarations for common app code.
//
// Author(s): Richard Kelsey, kelsey@ember.com
//
// Copyright 2004 by Ember Corporation. All rights reserved.                *80*

extern int8u serialPort;
extern SerialBaudRate serialBaudRate;

// For identifying ourselves over the serial port.
extern PGM_NO_CONST PGM_P applicationString;

// Boilerplate
void configureSerial(int8u port, SerialBaudRate rate);
void toggleBlinker(void);

#if !defined EZSP_HOST
  extern EmberEventControl blinkEvent;
#endif

void initialize(void);
void run(EmberEventData* events,
         void (* heartbeat)(void));

// Utilities
void printLittleEndianEui64(int8u port, EmberEUI64 eui64);
void printBigEndianEui64(int8u port, EmberEUI64 eui64);
#define printEui64(port, eui64) printBigEndianEui64((port), (eui64))
void printHexByteArray(int8u port, int8u *byteArray, int8u length);
#define printEncryptionKey(port, keyData) \
  printHexByteArray(port, keyData, EMBER_ENCRYPTION_KEY_SIZE)

int8u asciiHexToByteArray(int8u *bytesOut, int8u* asciiIn, int8u asciiCharLength);

void hexToEui64(int8u *contents, int8u length, EmberEUI64 returnEui64);
void hexToKeyData(int8u *contents, int8u length, EmberKeyData* returnKeyData);
int8u hexDigitValue(int8u digit);

// Host / Non-host specific implementations
boolean setSecurityLevel(int8u level);
boolean generateRandomBytes(int8u* result, int8u length);
boolean getNetworkParameters(EmberNodeType* nodeType, 
                             EmberNetworkParameters* networkParams);
void initializeEmberStack(void);
EmberStatus getOnlineNodeParameters(int8u* childCountReturn, 
                                    int8u* routerCountReturn,
                                    EmberNodeType* nodeTypeReturn,
                                    EmberNodeId* parentNodeIdReturn,
                                    EmberEUI64 parentEuiReturn,
                                    EmberNetworkParameters* networkParamReturn);
EmberStatus getOfflineNodeParameters(EmberNodeId *myNodeIdReturn,
                                     EmberNodeType *myNodeTypeReturn,
                                     int8u* stackProfileReturn);
void runEvents(EmberEventData* events);

#define generateRandomKey(result) \
  (EMBER_SUCCESS == emberGenerateRandomKey(result))

void getOrGenerateKey(int8u argumentIndex, EmberKeyData *key);

void createMulticastBinding(int8u index, int8u *multicastGroup, int8u endpoint);
boolean findEui64Binding(EmberEUI64 key, int8u *index);
void printCommandStatus(EmberStatus status, PGM_P good, PGM_P bad);
void setCommissionParameters(void);
EmberStatus joinNetwork(void);


// Common commands
void helpCommand(void);
void statusCommand(void);
void stateCommand(void);
void rebootCommand(void);
void radioCommand(void);
void formNetworkCommand(void);
void setExtPanIdCommand(void);
void setJoinMethod(void);
void joinNetworkCommand(void);
void rejoinCommand(void);
void networkInitCommand(void);
void leaveNetworkCommand(void);
void addressRequestCommand(void);
void permitJoiningCommand(void);
void setSecurityCommand(void);

void printCarriageReturn(void);
void printErrorMessage(PGM_P message);

#define COMMON_COMMANDS                                                 \
  { "status",       statusCommand,          ""},                    \
  { "state",        stateCommand,           ""},                    \
  { "reboot",       rebootCommand,          ""},                    \
                                                                        \
  /* Parameters: security bitmask, preconfigured key, network key, */   \
  /*   network key sequence */                                          \
  /* If security bitmask is 0x8000 then security is turned off, */      \
  /*   nothing else is set. */                                          \
  /* If security bitmask is 0xFFFF then security is turned on  */       \
  /*   without setting any other security settings. */                  \
  /* If security bitmask is anything else, security is turned on, */    \
  /*   the bitmask is set and so are the other values */                \
  /*   (as appropriate for the bitmask) */                              \
  { "set_security", setSecurityCommand,     "vbbu"},      \
                                                                \
  /* channel, pan id, tx power */                               \
  { "form",         formNetworkCommand,     "uvs"},      \
  /* channel, pan id, tx power, extended pan id */              \
  { "form_ext",     formNetworkCommand,     "uvsb"},            \
                                                                \
  /* extended pan id */                                         \
  { "set_ext_pan",  setExtPanIdCommand,     "b"},               \
                                                                \
  /* Set the join method: MAC Associate (0), */                 \
  /*   NWK Rejoin w/out security (1), */                        \
  /*   NWK Rejoin with Security (2) */                          \
  /*   NWK commission, i.e. join quietly (3) */                 \
  { "join_method",  setJoinMethod,          "u" },              \
                                                                \
  /* Set the Commissioning parameters: */                       \
  /*   coordinator boolean (1 byte) */                          \
  /*   nwkManagerId (2 bytes) */                                \
  /*   nwkUpdateId  (1 byte) */                                 \
  /*   channels  (lower 16-bits of 32-bit value) */             \
  /*   channels  (upper 16-bits of 32-bit value) */             \
  /*   trust center eui64 */                                    \
  { "commission", setCommissionParameters, "uvuvvb" },           \
                                                                \
  /* channel, pan id, tx power */                               \
  { "join",         joinNetworkCommand,     "uvs"},    \
  { "join_end",     joinNetworkCommand,     "uvs"},    \
  { "join_sleepy",  joinNetworkCommand,     "uvs"},    \
  { "join_mobile",  joinNetworkCommand,     "uvs"},    \
                                                                \
  { "nwk_init",     networkInitCommand,     ""},                \
  { "leave",        leaveNetworkCommand,    ""},                \
                                                                \
  /* have current network key (secure vs. insecure) */          \
  { "rejoin",      rejoinCommand,           "u"},               \
                                                                \
  /* target network id, include kids */                         \
  { "ieee_address", addressRequestCommand,  "vu"},              \
                                                                \
  /* target EUI64, include kids */                              \
  { "nwk_address",  addressRequestCommand,  "bu"},              \
                                                                \
  /* duration in seconds, 0 to prohibit, 0xff to allow */       \
  { "permit_joins", permitJoiningCommand,   "u"},          \
                                                                \
  /* help commands */                                           \
  { "help",         helpCommand,            "",},
