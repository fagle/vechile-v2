#ifndef __MEM_UTIL_H__
#define __MEM_UTIL_H__

#include "config.h"

/*******************************************************************************
* Function Name  : void sea_memcpy ( const void * dest, void * src, u16 bytes )
* Description    : A version of memcopy that can handle overlapping source and
                   destination regions.
* Input          : input data: val; number of bytes: bytes
* Output         : destination: dest
* Return         : None
*******************************************************************************/
void sea_memcpy ( const void * dest, void * src, u16 bytes );

/*******************************************************************************
* Function Name  : void sea_memset ( void *dest, u8 val, u16 bytes )
* Description    : Set memory
* Input          : input data: val; number of bytes: bytes
* Output         : destination: dest
* Return         : None
*******************************************************************************/
void sea_memset ( void * dest, u8 val, u16 bytes );

/*******************************************************************************
* Function Name  : s8 sea_memcomp ( const void * str1, const void * str2, u16 bytes )
* Description    : Compare memory
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
s8 sea_memcomp ( const void * str1, const void * str2, u16 bytes );

/*******************************************************************************
* Function Name  : s8 sea_memstr (const void * str1, const void * str2, u16 bytes )
* Description    : Compare two strings
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
s8 sea_memstr ( const void * str1, const void * str2, u16 bytes );

/*******************************************************************************
* Function Name  : s8 sea_memstr (const void * str1, const void * str2, u16 bytes )
* Description    : Compare two strings
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
u16 sea_strlen ( const void * str1 );

#endif // _MEM_UTIL_H__ 

/////////////////////////////////////////////////////////////////////////////////////
