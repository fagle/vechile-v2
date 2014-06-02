
#ifndef __COMMON_H__
#define __COMMON_H__

#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "serial.h"
#include "packet-buffer.h"
#include "form-and-join.h"
#include "lamp.h"
#include "spi.h"
#include "gateway.h"
#include "fragment.h"
#include "ami-inter-pan.h"

#ifdef USE_BOOTLOADER_LIB
  #include "bootload-utils.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
// End application specific constants and globals
// 
/////////////////////////////////////////////////////////////////////////////////////////////
// common utility functions
void halButtonIsr               ( int8u button, int8u state ); 
void sensorCommonSetupSecurity  ( void );
void printEUI64                 ( EmberEUI64 * eui );
void printExtendedPanId         ( int8u * extendedPanId );
void printNodeInfo              ( void );
void printTokens                ( void );
void printChildTable            ( void );

#if EMBER_SECURITY_LEVEL == 5
void sensorCommonPrintKeys      ( void );
#endif //EMBER_SECURITY_LEVEL == 5
void sensorCommonPrint16ByteKey ( int8u * key );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printLedLampToken(void)
//* 功能        : 打印token_ledlamp
//* 输入参数    : 
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void printLedLampToken ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void initLedLampToken(void)
//* 功能        : 打印token_ledlamp
//* 输入参数    : 
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void initLedLampToken    ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void delay ( u16 times )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void delay ( u16 times );

/////////////////////////////////////////////////////////////////////////////////////////////
// The following function are to support sleepy end devices
// adds a JIT (just-in-time) message of type msgType for all current children.
void appAddJitForAllChildren ( int8u msgType, int8u* data, int8u dataLength );

/////////////////////////////////////////////////////////////////////////////////////////////
// construct and send an APS message based on a stored JIT message
// this is called from emberPollHandler when transmitExpected is TRUE
void appSendJitToChild ( EmberNodeId childId );

/////////////////////////////////////////////////////////////////////////////////////////////
// print the variables that keep track of the JIT messages.
// (primarily used for debugging)
void jitMessageStatus  ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_BOOTLOADER_LIB
// ****************************************************************
// the following functions are defined to support the em250 standalone 
// bootloader in the sensor sample application. 
// 
// The sensor application only makes use of passthru bootloading and does not 
// use clone or recovery bootload modes. The solution presented is not a 
// complete solution as the command line interface on the applications only 
// supports bootloading the first child in the child table or first entry in 
// the address table. (This could be solved with either an extension to the
// command line interface, or a different model for determining the EUI to
// be bootloaded). This is meant as a reference to show what is needed
// to support bootloading in an existing application. All code related to 
// bootloading is defined under USE_BOOTLOADER_LIB.
//
// Please see the standalone-bootloader-demo sample application for a 
// complete example of using the bootloader.
// called by a parent when a child should be bootloaded. The Bootload does
// not start until the next time the child wakes up.
void bootloadMySleepyChild ( EmberEUI64 targetEui );

/////////////////////////////////////////////////////////////////////////////////////////////
// called by a non-sleepy to bootload a neighbor. The bootload will attempt 
// to start immediately.
void bootloadMyNeighborRouter ( EmberEUI64 targetEui );

/////////////////////////////////////////////////////////////////////////////////////////////
// utility to determine if a device(based on EUI) is a child or not.
boolean isMyChild ( EmberEUI64 candidateEui, int8u* childIndex );

/////////////////////////////////////////////////////////////////////////////////////////////
// utility to determine if a device(based on EUI) is a neighbor or not
boolean isMyNeighbor ( EmberEUI64 eui );

#endif //USE_BOOTLOADER_LIB

/////////////////////////////////////////////////////////////////////////////////////////////
extern boolean buttonZeroPress;
extern boolean buttonOnePress;

/////////////////////////////////////////////////////////////////////////////////////////////
#endif   //  __COMMON_H__

