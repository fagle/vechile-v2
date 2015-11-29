//////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef __CONFIG_H__
#define __CONFIG_H__

//////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef  __cplusplus
extern "C" {
#endif
  
//////////////////////////////////////////////////////////////////////////////////////////////
//
//#define NULL_BTL                                   // 是否用em3xx-converter生成btl文件
#define CORTEXM3
//#define CORTEXM3_STM32W108
//#define CORTEXM3_EM357
#define PHY_EM3XX
#define BOARD_MB851
#define __SOURCEFILE__                   "$FILE_FNAME$"

#define APP_SERIAL                       (0x01)     // serial ports (port)
#define EMBER_SERIAL1_MODE                EMBER_SERIAL_FIFO  // EMBER_SERIAL_BUFFER  // EMBER_SERIAL_FIFO
#define EMBER_SECURITY_LEVEL              5         // 安全等级（0-5）
#define EMBER_MAXDEVS                     (250)     // for ack 32 bytes( 300) // max. devices in the ember network
#define MOBILEDEVS                        (50)      // max. devices of mobile vehicle
#define CALLERDEVS                        (60)      // max. callers
#define MOBILEBASE                        (EMBER_MAXDEVS - MOBILEDEVS)
#define MAXCARDS                          (0x80)    // max. cards on the road
  
//////////////////////////////////////////////////////////////////////////////////////////////
// 基本配置  
//#define DISABLE_WATCHDOG  
//#define ENERGY_LAMP                 //num 18 lamp version
//#define SINK_APP                    // 协调器容许
//#define MOBILE_APP  
//#define EMZ3118                   // 大功率模块配置
//#define RANGE_TEST                // 距离测试配置  
//#define RF_TEST                   // 射频测试
//#define ENABLE_GATEWAY
#define ENABLE_VEHICLE              // vehicle configuration
#define EMZ3118B                    // 新大功率模块配置,2013/12
#define VEHICLE_RELEASE            // debug/release version, 2014/01/12
    
#ifdef SINK_APP
  #define USE_HARDCODED_NETWORK_SETTINGS  // it is not recommended to ever hard code the network settings applications should always scan 
                                          // to determine the channel and panid to use. However, in some instances, this can aid in debugging       
#else     // SENSOR_APP
  #define SENSOR_APP
//  #define USE_HARDCODED_NETWORK_SETTINGS  // it is not recommended to ever hard code the network settings . should always scan 
#endif
  
//////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x_type.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef BOARD_MB851
#include "MB851.h"
#endif
#ifdef BOARD_MB850
#include "MB850.h"
#endif
#include "iar.h"
#include "ledlamp-configuration.h"    
  
//////////////////////////////////////////////////////////////////////////////////////////////
//
enum { SOFTWARE_VERSION = 0x4032, };
  
//////////////////////////////////////////////////////////////////////////////////////////////
// functions
#define sea_abs(x)            ((x < 0x00) ? (-x) : (x))
#define sea_fabs(x)           ((x < 0.00) ? (x * (-1.00)) : (x))
#define sea_pu(x)             ((sea_fabs(x) > 1.00) ? ((x > 0.00) ? (x - (int)x) : (x + (int)x)) : (x))
#define sea_sign(x,y)         ((x < 0x00) ? (-y) : (y))
#define sea_fsign(x,y)        ((x < 0.00) ? (-y) : (y))
#define sea_swap(x,y)         { int temp; temp = x; x = y; y = temp; }

#define sea_ntos(n)           (((n >> 0x08) & 0xff) | ((n & 0xff) << 0x08))
#define sea_ntol(n)           (((n >> 0x18) & 0xff) | ((n & 0xff) << 0x18) | ((n >> 0x08) & 0xff00) | ((n & 0xff00) << 0x08))
#define sea_short(x,y)        ((((unsigned char)y & 0xff) << 0x08) | ((unsigned char)x & 0xff))  
#define sea_long(a,b,c,d)     ((unsigned long)sea_short(c,d) << 0x10 | (unsigned long)sea_short(a,b))  

#define sea_tmp(v)            (u16)(-39.66 + 0.01 * v) 
#define sea_hum(v)            (u16)(-4.0 + 0.0405 * v - 0.0000028 * v * v)

//#define  USE_RS485
  
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////
#endif	//__CONFIG_H__
