#ifndef __CONFIG_H__
#define __CONFIG_H__

/////////////////////////////////////////////////////////////////////////////////////
//
#include  <stdio.h>	
#include  <stdlib.h>
#include  <string.h>

#include "stm32f10x.h" 	            // CPU最底层操作库函数所有头文件 

/////////////////////////////////////////////////////////////////////////////////////
//
#define DEBUG_PRINTF                // sea_printf output 
//#define GPRS_DEBUG                  //output GPRS work message when debug
//#define BIG_ENDIAN                  // frame handler
//#define WATCH_DOG                   // watch dog define

//#define MSGSEND_ENABLE              // enable send telemassage to a staff if lamps open/close.
//#define ADC_ENABLE                  // adc converter enable
//#define KEY_ENABLE                  // key enable
//#define LCD_ENABLE                  // lcd display enable
//#define PCF8365T_ENABLE             // enable pcf8365t       
#define LWIP_ENABLE                 // enable/disable
//#define MACINT_ENABLE
//#ifdef CARD_ENABLE                  // card check enable

//#define LANDSCAPE_MODE              // 景观灯控制
#define VEHICLE_MODE                // vehicle mode
//#define SHRIMP_MODE                 // 虾米养殖环境监测 
//#define ARC_MODE                    // air condition controller
//#define WIFI_MODE

//#define VEHICLE_RELEASE             // release/debug control

/////////////////////////////////////////////////////////////////////////////////////
//
#endif  //  __CONFIG_H__
