#ifndef __CONSOLE_H__
#define __CONSOLE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAXCMDSIZE        (0x60)
#define MAXARGS           (0x30)
#define CMDSTATE          (0x00)
#define INPSTATE          (0x01)
#define EDTSTATE          (0x02)
#define csPrompt          "\r\n$>\0"
#define csCmdPrompt       "\r\ncmd>\0"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#if 0
typedef struct
{
    u8    index;
    u8    state;
    char  buffer[MAXCMDSIZE];
} cmdin_t, *pcmdin_t;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    char * cmd;
    char * help;
    void (*proc)( int argc, char * argv[] );
} menu_t, *pmenu_t;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    pmenu_t  menu;
    void     (*keyboard)( char ch );
    u8       index;
    u8       state;
    u8       argc;
    u8       optpos;
    char *   optarg;
    char *   argv[MAXARGS];
    char     buffer[MAXCMDSIZE];
} cmdhd_t, *pcmdhd_t;

/******************************************************************************
* 函数名      : s8 sea_getopt ( int argc, char *argv[], char *opt )
* 功能        : get option char
* 输入参数    : 无
* 输出参数    : option char
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
s8 sea_getopt ( int argc, char *argv[], char *opt );

/******************************************************************************
* 函数名      : void sea_parsecommand ( char * cmd, u8 len )
* 功能        : parse input command
* 输入参数    : char * cmd, u8 len
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
void sea_parsecommand ( char * cmd, u8 len );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
extern cmdhd_t cmdhd1;

///////////////////////////////////////////////////////////////////////////////////////////////////
#endif   //  __CONSOLE_H__
