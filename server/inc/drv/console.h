#ifndef __CONSOLE_H__
#define __CONSOLE_H__

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h" 
#include "systicker.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAXCMDSIZE        (0x80)
#define MAXARGS           (0x40)
#define CMDSTATE          (0x00)
#define INPSTATE          (0x01)
#define EDTSTATE          (0x02)
#define csPrompt          "\n\r$>\0"
#define csCmdPrompt       "\n\rcmd>\0"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8    index;
    u8    state;
    char  buffer[MAXCMDSIZE];
} cmdin_t, *pcmdin_t;

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
    u8     argc;
    u8     optpos;
    char * optarg;
    char * argv[MAXARGS];
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
