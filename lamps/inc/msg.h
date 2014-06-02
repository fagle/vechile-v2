#ifndef __MSG_H__
#define __MSG_H__

#include "stm32f10x_type.h"

#define MAXMSGSIZE   (0x04)
#define UnknowCmd    (0x00)

/////////////////////////////////////////////////////////////////////////////////////////////////
// message type definition
#define MULTICAST    (0x01)
#define UNICAST      (0x02)
#define BROADCAST    (0x03)
#define MULTISTR     (0x04)
#define FRAGMENT     (0x05)
#define INTERPAN     (0x06)

// messge status definition
#define NOTSEND      (0x00)
#define ONLYSEND     (0x01)
#define FEEDBACK     (0x02)
#define SENDFAIL     (0x00)

/////////////////////////////////////////////////////////////////////////////////////////////////
// message body definition
typedef struct
{
    u8     type;
    u8     id;             // note: use different message ids for different node.
    u8     size;           // message body size.
    u16    dest;           // destination short id
    u8     body[32];
} msg_t, *pmsg_t;

////////////////////////////////////////////////////////////////////////////////
// message information definition
typedef struct 
{
    u8     status;                // send command status
    u8     in, out;
    u16    wait;                  // if network is busy, msg queue should wait some main loops
    pmsg_t curmsg;                // current send id
    msg_t  list[MAXMSGSIZE];
} msg_info_t, *pmsg_info_t;

////////////////////////////////////////////////////////////////////////////////
// functions definition
/*******************************************************************************
* Function Name  : void init_msginfo ( void )
* Description    : initialize msg_info structure body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void init_msginfo ( void );

/*******************************************************************************
* Function Name  : void sea_sendmsg ( pmsg_info_t ptr, u8 type, u16 dset, u8 id, u8 size, void * msg )
* Description    : Inserts msg_t body to list.
* Input          : pmsg_info_t ptr, u8 type, u8 id, u16 dset, u8 size, void * msg
* Output         : None
* Return         : None
*******************************************************************************/
void sea_sendmsg ( pmsg_info_t ptr, u8 type, u16 dest, u8 id, u8 size, void * msg );

/*******************************************************************************
* Function Name  : pmsg_t sea_getmsg ( pmsg_info_t ptr )
* Description    : get msg_t body from ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
pmsg_t sea_getmsg ( pmsg_info_t ptr );

/*******************************************************************************
* Function Name  : bool sea_isempty ( pmsg_info_t ptr  )
* Description    : is empty of list.
* Input          : pmsg_info_t ptr 
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_msgisempty ( pmsg_info_t ptr  ); 

/*******************************************************************************
* Function Name  : pmsg_t sea_readmsg ( pmsg_info_t ptr )
* Description    : get body of msg_t ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
pmsg_t sea_readmsg ( pmsg_info_t ptr );

////////////////////////////////////////////////////////////////////////////////
//
extern msg_info_t  send1, fail1; 

////////////////////////////////////////////////////////////////////////////////

#endif // end of __MSG_H__ 


