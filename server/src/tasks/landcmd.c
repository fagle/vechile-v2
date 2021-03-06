#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "network.h"
#include "mem.h"
#include "netconf.h"

////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;

////////////////////////////////////////////////////////////////////////////////////
//
static void printCommandHelp ( void )
{
    sea_printf("\nland Usage: land -[dviculprwfb] nnnn ...");
    sea_printf("\n  -d yy-mm-dd hh:mm:ss, setting landscape system date and time.");
    sea_printf("\n  -v nn, set max. vehicles in application.");
    sea_printf("\n  -c nn, set max. callers in application.");
    sea_printf("\n  -u nn, print devices of wrong number or type.");
    sea_printf("\n  -f nn, set system configuration.");
    sea_printf("\n  -p nn, print nn vehicle and caller route table, 0x00 for all");
    sea_printf("\n  -r nn, set relase/debug mode.");
    sea_printf("\n  -i nnn.nnn.nnn:port domain_name, setting server ip address and domain name.");
    sea_printf("\n  -l nnn.nnn.nnn mmm.mmm.mmm ggg.ggg.ggg, setting host ip address, mask and gateway ip address.");
    sea_printf("\n  -w ss nn nn ..., set card's map list, ss index of start, nn cards number.");
    sea_printf("\n  -b nn idx id act ..., set beeper's route table, nn beeper no, idx index, id act...");
}

static void sea_printcallroute ( u8 num )
{
    u8 i;
    
    if (!isCallDevice(CALLIDST, num))
        return;
    
    sea_printf("\ncaller %d, route size %d\n    id:", num, msg_info.call[num - 0x01].cnt);
    for (i = 0x00; i < msg_info.call[num - 0x01].cnt; i ++)
        sea_printf("%2x ", msg_info.call[num - 0x01].route[i].id);
    sea_printf("\naction:");
    for (i = 0x00; i < msg_info.call[num - 0x01].cnt; i ++)
        sea_printf("%2x ", msg_info.call[num - 0x01].route[i].action);
}

void sea_landconfig  ( int argc, char * argv[] )
{
    static u8 config = LAMP_CF_OPEN;
    s8  opt;
    int num;

    if (argc < 0x02)
    {
        sea_printsysinfo();
        sea_printf ("\nset date time is %d-%d-%d %d:%d:%d", sea_getsystime()->year, sea_getsystime()->mon, sea_getsystime()->day, 
                    sea_getsystime()->hour, sea_getsystime()->min, sea_getsystime()->sec);
	return;
    }

    if (argv[0x01][0x00] == '?' && argc == 0x02)
    {
        printCommandHelp();
        return;
    }
    
    while ((opt = sea_getopt(argc, argv, "dvicublprwf0")) != -1) 
    {
        if (opt != 0x00 && cmdhd1.optarg != NULL)
        {
            switch (opt) 
            {
                case 'd':
                    if (argc == 0x04)
                    {
                        systime_t time;
                        int yy, mm, dd, hh, ss;
                        sscanf(cmdhd1.optarg, "%2d-%2d-%2d", &yy, &mm, &dd);
                        time.year = yy & 0xff;
                        time.mon  = mm & 0xff;
                        time.day  = dd & 0xff;
                        if (time.year < 2000)
                            time.year += 2000;
                        sscanf(argv[0x03], "%2d:%2d:%2d", &hh, &mm, &ss);
                        time.hour = hh & 0xff;
                        time.min  = mm & 0xff;
                        time.sec  = ss & 0xff;
                        sea_printf ("\nset date time is %d-%d-%d %d:%d:%d", time.year, time.mon, time.day, time.hour, time.min, time.sec);
                        memcpy(sea_getsystime(), &time, sizeof(systime_t));
                        PCF8365T_WriteDate(time);
                    }
                    break;
                case 'v':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num && num <= MOBILEDEVS)
                    {
                        if (num != sys_info.ctrl.car && sys_info.ctrl.config == 0x63)
                        {
                            sys_info.ctrl.car     = num;
                            sys_info.ctrl.config  = config;              
                            sea_printf("\nchange max vehicles to %d", num);
                            sea_updatesysinfo();
                            consfrm1.put(&consfrm1, ICHP_SV_CTRL, sizeof(ctrl_t), sys_info.ctrl.road, (u8 *)&sys_info.ctrl);
                        }
                    }  
                    break; 
                case '0':
                    if (sys_info.ctrl.config == 0x11)
                    {
                        if (sea_memcomp(cmdhd1.optarg, "610\0", 0x03) == 0x00)
                        {
                            sea_printf("\nwrite system defalut value to flash.");
                            sea_writedefaultsysinfo();
                        }
                    }
                    break; 
                case 'f':
                    sscanf(cmdhd1.optarg, "%x", &num);
                    if (num)
                    {
                        config = sys_info.ctrl.config;
                        sys_info.ctrl.config = num & 0xff;
                        sea_printf("\nyou input value %02x", num & 0xff);
                    }
                    break; 
                case 'w':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (argc > 0x03)
                    {  
                        u8 i;
                        sys_info.card.cnt = num;
                        for (i = 0x00; i < argc - 0x03; i ++)
                        {
                            sscanf(argv[0x03 + i], "%d", &num);
                            if (num)
                                sea_flashwritecards(sys_info.card.cnt ++, num);  // sea_printf("%d ", num);  
                        }
                        sea_printf("\ncards: ");
                        i = 0x00;
                        while (*((u16 *)sea_flashreadcards(i)) != EMPTYFLASH)
                            sea_printf("%02x ",  *((u16 *)sea_flashreadcards(i ++)));
                    }
                    break; 
                case 'b':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (isCallDevice(CALLIDST, num))
                    {
                        u8  idx;
                        int tmp;
                        sscanf(argv[0x03], "%d", &tmp);
                        idx = tmp & 0xff;
                        for (u8 i = 0x00; i < argc - 0x04; i ++)
                        {
                            sscanf(argv[0x04 + i], "%d", &tmp);
                            dyn_info.buffer[i + idx * 0x02] = tmp & 0xff;
                        }
                        idx = idx * 0x02 + argc - 0x04;
                        msg_info.call[num - 0x01].route = (paction_t)sea_flashupdateroute(num - 0x01, msg_info.call[num - 0x01].route, 
                                                                                          dyn_info.buffer, idx);
                        msg_info.call[num - 0x01].cnt = idx / 0x02;
                        sea_printcallroute(num);
                    }
                    else
                        sea_printf("\n%dth device is not beeper.");
                    break; 
                case 'c':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (num && num <= CALLERDEVS)
                    {
                        if (num != sys_info.ctrl.call && sys_info.ctrl.config == 0x07)
                        {
                            sys_info.ctrl.call    = num;
                            sys_info.ctrl.config  = config;              
                            sea_printf("\nchange max caller to %d", num);
                            sea_updatesysinfo();
                            consfrm1.put(&consfrm1, ICHP_SV_CTRL, sizeof(ctrl_t), sys_info.ctrl.road, (u8 *)&sys_info.ctrl);
                        }
                    }
                    break; 
                case 'u':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    for (u8 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
                    {
                        if (rep_info.key[i].vehicle.fail & LAMP_ER_NP)
                        {
                            sea_printf("\nnumber %d, type %d unknow device.", rep_info.key[i].vehicle.number, rep_info.key[i].vehicle.type);
                            if (!(num --))
                                break;
                        }
                        else if (rep_info.key[i].vehicle.status)
                        {
                            sea_printf("\nnumber %d, type %d active.", rep_info.key[i].vehicle.number, rep_info.key[i].vehicle.type);
                            if (!(num --))
                                break;
                        }
                    }
                    break;
                case 'p':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    if (isCarDevice(CARIDST, num))
                        msg_info.print(num);
                    else if (isCallDevice(CALLIDST, num))
                    {
                        if (msg_info.call[num - 0x01].route == NULL)
                            sea_printf("\n%dth caller route table is empty.", num);
                        else
                            sea_printcallroute(num);
                    }
                    else if (!num)
                    {
                        for (u8 i = 0x01; i <= sys_info.ctrl.car; i ++)
                            msg_info.print(sys_info.ctrl.base + i);
                    }
                    break;
                case 'r':
                    sscanf(cmdhd1.optarg, "%d", &num);
                    num ? sea_printf("\nrelease mode on") : sea_printf("\ndebug mode on");
                    if (num != sys_info.ctrl.release)
                    {
                        sys_info.ctrl.release = num ? 0x01 : 0x00;
                        sea_updatesysinfo();
                        sea_releasemode();
                        consfrm1.put(&consfrm1, ICHP_SV_CTRL, sizeof(ctrl_t), sys_info.ctrl.road, (u8 *)&sys_info.ctrl);
                    }
                    break;
                case 'i':
                    if (argc == 0x04)
                    {
                        int ip[0x04], port;
                        sscanf(cmdhd1.optarg, "%3d.%3d.%3d.%3d:%d", &ip[0], &ip[1], &ip[2], &ip[3], &port);
                        sys_info.ip[0x00] = ip[0x00] & 0xff;
                        sys_info.ip[0x01] = ip[0x01] & 0xff;
                        sys_info.ip[0x02] = ip[0x02] & 0xff;
                        sys_info.ip[0x03] = ip[0x03] & 0xff;
                        if (port)
                            sys_info.port = port;
                        if (argv[0x03] != NULL)
                        {
                            sea_memset(sys_info.domain, 0x00, DOSIZE);
                            sea_memcpy(sys_info.domain, argv[0x03], strlen(argv[0x03]));
                        }
                        sea_updatesysinfo();
#ifdef LWIP_ENABLE     
                        if (client.conn)
                            client.conn = 0x00;
#endif
                    }
                    break;
                case 'l':
                    if (argc == 0x05)
                    {
                        int ip[0x04];
                        sscanf(cmdhd1.optarg, "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2], &ip[3]);
                        sys_info.local.ip[0x00] = ip[0x00] & 0xff;
                        sys_info.local.ip[0x01] = ip[0x01] & 0xff;
                        sys_info.local.ip[0x02] = ip[0x02] & 0xff;
                        sys_info.local.ip[0x03] = ip[0x03] & 0xff;
                        sscanf(argv[0x03], "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2], &ip[3]);
                        sys_info.local.mk[0x00] = ip[0x00] & 0xff;
                        sys_info.local.mk[0x01] = ip[0x01] & 0xff;
                        sys_info.local.mk[0x02] = ip[0x02] & 0xff;
                        sys_info.local.mk[0x03] = ip[0x03] & 0xff;
                        sscanf(argv[0x04], "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2], &ip[3]);
                        sys_info.local.gw[0x00] = ip[0x00] & 0xff;
                        sys_info.local.gw[0x01] = ip[0x01] & 0xff;
                        sys_info.local.gw[0x02] = ip[0x02] & 0xff;
                        sys_info.local.gw[0x03] = ip[0x03] & 0xff;
                        sea_updatesysinfo();
                        sea_updatenetaddr();
                    }
                    break;
                default:
                    printCommandHelp();
                    return;
            }
        }
    }
}

/*******************************************************************************
* Function Name  : void KeyCommandHandler ( u8 key )
* Description    : keybord command handler
* Input          : u8 key
* Output         : None
* Return         : None
*******************************************************************************/
void KeyCommandHandler ( u8 key )
{ 
    frame_t frm;
    
    switch (key)
    {
        case eKeyNum0:
            if (dyn_info.state != LAMP_OPEN)
            {
                sea_printf("\nopen all lamps.");
                frm.cmd = ICHP_SV_OPEN;
                frm.road = sys_info.ctrl.road;
#if defined(HDU_ENABLE) || defined(LANDSCAPE_MODE)         
                frm.body[0x00] = LAMPALLON >> 0x08; 
#else                
                frm.body[0x00] = MAXPERCENT; 
#endif            
                ServerFrameCmdHandler(frm);
            }
            break;
        case eKeyNum1:
            if (dyn_info.state != LAMP_CLOSE)
            {
                sea_printf("\nclose all lamps.");
                frm.cmd = ICHP_SV_CLOSE;
                frm.road = sys_info.ctrl.road;
                frm.body[0x00] = 0x00; 
                ServerFrameCmdHandler(frm);
            }
            break;
        case eKeyNum2:
            break;
        case eKeyNum3:
            break;
        case eKeyNum4:
            break;
        case eKeyNum5:
            break;
        case eKeyNum6:
            break;
        case eKeyNum7:
            break;
        case eKeyNum8:
            break;
        case eKeyNum9:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////

