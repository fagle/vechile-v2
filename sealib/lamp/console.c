#include <stdlib.h>
#include "network.h"

/******************************************************************************
* 函数名      : s8 sea_getopt ( int argc, char *argv[], char *opt )
* 功能        : get option char
* 输入参数    : 无
* 输出参数    : option char
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
s8 sea_getopt ( int argc, char *argv[], char *opt )
{
    int         i;
    char        token;
 
    cmdhd1.optarg = NULL;
    token = *(opt + cmdhd1.optpos);
    if (token != '\0')
    {
        for (i = 0x00; i < argc; i ++)
	{
            if (argv[i][0] != '-')
                continue;

            if (strchr(argv[i], token) != 0x00)
	    {
                cmdhd1.optarg = argv[i + 0x01];
		cmdhd1.optpos ++;
                return token & 0xff;
	    }
	}
        cmdhd1.optpos ++;
        return 0x00;
    }
  
    cmdhd1.optpos = 0x00;
    return -1;  
}

/******************************************************************************
* 函数名      : static u8 sea_parseargs ( char *cmd )
* 功能        : parse input command
* 输入参数    : 无
* 输出参数    : count of parameters
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
static u8 sea_parseargs ( char *cmd )
{
    char *c;
    int  state = 0x00;

    cmdhd1.argc = 0x00;
    if (strlen(cmd) == 0x00)
        return cmdhd1.argc;

    c = cmd;
    while (*c != '\0')
    {
        if (*c == '\t')
            *c = ' ';
	c ++;
    }
	
    c = cmd;
    while (*c != '\0')
    {
        if (state == 0x00)
        {
            if (*c != ' ')
            {
                cmdhd1.argv[cmdhd1.argc ++] = c;		
                state = 0x01;
            }
        }
        else
        { 
            if (*c == ' ')
            {
                *c = '\0';
                state = 0x00;
            }
        }
        c ++;
    }
    return cmdhd1.argc;
}

/******************************************************************************
* 函数名      : static int sea_getcmdmatche ( char *cmd )
* 功能        : match input command
* 输入参数    : char *cmd
* 输出参数    : index of command
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
static int sea_getcmdmatche ( char *cmd )
{
    int i;	
	
    for (i = 0x00; cmdhd1.menu[i].cmd != NULL; i ++)
    {
        if (strncmp(cmdhd1.menu[i].cmd, cmd, strlen(cmdhd1.menu[i].cmd)) == 0x00)
            return i;
    }
	
    return -1;
}

/******************************************************************************
* 函数名      : void sea_parsecommand ( char * cmd, u8 len )
* 功能        : parse input command
* 输入参数    : char * cmd, u8 len
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
void sea_parsecommand ( char * cmd, u8 len )
{
    int   index;

    if (sea_parseargs(cmd) == 0x00)
        return;
	
    if ((index = sea_getcmdmatche(cmdhd1.argv[0x00])) >= 0x00)
    {	
        if (cmdhd1.menu[index].proc != NULL)	
	    cmdhd1.menu[index].proc(cmdhd1.argc, cmdhd1.argv);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
