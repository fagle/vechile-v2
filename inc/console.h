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
* ������      : s8 sea_getopt ( int argc, char *argv[], char *opt )
* ����        : get option char
* �������    : ��
* �������    : option char
* �޸ļ�¼    : ��
* ��ע        : ��
******************************************************************************/
s8 sea_getopt ( int argc, char *argv[], char *opt );

/******************************************************************************
* ������      : void sea_parsecommand ( char * cmd, u8 len )
* ����        : parse input command
* �������    : char * cmd, u8 len
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
******************************************************************************/
void sea_parsecommand ( char * cmd, u8 len );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
extern cmdhd_t cmdhd1;

///////////////////////////////////////////////////////////////////////////////////////////////////
#endif   //  __CONSOLE_H__
