#include "network.h"
#include "sink.h"
#include "sensor.h"
#include "console.h"
#include "frame.h"
#include "vehicle.h"

////////////////////////////////////////////////////////////////////////////////////
//
void sea_lampconfig  ( int argc, char * argv[] );
void sea_roadconfig  ( int argc, char * argv[] );

////////////////////////////////////////////////////////////////////////////////////
//
const menu_t mnArray[] = { 
#ifdef SINK_APP
                           {"road", "config road information", sea_roadconfig},
#else
                           {"lamp", "config lamp information", sea_lampconfig},
#endif                           
                           {NULL, NULL, NULL} };

//////////////////////////////////////////////////////////////////////////////////////
//
cmdhd_t cmdhd1 = { (pmenu_t)mnArray, NULL, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
frame_info_t   uartfrm;

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
    
    DBG("\r\n------------------------- help -----------------------");
    DBG("\r\nsinglekey command as following, only '0~9' as commands.");                     
    DBG("\r\n1 : turn on all lamps.");                               // 打开全部路灯
    DBG("\r\n2 : turn off all lamps.");                              // 关闭全部路灯
    DBG("\r\n3 : change radio channel (default channel 13, id 0x1010).");        
    DBG("\r\n4 : reboot all lamps on the raod.");        
    DBG("\r\n5 : sent ICHP_CO_ACKNOWLEDGE cmmand to all lamps.");        
    DBG("\r\n6 : print active lamps and open state.");        
    DBG("\r\n7 : not used yet.");        
    DBG("\r\n8 : not used yet.");        
    DBG("\r\n9 : not used yet.");        
    DBG("\r\n0 : write default configuration to flash.");        
    DBG("\r\nmultikey command as following, [enter] key is end of commands.");                     
    DBG("\r\ncommand format : <cmd> [p1] [p2] ... etc");   
    while (cmdhd1.menu[pos].cmd != NULL)
    {
        DBG("\r\n%s : %s", cmdhd1.menu[pos].cmd, cmdhd1.menu[pos].help);             
        pos ++;
    }
    DBG("\r\n? : Print help command, not kit 'h(0x68)' for a special char");  
}

#ifdef SINK_APP
static void sea_sendresponse ( u8 num, u8 cmd, u8 status )
{
    if (sys_info.ctrl.release)
    {
        u8  resp[0x03];
        resp[0x00] = num;
        resp[0x01] = cmd;
        resp[0x02] = status;
        uartfrm.put(&uartfrm, ICHP_SV_RESPONSE, 0x00, 0x03, resp);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_parseroadframe ( pframe_t ptr )
//* 功能        : 处理上位机通过串口发的信息
//* 输入参数    : pframe_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_parseroadframe ( pframe_t ptr )
{
    u8  modify = 0x00;
    u16 addr;
    
    switch (ptr->cmd)
    {
        case ICHP_SV_ACKNOWLEDGE:
            if (ptr->len <= road_info.map.size)
            {
                DBG("\r\n[sink]acknowledge len %d", ptr->len);
                sea_memcpy(road_info.map.state, ptr->body, ptr->len);
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_ACKNOWLEDGE, ptr->len, road_info.map.state); 
            }
            break;
        case ICHP_SV_ROUTE:
            if (ptr->len < (ACTSIZE * sizeof(action_t) + 0x01) && (ptr->len & 0x01) == 0x00)
            {
                ptr->road = road_info.address(ptr->body[0x00]);
                if (ptr->road)
                    sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ROUTE, ptr->len, ptr->body);   
                else
                    sea_sendresponse(ptr->body[0x00], ICHP_SV_ROUTE, SENDERROR);
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_ROUTE, SENDERROR);
            break;
        case ICHP_SV_OPEN:      
            ptr->road = road_info.address(ptr->body[0x00]);
            if (ptr->road)
            {
                DBG("\r\n[sink]start %d vehicle", road_info.number(ptr->road));  
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_OPEN, ptr->len, ptr->body); 
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_OPEN, SENDERROR);
            break;
        case ICHP_SV_CLOSE:
            ptr->road = road_info.address(ptr->body[0x00]);
            if (ptr->road)
            {
                DBG("\r\n[sink]stop %d vehicle", road_info.number(ptr->road));
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_CLOSE, ptr->len, ptr->body); 
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_CLOSE, SENDERROR);
            break;
        case ICHP_SV_REBOOT:        
            addr = ptr->road ? ptr->road : road_info.address(ptr->body[0x00]);
            if (addr)
            {
                DBG("\r\n[sink]reboot %d vehicle", road_info.number(ptr->road));
                sea_sendmsg(&send1, UNICAST, addr, ICHP_SV_REBOOT, ptr->len, ptr->body); 
            }
            else
            {
                sea_sendresponse(ptr->body[0x00], ICHP_SV_REBOOT, SENDOK);
                halReboot();
            }
            break;
        case ICHP_SV_RPT:       
            if (ptr->road)
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_RPT, 0x00, NULL); 
            else
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_RPT, 0x00, NULL); 
            break;
        case ICHP_SV_END:        
            ptr->road = road_info.address(ptr->body[0x00]);
            if (ptr->road)
            {
                DBG("\r\n[sink]caller's vehicle arrival", road_info.number(ptr->road));  
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_END, ptr->len, ptr->body); 
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_END, SENDERROR);
            break;
        case ICHP_SV_DEFAULT:      
            ptr->road = road_info.address(ptr->body[0x00]);
            if (ptr->road)
            {
                DBG("\r\n[sink]set default %d vehicle", road_info.number(ptr->road));  
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_DEFAULT, ptr->len, ptr->body); 
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_DEFAULT, SENDERROR);
            break;
        case ICHP_SV_ADJUST:      
            ptr->road = road_info.address(ptr->body[0x00]);
            if (ptr->road)
            {
                DBG("\r\n[sink]adjust %dth vehicle radio channel to %d", road_info.number(ptr->road), ptr->body[0x01]);  
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ADJUST, ptr->len, ptr->body); 
            }
            else if (!ptr->body[0x00])
            {
                DBG("\r\n[sink]adjust coordinator radio channel to %d", ptr->body[0x01]);  
                sea_adjustchannel(ptr->body[0x01]);
                sea_sendresponse(ptr->body[0x00], ICHP_SV_ADJUST, SENDOK);
                halReboot();
            }
            else
                sea_sendresponse(ptr->body[0x00], ICHP_SV_ROUTE, SENDERROR);;
            break;
        case ICHP_SV_ADDRESS:             // synchronous device_t structure body
            if (ptr->road && ptr->len == sizeof(device_t))
            {
                u16 num = road_info.number(ptr->road);
                sea_memcpy(&network_info.dev, ptr->body, sizeof(device_t));
                if (num != network_info.dev.num)
                {
                    DBG("\r\n[sink]change number %d to %d ", num, network_info.dev.num);  
                    sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ADDRESS, sizeof(device_t), (u8 *)&network_info.dev); 
                    if (sys_info.ctrl.release)
                        uartfrm.put(&uartfrm, ptr->cmd, ptr->road, sizeof(device_t), (u8 *)&network_info.dev);
                }
                else
                    sea_sendresponse(ptr->body[0x00], ICHP_SV_ROUTE, SENDERROR);
            }
            else if (ptr->road == 0x00)
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_ADDRESS, 0x00, NULL); 
            else if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ptr->cmd, sys_info.ctrl.road, sizeof(device_t), (u8 *)&sys_info.dev);
            break;   
        case ICHP_SV_CTRL:            // set control mode, period etc., handler subcommand
            if (ptr->len == sizeof(ctrl_t))
            {
                sea_memcpy(&sys_info.ctrl, ptr->body, sizeof(ctrl_t));
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_CTRL, sizeof(ctrl_t), &sys_info.ctrl); 
                modify = 0x01;
            }
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ptr->cmd, sys_info.ctrl.road, sizeof(ctrl_t), (u8 *)&sys_info.ctrl);
            break;   
        case ICHP_SV_CHANNEL:             // synchronous channel_t structure body
            if (ptr->len == sizeof(channel_t))
            {
                pchannel_t chn = (pchannel_t)ptr->body;
                if (chn->index != sys_info.channel.index || chn->mask != sys_info.channel.mask)
                {
                    sea_writedefaultconfig(sys_info.ctrl.app, sys_info.dev.type, chn->index, chn->mask);
                    DBG("\r\n[sink]change radio channel, rebooting ...");
                    halReboot();
                }
            }
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ptr->cmd, sys_info.ctrl.road, sizeof(channel_t), (u8 *)&sys_info.channel);
            break;
        case ICHP_SV_SECKEY:              // synchronous key_t structure body
            if (ptr->len == sizeof(key_t))
            {
                sea_memcpy(&sys_info.key, ptr->body, sizeof(key_t));
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_SECKEY, sizeof(key_t), &sys_info.key); 
                modify = 0x01;
            }
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ptr->cmd, sys_info.ctrl.road, sizeof(key_t), (u8 *)&sys_info.key);
            break;
        case ICHP_SV_DATE:                // 与Server时间同步，更新时间
            sea_memcpy(&single_info.time, ptr->body, sizeof(systime_t) - 0x04);
            sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_DATE, sizeof(systime_t), &single_info.time);
            sea_sendresponse(ptr->body[0x00], ICHP_SV_DATE, SENDOK);
            break;   
        case ICHP_SV_ASSIGN:
        {
            if (ptr->road)
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ASSIGN, ptr->len, ptr->body); 
            else
            {
                ptr->road = road_info.address(ptr->body[0x00]);
                if (ptr->road)
                    sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ASSIGN, ptr->len, ptr->body); 
                else
                    sea_sendresponse(ptr->body[0x00], ICHP_SV_ASSIGN, SENDERROR);
            }
            ptr->road = road_info.address(ptr->body[0x03]);            // body[0x03], vehicle's number, send to vehicle
            if (ptr->road)
                sea_sendmsg(&send1, UNICAST, ptr->road, ICHP_SV_ASSIGN, ptr->len, ptr->body); 
            break;
        }
        default:
            sea_sendresponse(ptr->body[0x00], ptr->cmd, SENDERROR);
            DBG("\r\n[sink]unknown cmd: 0x%x, road %d from server", ptr->cmd, ptr->road);
            break;
    }
    
    if (modify)
        sea_updatesysconfig();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void printRoadHelp ( void )
//* 功能        : print road help
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void printRoadHelp ( void )
{
    DBG("\r\nroad Usage: road -[octrnjaxpdb] nnnn ...");
    DBG("\r\n  -o nn, start vehicle you given.");
    DBG("\r\n  -c nn, stop vehicle you given.");
    DBG("\r\n  -t nn, reboot vehicle you given.");
    DBG("\r\n  -r n1 idx id act id act ..., set route table to vehicle.");
    DBG("\r\n  -n n1 num type, change vehicle from n1 to number and type.");
    DBG("\r\n  -j nnn indeix, adjust channel(nnn: 0x00-coordinator, other-vehicles).");
    DBG("\r\n  -a nnn, change vehicle application type.");
    DBG("\r\n  -x mmm bbb, change vehicle max. and base devices.");
    DBG("\r\n  -p nnn ccc, change vehicle report period and config mode.");
    DBG("\r\n  -d nnn, change vehicle debug/release mode.");
    DBG("\r\n  -b nnn, set system configuration (0x00 for default value).");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_roadconfig  ( int argc, char * argv[] )
//* 功能        : road command interface
//* 输入参数    : int argc, char * argv[]
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_roadconfig  ( int argc, char * argv[] )
{
    int num;
    s8  opt;
    frame_t  frm;

    if (argc < 0x02)
    {
        sea_printsysconfig();
	return;
    }

    if (argv[0x01][0x00] == '?' && argc == 0x02)
    {
        printRoadHelp();
        return;
    }
    
    while ((opt = sea_getopt(argc, argv, "octrnjaxpdb")) != -1) 
    {
        if (opt != 0x00 && cmdhd1.optarg != NULL)
        {
            switch (opt) 
            {
                case 'o':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num > 0x00 && num <= sys_info.ctrl.maxdev)
                    {
                        frm.cmd        = ICHP_SV_OPEN;
                        frm.road       = road_info.address(num);
                        frm.len        = 0x01;
                        frm.body[0x00] = num & 0xff;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'c':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num > 0x00 && num <= sys_info.ctrl.maxdev)
                    {
                        frm.cmd        = ICHP_SV_CLOSE;
                        frm.road       = road_info.address(num);
                        frm.len        = 0x01;
                        frm.body[0x00] = num & 0xff;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 't':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num > 0x00 && num <= sys_info.ctrl.maxdev)
                    {
                        frm.cmd        = ICHP_SV_REBOOT;
                        frm.road       = road_info.address(num);
                        frm.len        = 0x01;
                        frm.body[0x00] = num & 0xff;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'j':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= sys_info.ctrl.maxdev)   
                    {
                        frm.body[0x00] = num & 0xff;
                        sscanf(argv[0x03], "%d", &num);
                        frm.body[0x01] = num & 0xff;
                        frm.cmd        = ICHP_SV_ADJUST;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = 0x02;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'n':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (road_info.address(num))
                    {
                        u16 addr = road_info.address(num);
                        sscanf(argv[0x03], "%d", &num);
                        network_info.dev.num = num & 0xff;
                        sscanf(argv[0x04], "%d", &num);
                        network_info.dev.type = num & 0xff;
                        if (network_info.dev.num > 0x00 && network_info.dev.num < sys_info.ctrl.maxdev || 
                            addr != road_info.address(network_info.dev.num))
                        {
                            frm.cmd  = ICHP_SV_ADDRESS;
                            frm.len  = sizeof(device_t);
                            frm.road = addr;
                            sea_memcpy(&frm.body, &network_info.dev, sizeof(device_t));
                            sea_parseroadframe(&frm);
                        }
                    }
                    break;
                case 'r':
                    sscanf(cmdhd1.optarg, "%d", &num);                   
                    if ((frm.road = road_info.address(num & 0xff)) != 0x00 && (argc & 0x01) == 0x00)
                    {
                        frm.body[0x00] = num & 0xff;              // number of logic 
                        sscanf(argv[0x03], "%d", &num);           
                        frm.body[0x01] = num & 0xff;              // index of route table
                        for (u8 i = 0x00; i < argc - 0x04; i ++)
                        {
                            sscanf(argv[0x04 + i], "%d", &num);   // id and action
                            frm.body[i + 0x02] = num & 0xff;
                        }
                        frm.cmd = ICHP_SV_ROUTE;
                        frm.len = argc - 0x02;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'x':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= EMBER_MAXDEVS)
                    {
                        sys_info.ctrl.maxdev = num;
                        sscanf(argv[0x03], "%d", &num);
                        if (num < sys_info.ctrl.maxdev)
                        {
                            sys_info.ctrl.base = num;
                            frm.cmd        = ICHP_SV_CTRL;
                            frm.road       = sys_info.ctrl.road;
                            frm.len        = sizeof(ctrl_t);
                            sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                            sea_parseroadframe(&frm);
                        }
                    }
                    break;
                case 'a':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= eCondition)
                    {
                        frm.cmd        = ICHP_SV_CTRL;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(ctrl_t);
                        sys_info.ctrl.app = num;
                        sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'd':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (serial_info.enable)
                        sea_printf("\nenter release mode.");
                    sys_info.ctrl.release = sys_info.ctrl.release ? 0x00 : 0x01;   // num & 0x01;
                    serial_info.enable    = sys_info.ctrl.release ? 0x00 : 0x01;
                    if (serial_info.enable)
                        sea_printf("\nenter debug mode.");
                    break;
                case 'b':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (!num)
                    {
                        sea_defaultconfig();
                        halReboot();
                    }
                    else
                    {
                        frm.cmd        = ICHP_SV_DEFAULT;
                        frm.road       = road_info.address(num);
                        frm.len        = 0x01;
                        frm.body[0x00] = num & 0xff;
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'q':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num)
                    {
                        frm.cmd        = ICHP_SV_CTRL;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(ctrl_t);
                        sys_info.ctrl.road = num;
                        sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                        sea_parseroadframe(&frm);
                    }
                    break;
                case 'p':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num)
                    {
                        sys_info.ctrl.period = num;
                        sscanf(argv[0x03], "%d", &num);
                        sys_info.ctrl.config = num;
                        frm.cmd        = ICHP_SV_CTRL;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(ctrl_t);
                        sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                        sea_parseroadframe(&frm);
                    }
                    break;
                default:
                    printRoadHelp();
                    return;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_roadkeyboardinput ( char ch )
//* 功能        : road command input from keyboard or uart
//* 输入参数    : char ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_roadkeyboardinput ( char ch )
{
    if (cmdhd1.state == CMDSTATE)
    {
#if 0
        switch (ch)
        {
            case '?':
                printHelp();
                break;
            case '0':
                DBG("\r\nwrite default configuration to flash, application will reboot, waiting...");
                sea_defaultconfig();
                halReboot();
                break;
            case '1': 
                break;   
            case '2': 
                break;
            case '3':
                DBG("\r\ntell caller or vehicle route end.");
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_CO_END, 0x00, NULL);
                break;
            case '4':
                DBG("\r\nall lamps will reboot.");
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_CO_REBOOT, 0x00, NULL);
                break;
            case '5':
                DBG("\r\nsend acknowlege.");
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_CO_ACKNOWLEDGE, road_info.map.size, road_info.map.state); 
               break;
            case '6':
                break;
            case '7':                
                break;
            case '8':
                break;
            case '9':
                break;
            default:
                cmdhd1.state = INPSTATE;
                if (cmdhd1.index != 0x00)
                { 
                    cmdhd1.index = 0x00;
                    memset(cmdhd1.buffer, 0x00, MAXCMDSIZE);
                }
                cmdhd1.buffer[cmdhd1.index ++] = ch;
                if (ch == '\n' || ch == '\r')
                    cmdhd1.state = CMDSTATE;
                DBG("%s%c", csPrompt, ch);
                break;
        }
#else
        cmdhd1.state = INPSTATE;
        if (cmdhd1.index != 0x00)
        { 
            cmdhd1.index = 0x00;
            sea_memset(cmdhd1.buffer, 0x00, MAXCMDSIZE);
        }
        cmdhd1.buffer[cmdhd1.index ++] = ch;
        if (ch == '\n' || ch == '\r')
            cmdhd1.state = CMDSTATE;
        DBG("%s%c", csPrompt, ch);
#endif        
    }
    else if (cmdhd1.state == INPSTATE) 
    {
        if (ch == '\n' || ch == '\r')
        {
            if (cmdhd1.index)
            {
                DBG("\n\r");
                sea_parsecommand(cmdhd1.buffer, cmdhd1.index);
            }
            DBG(csCmdPrompt);
            cmdhd1.state = CMDSTATE;
        }
        else
        {
            DBG("%c", ch);
            cmdhd1.buffer[cmdhd1.index ++] = ch;
            if (cmdhd1.index >= MAXCMDSIZE)
            {
                DBG("\n\r");
                sea_parsecommand(cmdhd1.buffer, cmdhd1.index);
                DBG(csCmdPrompt);
                cmdhd1.state = CMDSTATE;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////
#else  // end SINK_APP
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void printLampHelp ( void )
//* 功能        : print lamp help
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void printLampHelp ( void )
{
    DBG("\r\nlamp Usage: lamp -[ntjcaxklbr] nnnn ...");
    DBG("\r\n  -n num, change vehicle number.");
    DBG("\r\n  -t type, change vehicle type.");
    DBG("\r\n  -j index, adjust radio channel");
    DBG("\r\n  -a nnn, change vehicle application type.");
    DBG("\r\n  -x nnn, change vehicle max. devices.");
    DBG("\r\n  -k nnn, change road rf_id, or caller's type(4~8).");
    DBG("\r\n  -l nnn, print route table.");
    DBG("\r\n  -r nnn, set debug/release mode.");
    DBG("\r\n  -c print current vehicle/beep status.");
    DBG("\r\n  -b nnn, set system configuration (0x00 for default value).");
    DBG("\r\n  -p send current vehicle reoprt to server.");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_lampkeyboardinput ( char ch )
//* 功能        : lamp command input from keyboard or uart
//* 输入参数    : char ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_lampkeyboardinput ( char ch )
{
    if (cmdhd1.state == CMDSTATE)
    {
#if 0
        switch (ch)
        {
            case '?':
                printHelp();
                break;
            case '0':
                DBG("\r\nwrite default configuration to flash, application will reboot, waiting...");
                sea_defaultconfig();
                halReboot();
                break;
            case '3':      
                if (!carInfo.route.count)
                    DBG("\r\nroute table is empty.");
                else
                {
                    u8 i;
                    DBG("\r\nroute table size %d.\r\n", carInfo.route.count);
                    for (i = 0x00; i < carInfo.route.count; i ++)
                        DBG("%02x ", carInfo.route.line[i].id);
                    DBG("\r\n");
                    for (i = 0x00; i < carInfo.route.count; i ++)
                        DBG("%02x ", carInfo.route.line[i].action);
                }
                break;
            case '4':
                DBG("\r\nvehicle route end.");
                carInfo.plc.fail |= END_LINE;
                sea_sendmsg(&send1, UNICAST, COORDID, ICHP_CO_END, sizeof(vehicle_t), &carInfo.plc);
                break;
            case '5':
                if (carInfo.route.count)
                {
                    DBG("\r\nreport currnt card %02x, action %02x, index %d.", carInfo.route.line[carInfo.route.index].id, 
                                                                               carInfo.route.line[carInfo.route.index].action,
                                                                               carInfo.route.index);
                    carInfo.plc.card  = carInfo.route.line[carInfo.route.index].id;
                    carInfo.route.index ++;
                    if (carInfo.route.index >= carInfo.route.count)
                        sea_memset(&carInfo.route, 0x00, sizeof(route_t));
                    sea_printlamp(&single_info.lamp);
                }
                else
                {
                    DBG("\r\nroute is empty, sample card %02x", card);
                    carInfo.plc.card  = card;
                    card ++;
                    if (card >= 0x80)
                        card = 0x01;
                }
                magic >>= 0x01;
                carInfo.plc.magic = magic;
                if (!magic)
                    magic = 0xffff;
                set_lampmode(LAMP_UPDATE);
                break;
            case '6':   
                if (sys_info.dev.type == CALLAID)
                {
                    DBG("\r\ncaller A, type A");
                    carInfo.plc.status = MOBILEAID;
                    set_lampmode(LAMP_UPDATE);
                }
                else if (sys_info.dev.type == CALLBID)
                {
                    DBG("\r\ncaller B, type C");
                    carInfo.plc.status = MOBILECID;
                    set_lampmode(LAMP_UPDATE);
                }
                break;
            case '7':
                if (sys_info.dev.type == CALLAID)
                {
                    DBG("\r\ncaller A, type B");
                    carInfo.plc.status = MOBILEBID;
                    set_lampmode(LAMP_UPDATE);
                }
                else if (sys_info.dev.type == CALLBID)
                {
                    DBG("\r\ncaller B, type D");
                    carInfo.plc.status = MOBILEDID;
                    set_lampmode(LAMP_UPDATE);
                }
                break;
            case '8':
                if(isCarDevice())
                {
                    carInfo.plc.status = CAR_RUN;
                    set_lampmode(LAMP_UPDATE);
                }
                break;
            case '9':
                sea_printf("\napplication reboot.");
                halReboot();
//                DBG("\r\nclear table.");
//                table_info.clear();
                break;
            case '1': 
                DBG("\r\nopen lamp ...");
                single_info.on(NULL);
                break;   
            case '2': 
            {
                DBG("\r\nclose lamp just now ...");
                single_info.off(NULL);
                break;
            }
            default:
                cmdhd1.state = INPSTATE;
                if (cmdhd1.index != 0x00)
                { 
                    cmdhd1.index = 0x00;
                    memset(cmdhd1.buffer, 0x00, MAXCMDSIZE);
                }
                cmdhd1.buffer[cmdhd1.index ++] = ch;
                if (ch == '\n' || ch == '\r')
                    cmdhd1.state = CMDSTATE;
                 DBG("%s%c", csPrompt, ch);
                break;
        }
#else
        cmdhd1.state = INPSTATE;
        if (cmdhd1.index != 0x00)
        { 
            cmdhd1.index = 0x00;
            sea_memset(cmdhd1.buffer, 0x00, MAXCMDSIZE);
        }
        cmdhd1.buffer[cmdhd1.index ++] = ch;
        if (ch == '\n' || ch == '\r')
            cmdhd1.state = CMDSTATE;
        DBG("%s%c", csPrompt, ch);
#endif        
    }
    else if (cmdhd1.state == INPSTATE) 
    {
        if (ch == '\n' || ch == '\r')
        {
            if (cmdhd1.index)
            {
                DBG("\n\r");
                sea_parsecommand(cmdhd1.buffer, cmdhd1.index);
            }
            DBG(csCmdPrompt);
            cmdhd1.state = CMDSTATE;
        }
        else
        {
             DBG("%c", ch);
            cmdhd1.buffer[cmdhd1.index ++] = ch;
            if (cmdhd1.index >= MAXCMDSIZE)
            {
                DBG("\n\r");
                sea_parsecommand(cmdhd1.buffer, cmdhd1.index);
                DBG(csCmdPrompt);
                cmdhd1.state = CMDSTATE;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_parselampframe ( pframe_t ptr )
//* 功能        : 处理上位机通过串口发的信息
//* 输入参数    : pframe_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_parselampframe ( pframe_t ptr )
{
    switch (ptr->cmd)
    {
        case ICHP_SV_OPEN:
            DBG("\r\nopen lamp");
            single_info.on(NULL);
            break;
        case ICHP_SV_END:
            DBG("\r\nclose led");
            carInfo.off();
            break;
        case ICHP_SV_CLOSE:
            DBG("\r\nclose lamp");
            single_info.off(NULL);
            break;
        case ICHP_SV_ADJUST:
            DBG("\r\nadjust radio channel to %d", ptr->body[0x01]);
            sea_adjustchannel(ptr->body[0x01]);
            break;
        case ICHP_SV_ADDRESS:
        {
            pdevice_t tmp = (pdevice_t)ptr->body;
            if (tmp->num > 0x00 && tmp->num < sys_info.ctrl.maxdev && ptr->len == sizeof(device_t))
            {
                sys_info.dev.num  = tmp->num;
                sys_info.dev.type = tmp->type;
                Debug("\r\nchange device_t body information.");
                sea_updatesysconfig();
            }
            break;
        }
        case ICHP_SV_CHANNEL:
        {
            pchannel_t chn = (pchannel_t)ptr->body;
            if (chn->index != sys_info.channel.index || chn->mask != sys_info.channel.mask)
            {
                if (chn->index != sys_info.channel.index && chn->index <= MAXCHANNEL)
                    chn->mask = 0x01 << chn->index;
                sea_writedefaultconfig(sys_info.ctrl.app, sys_info.dev.type, chn->index, chn->mask);
                DBG("\r\nchange radio channel, rebooting ...");
                halReboot();
            }
            break;
        }
        case ICHP_SV_CTRL:
        {
            pctrl_t  tmp = (pctrl_t)ptr->body;
            if (tmp->maxdev < MAXLAMPS && tmp->period >= MINPERIOD && tmp->period <= MAXPERIOD &&
                tmp->app >= eLights && tmp->app <= eCondition && ptr->len == sizeof(ctrl_t))
            {
                sea_memcpy(&sys_info.ctrl, tmp, sizeof(ctrl_t));
                Debug("\r\nchange ctrl_t body information.");
                sea_updatesysconfig();
            }
            break;
        }
        case PLC_CAR_INFO:
            recvPlcStateFrm(ptr);            
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_lampconfig  ( int argc, char * argv[] )
//* 功能        : lamp command interface
//* 输入参数    : int argc, char * argv[]
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_lampconfig  ( int argc, char * argv[] )
{
    frame_t  frm;
    s8       opt;
    int      num;

    if (argc < 0x02)
    {
	sea_printsysconfig();
        return;
    }

    if (argv[0x01][0x00] == '?' && argc == 0x02)
    {
        printLampHelp();
        return;
    }
    
    while ((opt = sea_getopt(argc, argv, "ntjcaxpklbr")) != -1) 
    {
        if (opt != 0x00 && cmdhd1.optarg != NULL)
        {
            switch (opt) 
            {
                case 'n':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num < sys_info.ctrl.maxdev)
                    {
                        sys_info.dev.num = num;
                        frm.cmd        = ICHP_SV_ADDRESS;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(device_t);
                        sea_memcpy(frm.body, &sys_info.dev, sizeof(device_t));
                        sea_parselampframe(&frm);
                    }
                    break;
                case 't':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num > CENTERIDCNT)
                    {
                        sys_info.dev.type = num;
                        frm.cmd        = ICHP_SV_ADDRESS;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(device_t);
                        sea_memcpy(frm.body, &sys_info.dev, sizeof(device_t));
                        sea_parselampframe(&frm);
                    }
                    break;
                case 'j':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num < MAXCHANNEL)
                    {
                        frm.cmd        = ICHP_SV_ADJUST;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = 0x01;
                        frm.body[0x00] = sys_info.dev.num;
                        frm.body[0x01] = num;
                        sea_parselampframe(&frm);
                    }
                    break;
                case 'a':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= eCondition)
                    {
                        sys_info.ctrl.app = num;
                        frm.cmd        = ICHP_SV_CTRL;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(ctrl_t);
                        sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                        sea_parselampframe(&frm);
                    }
                    break;
                case 'x':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num <= EMBER_MAXDEVS)
                    {
                        sys_info.ctrl.maxdev = num;
                        frm.cmd        = ICHP_SV_CTRL;
                        frm.road       = sys_info.ctrl.road;
                        frm.len        = sizeof(ctrl_t);
                        sea_memcpy(frm.body, &sys_info.ctrl, sizeof(ctrl_t));
                        sea_parselampframe(&frm);
                    }
                    break;
                case 'r':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (serial_info.enable)
                        sea_printf("\nenter release mode.");
                    sys_info.ctrl.release = sys_info.ctrl.release ? 0x00 : 0x01;   // num & 0x01;
                    serial_info.enable    = sys_info.ctrl.release ? 0x00 : 0x01;
                    if (serial_info.enable)
                        sea_printf("\nenter debug mode.");
                    break;
                case 'l':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (!carInfo.route.count)
                        DBG("\r\nroute table is empty.");
                    else
                    {
                        u8 i;
                        DBG("\r\nroute table size %d.\r\n", carInfo.route.count);
                        for (i = 0x00; i < carInfo.route.count; i ++)
                            DBG("%02x ", carInfo.route.line[i].id);
                        DBG("\r\n");
                        for (i = 0x00; i < carInfo.route.count; i ++)
                            DBG("%02x ", carInfo.route.line[i].action);
                    }
                    break;
                case 'c':
                    sea_printlamp(&single_info.lamp);
                    if (isCarDevice())
                    {
                        if (carInfo.key)
                            DBG("\r\n%dth vehicle will go to %dth station.", sys_info.dev.num, carInfo.key);
                    }
                    break;
                case 'p':
                    set_lampmode(LAMP_CHANGE);
                    break;
                case 'k':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (isCallDevice())
                    {
                        DBG("\r\ncaller type %d (%d~%d).", num, CARIDST, CARIDST + CARIDCNT);
                        if (num >= CARIDST && num < CARIDST + CARIDCNT)
                        {
                            carInfo.beep->call = num;
                            set_lampmode(LAMP_FORCE);
                            carInfo.beep->assign = 0x00;
                            sea_printlamp(&single_info.lamp);
                        }
                    }
                    else 
                    {
                        if (carInfo.route.count)
                        {
                            DBG("\r\nreport currnt card %02x, action %02x, index %d.", carInfo.route.line[carInfo.route.index].id, 
                                                                                       carInfo.route.line[carInfo.route.index].action,
                                                                                       carInfo.route.index);
                            carInfo.plc.card  = carInfo.route.line[carInfo.route.index].id;
                            carInfo.route.index ++;
                            if (carInfo.route.index >= carInfo.route.count)
                                sea_memset(&carInfo.route, 0x00, sizeof(route_t));
                            
                        }
                        else
                            DBG("\r\nroute is empty.");
                    }
                    break;
                case 'b':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    switch (num)
                    {
                        case 0x00:
                            DBG("\r\nwrite default configuration to flash, application will reboot, waiting...");
                            sea_defaultconfig();
                            halReboot();
                            break;
                        case 0x04:
                            DBG("\r\nvehicle route end.");
                            carInfo.plc.fail |= END_LINE;
                            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_END, sizeof(vehicle_t), &carInfo.plc);
                            break;
                    }
                    break;
                default:
                    printLampHelp();
                    return;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////
#endif  // _LAMP_
