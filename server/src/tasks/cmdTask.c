#include "network.h"

#define csSamples  "searen\0"

////////////////////////////////////////////////////////////////////////////////////
//
void sea_landconfig  ( int argc, char * argv[] );
void sea_w108config  ( int argc, char * argv[] );

////////////////////////////////////////////////////////////////////////////////////
//
const menu_t mnArray[] = { {"land", "config server information", sea_landconfig},
                           {"w108", "config zigbee information", sea_w108config},
                           {NULL, NULL, NULL} };

////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;     // for console protocol
static cmdin_t cmdin1 = { 0x00, 0x00, 0x00, 0x00, };

/******************************************************************************
* ������      : static void printHelp ( void )
* ����        : ����˵��
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
******************************************************************************/
void printHelp ( void )
{
    u8       pos = 0x00;
    
    sea_printf("\n------------------------- help -----------------------\n");
    sea_printf("singlekey command as following, only '0~9' as commands.\n");                     
    sea_printf("1 : Turn on all lamps\n");                           //��ȫ��·��
    sea_printf("2 : Turn off all lamps\n");                          //�ر�ȫ��·��
    sea_printf("3 : not used yet\n");        
    sea_printf("4 : not used yet\n");        
    sea_printf("5 : not used yet\n");        
    sea_printf("6 : not used yet\n");        
    sea_printf("7 : not used yet\n");        
    sea_printf("8 : not used yet\n");        
    sea_printf("9 : not used yet\n");        
    sea_printf("0 : not used yet\n");        
    sea_printf("multikey command as following, [enter] key is end of commands.\n");                     
    sea_printf("command format : <cmd> [p1] [p2] ... etc\n");   
    while (mnArray[pos].cmd != NULL)
    {
        sea_printf("%s : %s\n", mnArray[pos].cmd, mnArray[pos].help);             
        pos ++;
    }
    sea_printf("? : Print help command, not kit 'h(0x68)' for a special char\n");  
}

/******************************************************************************
* ������      : void keyboardinput ( char ch )
* ����        : keyboard input
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
******************************************************************************/
void keyboardinput ( char ch )
{
//    frame_t frm;

    if (cmdin1.state == CMDSTATE)
    {
#if 0      
        switch (ch)
        {
            case '?':
                printHelp();
                break;
            case '0':
                sea_printf("\nwrite default value to flash.");
                sea_writedefaultsysinfo();
                sea_reboot("\nwrite system information, need rebbot");
                break;
            case '3':
                sea_printaddress();     
                break;
            case '4':
                sea_printf("\ntype number status fail obstacle card magic index count speed  time");
                for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
                {
                    sea_printreport(&rep_info.key[i]);
                }
                break;
            case '5':
                for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
                    msg_info.print(i + 0x01);
                break;
            case '6':
                sea_printf("\nsend date and time to vehicles");
                w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
                break;
            case '7':
                break;
            case '8':
            case '9':
                break;
            case '1': 
                sea_printf("\nopen all lamps.");
                sea_openlandscape(0x01);
                frm.cmd = ICHP_SV_OPEN;
                frm.road = sys_info.ctrl.road;
                frm.body[0x00] = MAXPERCENT; 
                ServerFrameCmdHandler(frm);
                break;   
            case '2': 
                sea_printf("\nclose all lamps.");
                frm.cmd = ICHP_SV_CLOSE;
                frm.road = sys_info.ctrl.road;
                frm.body[0x00] = 0x00; 
                ServerFrameCmdHandler(frm);
                sea_closelandscape(0x01);
                break;
            default:
                cmdin1.state = INPSTATE;
                if (cmdin1.index != 0x00)
                { 
                    cmdin1.index = 0x00;
                    memset(cmdin1.buffer, 0x00, MAXCMDSIZE);
                }
                cmdin1.buffer[cmdin1.index ++] = ch;
                if (ch == '\n' || ch == '\r')
                    cmdin1.state = CMDSTATE;
                sea_printf("%s%c", csPrompt, ch);
                break;
        }
#else
        cmdin1.state = INPSTATE;
        if (cmdin1.index != 0x00)
        { 
             cmdin1.index = 0x00;
             memset(cmdin1.buffer, 0x00, MAXCMDSIZE);
        }
        cmdin1.buffer[cmdin1.index ++] = ch;
        if (ch == '\n' || ch == '\r')
           cmdin1.state = CMDSTATE;
        sea_printf("%s%c", csPrompt, ch);
#endif
    }
    else if (cmdin1.state == INPSTATE) 
    {
        if (ch == '\n' || ch == '\r')
        {
            if (cmdin1.index)
                sea_parsecommand(cmdin1.buffer, cmdin1.index);
            sea_printf("%s", csCmdPrompt);
            cmdin1.state = CMDSTATE;
        }
        else
        {
            sea_printf("%c", ch);
            cmdin1.buffer[cmdin1.index ++] = ch;
            if (cmdin1.index >= MAXCMDSIZE)
            {
                sea_parsecommand(cmdin1.buffer, cmdin1.index);
                sea_printf("%s", csCmdPrompt);
                cmdin1.state = CMDSTATE;
            }
        }
    }
}

/*********************************************************
* ������      : void vCommandTask ( void )
* ����        : ��console���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vCommandTask ( void *pvParameters )
{
    vu32     ticker = 0x00L;

#ifdef VEHICLE_RELEASE 
    if (!sys_info.ctrl.release)
    {
        sys_info.ctrl.release = 0x01;
        sea_updatesysinfo();
    }
#endif
    sea_releasemode();
#ifdef DEBUG_PRINTF
    ShowCopyrights();
#endif
    
    while (0x01)
    {
        ticker ++;  
        
        if (!sea_isempty(get_serialinfo(w108frm1.uart)))   
            w108frm1.recv(&w108frm1, sea_readbyte(get_serialinfo(w108frm1.uart)));  

        if (!sea_iskeyempty())
            KeyCommandHandler(sea_readkey());
 
#ifdef LWIP_ENABLE
        if (client.send.len)
        { 
            if (client.conn)
                lwip_send(client.sock, client.send.buf, client.send.len, 0x00);
            client.send.len = 0x00;
        }
#endif        
	vTaskDelay(0x01 / portTICK_RATE_MS);
    }
} 

////////////////////////////////////////////////////////////////////////////////////
