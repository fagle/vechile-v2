
#ifndef __CAN_H__
#define __CAN_H__

//////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////////////
//
#define CANTIMEOUT     (0x1000)
#define CANRETRY       (0x10)
#define CANFRAMELEN    (0x08)
#define TIMEOUT        (0x800)
#define CANSIZE        (0x03)

#define CANSENDID      0x05a5
#define CANRECVID      0x0002
#define STDSENDID      0x01e0
#define STDSENDP0      0x0268
#define STDSENDP1      0x0260

#define CANMAXSPEED    0xf000
#define CANMINSPEED    0x0000
#define CANSTEPSPD     0x20

/////////////////////////////////////////////////////////////////////////////////////////////
// commands types
//
typedef enum { CANNONE = 0x00, CANINIT, CANSTART, CANRUN, CANSTOP } eCmd;

/////////////////////////////////////////////////////////////////////////////////////////////
// types
//
typedef enum 
{
        ESTART     = 0x01,   // the following types of ECOMMAND, 
        EHALT      = 0x10,
        EDIRECTION = 0x08,
        ESPEEDUP   = 0x03,   // the following types of ECONTROL
        ESPEEDDOWN = 0x04,
        EFAN       = 0x01,
        ERELAY     = 0x02 
} eType;

typedef struct
{
    u8  cmd;                       //Forklift operation
    u8  type;                      //work state
    u16 speed;                     //running speed
    u16 left;                     
    u16 right;                     
} canfrm_t, *pcanfrm_t;

typedef struct
{
    u8   status;
    u8   retry;
    u8   in, out;
    u16  speed;
    canfrm_t list[CANSIZE];
} can_info_t, *pcan_info_t;

/*******************************************************************************
* Function Name  : CAN_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_Configuration   ( void );

/*******************************************************************************
* Function Name  : CAN_INIT ( void )
* Description    : initialize can information.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_INIT            ( void );

/*******************************************************************************
* Function Name  : void CAN_RxMessage ( void ) 
* Description    : can receive massage.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_RxMessage       ( void );      // called it from can rx interrupt process

/*******************************************************************************
* Function Name  : void CAN_TxMessage ( frame_t * ptr ) 
* Description    : can send frame massage.
* Input          : frame_t * ptr
* Output         : None
* Return         : None
*******************************************************************************/
//void CAN_TxMessage (  canfrm_t * ptr );

/*******************************************************************************
* Function Name  : void sea_canprint ( u16 id, u8 cmd, u8 type, u16 speed, u16 left, u16 right )
* Description    : can send frame massage.
* Input          : u16 id, u8 cmd, u8 type, u16 speed, u16 left, u16 right
* Output         : None
* Return         : None
*******************************************************************************/
//void sea_canprint ( u16 id, u8 cmd, u8 type, u16 speed, u16 left, u16 right );

/*******************************************************************************
* Function Name  : void sea_canhandler ( u32 ticker )
* Description    : can send frame handler.
* Input          : u32 ticker
* Output         : None
* Return         : None
*******************************************************************************/
void sea_canhandler ( u32 ticker );

//////////////////////////////////////////////////////////////////////////////////
//
extern can_info_t  can_info;

#endif /* __CAN_H__ */