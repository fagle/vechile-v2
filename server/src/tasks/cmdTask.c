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
extern frame_info_t consfrm1, w108frm1, tcpfrm1;     // for console protocol
static cmdin_t cmdin1 = { 0x00, 0x00, 0x00, 0x00, };

/******************************************************************************
* 函数名      : static void printHelp ( void )
* 功能        : 帮助说明
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
******************************************************************************/
void printHelp ( void )
{
    u8       pos = 0x00;
    
    sea_printf("\n------------------------- help -----------------------\n");
    sea_printf("singlekey command as following, only '0~9' as commands.\n");                     
    sea_printf("1 : Turn on all lamps\n");                           //打开全部路灯
    sea_printf("2 : Turn off all lamps\n");                          //关闭全部路灯
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
* 函数名      : void keyboardinput ( char ch )
* 功能        : keyboard input
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : void vCommandTask ( void )
* 功能        : 从console接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vCommandTask ( void *pvParameters )
{
    extern int lwip_send ();
    vu32     ticker = 0x00L;
    u8       ch;

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
        if (client.send.len && client.conn)
        { 
            if (lwip_send(client.sock, client.send.buf, client.send.len, 0x00) < 0x00)
                client.conn = 0x00;
            client.send.len = 0x00;
        }
#endif        
        
        if ((ticker % 0x200) == 0x00)           // ~4 second
        {
	    for (ch = 0x01; ch <= sys_info.ctrl.car && client.send.len == 0x00; ch ++)
            {
                ppath_t rut = msg_info.find(sys_info.ctrl.base + ch);
                if (rut->update)
                {
                    plamp_t ptr = (plamp_t)&rep_info.key[sys_info.ctrl.base + ch - 0x01];
                    if (dyn_info.report)
                    {
                        report_t  report1;
                        report1.carid    = ptr->vehicle.number;
                        report1.cardid   = ptr->vehicle.card;
                        report1.type     = ptr->vehicle.type;
                        report1.status   = ptr->vehicle.status;
                        report1.obstacle = ptr->vehicle.obstacle;
                        report1.step     = ptr->vehicle.index;
                        report1.count    = ptr->vehicle.count;
                        report1.fail     = ptr->vehicle.fail;
                        report1.speed    = ptr->vehicle.speed;
                        report1.magic    = ptr->vehicle.magic;
                        consfrm1.put(&consfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
#ifdef LWIP_ENABLE
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
#endif
                    }
                    else
                    {
                        sea_printreport(ptr);
#ifdef LWIP_ENABLE
                        tcpfrm1.print(&tcpfrm1, "\n%4d %4d %4x %4x %4x %4d %4x %4d %4d %4d, %2d:%2d", 
                                                            ptr->vehicle.type,
                                                            ptr->vehicle.number,
                                                            ptr->vehicle.status,
                                                            ptr->vehicle.fail,
                                                            ptr->vehicle.obstacle,
                                                            ptr->vehicle.card,
                                                            ptr->vehicle.magic,
                                                            ptr->vehicle.index,
                                                            ptr->vehicle.count,
                                                            ptr->vehicle.speed,
                                                            sea_getsystime()->min, sea_getsystime()->sec);
#endif
                    }
                    rut->update = 0x00;
                }
            }
            
            for (ch = 0x00; ch < sys_info.ctrl.call && client.send.len == 0x00; ch ++)
            {
                pcall_t cal = (pcall_t)rep_info.goal[ch];
                if (cal->update)
                {
                    if (dyn_info.report)
                    {
                        dyn_info.buffer[0x00] = cal->num;
                        dyn_info.buffer[0x01] = cal->type;
                        consfrm1.put(&consfrm1, ICHP_PC_RPTBEEP, 0x02, sys_info.ctrl.road, dyn_info.buffer);
#ifdef LWIP_ENABLE
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTBEEP, 0x02, sys_info.ctrl.road, dyn_info.buffer);
#endif
                    }
                    else
                    {
                        sea_printf("\ncall vehicle type %d from station %02x", cal->type, cal->num);
#ifdef LWIP_ENABLE
                        tcpfrm1.print(&tcpfrm1, "\ncall vehicle type %d from station %02x", cal->type, cal->num);
#endif
                    }
                    cal->update = 0x00;
                }
            }
        }
        
	vTaskDelay(0x01 / portTICK_RATE_MS);
    }
} 

////////////////////////////////////////////////////////////////////////////////////

