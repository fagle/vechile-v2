#include <stdio.h>
#include "msg.h"
#include "mem.h"

/////////////////////////////////////////////////////////////////////////////////
// the common data

/*******************************************************************************
* Function Name  : pmsg_t sea_readmsg ( pmsg_info_t ptr )
* Description    : get body of msg_t ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
pmsg_t sea_readmsg ( pmsg_info_t ptr )       // read message not delete message  
{
    if (ptr == NULL)
        return NULL;

    ptr->curmsg = NULL;
    if (ptr->out != ptr->in)
        return ptr->curmsg;
   
    ptr->curmsg = &ptr->list[ptr->out];
    return ptr->curmsg;
}

/*******************************************************************************
* Function Name  : bool sea_isempty ( pmsg_info_t ptr  )
* Description    : is empty of list.
* Input          : pmsg_info_t ptr 
* Output         : boolean
* Return         : None
*******************************************************************************/
bool sea_msgisempty ( pmsg_info_t ptr  )  
{
    if (ptr == NULL)
        return (bool)0x01;

    return (bool)(ptr->out == ptr->in);
}

/*******************************************************************************
* Function Name  : pmsg_t sea_getmsg ( pmsg_info_t ptr )
* Description    : get msg_t body from ring.
* Input          : pmsg_info_t ptr
* Output         : pointer of msg_t
* Return         : None
*******************************************************************************/
pmsg_t sea_getmsg ( pmsg_info_t ptr )        //get message and delete message
{
    if (ptr == NULL)
        return NULL;

    ptr->curmsg = NULL;
    if (ptr->out != ptr->in)
    {
        ptr->curmsg = &ptr->list[ptr->out];
        ptr->out ++;
        if (ptr->out >= MAXMSGSIZE)
            ptr->out = 0x00;
    }
    return ptr->curmsg;
}

/*******************************************************************************
* Function Name  : void sea_sendmsg ( pmsg_info_t ptr, u8 type, u16 dset, u8 id, u8 size, void * msg )
* Description    : Inserts msg_t body to list.
* Input          : pmsg_info_t ptr, u8 type, u8 id, u16 dset, u8 size, void * msg 
* Output         : None
* Return         : None
*******************************************************************************/
void sea_sendmsg ( pmsg_info_t ptr, u8 type, u16 dest, u8 id, u8 size, void * msg )          //the message count must <= 10
{
    if (ptr == NULL)
        return;
    
    ptr->list[ptr->in].type    = type;        //not use
    ptr->list[ptr->in].dest    = dest;
    ptr->list[ptr->in].id      = id;
    ptr->list[ptr->in].size    = size;
    sea_memcpy(ptr->list[ptr->in].body, msg, size);
    
    ptr->in ++;
    if (ptr->in >= MAXMSGSIZE)
        ptr->in = 0x00;
}

/*******************************************************************************
* Function Name  : void init_msginfo ( void )
* Description    : initialize msg_info structure body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void init_msginfo ( void )
{
    sea_memset(&send1, 0x00, sizeof(msg_info_t));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

