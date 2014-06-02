/////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __FRAME_H__
#define __FRAME_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "serial.h"

////////////////////////////////////////////////////////////////////////
//constants and globals
#define FRDATASIZE   (0x40)   // 0x78, // receive data max length, max.devices 500/8 = 62.5
#define FEXTRASIZE   (0x04)   // indclude len(1), cmd(1) and roadid(2)
#define PREFIX       (0x68)   // (0x68) '\r' = 0x0a, '@' = 0x40, '#' = 0x23, '$' = 0x24, '%' = 0x25
#define SUFFIX       (0x43)   // (0x43) '\n' = 0x0d
#define MAXCNT       (0x7f)   // max. of length

#define PRT_TALK    (0x1e)
#define PRT_RESP    (0x96)
#define PRT_DATA    (0x3c)
#define PRT_SUM     (0x2d)
#define PRT_AKC     (0xa5)
#define PRT_IDLE    (0x00)

#define PRT_FAIL    (-1)
#define PRT_TIMEOUT (-2)
#define PRT_ERROR   (-3)
#define PRT_OK      (0x01)
#define PRT_NOERROR (0x00)
    
#define FRAMEBUFLEN (0x04)
#define MAXFRMLEN   (FRDATASIZE + FEXTRASIZE)
    
#define RETRYTIME    0x0030
#define FRPRINTBUF   0x80
  
/////////////////////////////////////////////////////////////////////////////////////////////////
// 上位机—下位机通信帧结构
typedef struct
{
    u8   cmd;  
    u8   len;  
    u16  road;
    u8   body[FRDATASIZE];
} frame_t, *pframe_t;

///////////////////////////////////////////////////////////////////////
typedef struct frm_t
{
    u8              state;
    u8              in;	     
    u8              out;	 
    u8              idx;  
#ifdef FRPRINTBUF
    u8              buf[FRPRINTBUF];
#endif    
    frame_t         list[FRAMEBUFLEN];  
    pframe_t        (*get)      ( struct frm_t * frm, pframe_t ptr );
    void            (*recv)     ( struct frm_t * frm, u8 ch );
    s16             (*put)      ( struct frm_t * frm, u8 cmd, u16 road, u8 len, u8 * str );
    s16             (*putbyte)  ( struct frm_t * frm, u8 cmd, u16 road, u8 value );
    s16             (*putshort) ( struct frm_t * frm, u8 cmd, u16 road, u16 value );
    s16             (*putlong)  ( struct frm_t * frm, u8 cmd, u16 road, u32 value );
    u8              (*sendbyte) ( u8 ch );
#ifdef FRPRINTBUF
    void            (*print)    ( struct frm_t * frm, const char * str, ... );
#endif    
} frame_info_t, *pframe_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////////
//  functions prototypes definition
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_initframe ( pframe_info_t ptr )
//* 功能        : initialize frame information structure body
//* 输入参数    : pframe_info_t ptr,  USART_TypeDef * uart
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_initframe ( pframe_info_t ptr );

////////////////////////////////////////////////////////////////////////
#ifdef  __cplusplus
}
#endif  

#endif  // __FRAME_H__
////////////////////////////////////////////////////////////////////////////////
