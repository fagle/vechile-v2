#include "network.h"
#include "cmd.h"
#include "flash.h"
#include "frame.h"
#include "lamp.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
const time_t DEFOPEN[MAXMONTH]   = {{ 0x10, 0x1e, 0x00 },  // 1, open lamp time 16:30:00
                                    { 0x11, 0x00, 0x00 },  // 2, open lamp time 17:00:00
                                    { 0x11, 0x1e, 0x00 },  // 3, open lamp time 17:30:00
                                    { 0x12, 0x00, 0x00 },  // 4, open lamp time 18:00:00
                                    { 0x12, 0x1e, 0x00 },  // 5, open lamp time 18:30:00
                                    { 0x13, 0x00, 0x00 },  // 6, open lamp time 19:00:00
                                    { 0x13, 0x1e, 0x00 },  // 7, open lamp time 19:30:00
                                    { 0x13, 0x00, 0x00 },  // 8, open lamp time 19:00:00
                                    { 0x12, 0x1e, 0x00 },  // 9, open lamp time 18:30:00
                                    { 0x12, 0x00, 0x00 },  // 10, open lamp time 18:00:00
                                    { 0x11, 0x1e, 0x00 },  // 11, open lamp time 17:30:00
                                    { 0x11, 0x00, 0x00 }}; // 12, open lamp time 17:00:00
const time_t DEFCLOSE[MAXMONTH]  = {{ 0x07, 0x1e, 0x00 },  // 1, close lamp time 07:30:00
                                    { 0x07, 0x00, 0x00 },  // 2, close lamp time 07:00:00
                                    { 0x06, 0x1e, 0x00 },  // 3, close lamp time 06:30:00
                                    { 0x06, 0x00, 0x00 },  // 4, close lamp time 06:00:00
                                    { 0x05, 0x1e, 0x00 },  // 5, close lamp time 05:30:00
                                    { 0x05, 0x00, 0x00 },  // 6, close lamp time 05:00:00
                                    { 0x04, 0x1e, 0x00 },  // 7, close lamp time 04:30:00
                                    { 0x05, 0x00, 0x00 },  // 8, close lamp time 05:00:00
                                    { 0x05, 0x1e, 0x00 },  // 9, close lamp time 05:30:00
                                    { 0x06, 0x00, 0x00 },  // 10, close lamp time 06:00:00
                                    { 0x06, 0x1e, 0x00 },  // 11, close lamp time 06:30:00
                                    { 0x07, 0x00, 0x00 }}; // 12, close lamp time 07:00:00

/////////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;     // for console protocol
sys_info_t  sys_info;
dyn_info_t  dyn_info;

/************************************************
* 函数名      : void sea_writedefaultsysinfo ( void )
* 功能        : write defalut system information to flash
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_writedefaultsysinfo ( void )
{
//   const u8 DEFIP[IPSIZE] = { 10, 1, 130, 168 };     // ecs.hdu.edu.cn
     const u8 DEFIP[IPSIZE]      = { 210, 32, 34, 139 };      // lamp.dnsd.info:6181
     
     const u8 DEFLOCIP[IPSIZE]   = { 192, 168, 1, 105 };   // local ip address
     const u8 DEFLOCMK[IPSIZE]   = { 255, 255, 255, 0 };   // local mask
     const u8 DEFLOCGW[IPSIZE]   = { 192, 168, 1, 1 };     // local gateway ip
     const u8 DEFLOCMAC[MACSIZE] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x24 };     // mac physic address
#ifndef VEHICLE_MODE    
     const u8 DEFSECMODE       = Secure_WPA_WPA2_PSK;    // mode of security
#endif
     
    sea_memset(&sys_info, 0x00, sizeof(sys_info_t));
    sys_info.mark         = SYSMARK;
    sys_info.size         = sizeof(sys_info_t);
    
    sys_info.ctrl.maxdev  = MAXLAMPS;
    sys_info.ctrl.road    = DEFALUTROADID;
    sys_info.ctrl.ver     = SWVERSION;
    sys_info.ctrl.app     = eVehicle;                  // agv application, added 2013/12/14
    sys_info.ctrl.period  = DEFALUTPERIOD;             // report lamp information time(sec)  
    sys_info.ctrl.config  = LAMP_CF_OPEN;              // default config with auto mode
    sys_info.ctrl.base    = MOBILEBASE;                // base number of vehicles
    sys_info.ctrl.car     = MOBILEDEVS;
    sys_info.ctrl.call    = CALLERDEVS;
    sys_info.ctrl.type    = 0x04;                      // 5,6,7,8 types
    sys_info.ctrl.release = 0x00;                      // debug version
    sea_memcpy(sys_info.name, DEFROADNAME, strlen(DEFROADNAME)); 
    sys_info.port         = DEFPORT;

    sea_memcpy(sys_info.ip, (void *)DEFIP, IPSIZE);
    sea_memcpy(sys_info.domain, DEFDOMAIN, strlen(DEFDOMAIN));
    sea_memcpy(sys_info.staff[0x00].name, DEFSTAFFNAME, strlen(DEFSTAFFNAME));
    sea_memcpy(sys_info.staff[0x00].phone, DEFPHONE, strlen(DEFPHONE));

    sea_memcpy(sys_info.local.ip, (void *)DEFLOCIP, IPSIZE);
    sea_memcpy(sys_info.local.mk, (void *)DEFLOCMK, IPSIZE);
    sea_memcpy(sys_info.local.gw, (void *)DEFLOCGW, IPSIZE);
    sea_memcpy(sys_info.local.mac, (void *)DEFLOCMAC, MACSIZE);
    
#ifndef VEHICLE_MODE    
    sys_info.route.size = 0x05;
    sys_info.route.mode = DEFSECMODE;
    sea_memcpy(sys_info.route.ssid, DEFSSID, strlen(DEFSSID));
    sea_memcpy(sys_info.route.key, DEFPWAPSK, strlen(DEFPWAPSK));
    
    for (u8 i = 0x00; i < MAXMONTH; i ++)
        sea_stageinsert(DEFOPEN[i], DEFCLOSE[i], MAXPERCENT, i);
#else
    sys_info.card.cnt   = 0x00;
#endif    
    sea_flashwrite(0x00, &sys_info, sizeof(sys_info_t));    
}

/************************************************
* 函数名      : sea_updatesysinfo ( void )
* 功能        : update system information to flash
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_updatesysinfo ( void )
{
    sea_flashwrite(0x00, &sys_info, sizeof(sys_info_t));
}

/************************************************
* 函数名      : void sea_releasemode ( void )
* 功能        : set debug/release mode
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_releasemode ( void )
{
    if (!sys_info.ctrl.release)
    {
        dyn_info.serial = 0x00;
        dyn_info.report = 0x00;
    }
    else
    {
        dyn_info.serial = 0x01;
        dyn_info.report = 0x01;
    }
}

/************************************************
* 函数名      : phone_t * sea_getstaff ( u8 index )
* 功能        : get staff name and phone
* 输入参数    : 无
* 输出参数    : name
* 修改记录    : 无
* 备注        : 无
*************************************************/  
phone_t * sea_getstaff ( u8 index )
{
    if (sys_info.staff[index].name[0x00])
        return &sys_info.staff[index];
    
    return NULL;
}

/************************************************
* 函数名      : void sea_setstaff ( u8 index, phone_t *staff )
* 功能        : set staff name and phone
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_setstaff ( u8 index, phone_t *staff )
{
    sea_memcpy(&sys_info.staff[index], staff, sizeof(phone_t));
}

/************************************************
* 函数名      : void sea_printsysinfo ( void )
* 功能        : print system information
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/  
void sea_printsysinfo ( void )
{
    u8 i;
    
    sea_printf("\nsystem information:");
    sea_printf("\nmax. devices: %d", sys_info.ctrl.maxdev);
    sea_printf("\nbase of vehicle: %d", sys_info.ctrl.base);
    sea_printf("\nnode id and name: %dth,%s", sys_info.ctrl.road, sys_info.name);
    sea_printf("\nsoftware version: %d.%02d", sys_info.ctrl.ver >> 0x08, sys_info.ctrl.ver & 0xff);
    sea_printf("\nsoftware running in %s mode", sys_info.ctrl.release ? "release" : "debug");
    sea_printf("\nmax. vehicles work in the application: %d", sys_info.ctrl.car);
    sea_printf("\nmax. callers work in the application: %d", sys_info.ctrl.call);
    sea_printf("\nmax. types of vehicle work in the application: %d", sys_info.ctrl.type);
    sea_printf("\nsystem period: %d", sys_info.ctrl.period);
    sea_printf("\nsystem control: %02x", sys_info.ctrl.config);
    sea_printf("\ndevice physical address: ");
    printEUI64(&sys_info.dev.eui);
    
    sea_printf("\nweb server ip & port: %d.%d.%d.%d:%d", sys_info.ip[0], sys_info.ip[1], sys_info.ip[2], sys_info.ip[3], sys_info.port);
    sea_printf("\nweb server domain: %s", sys_info.domain);
    
    sea_printf("\nlocal ip, mask & gateway: %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d", 
                                            sys_info.local.ip[0], sys_info.local.ip[1], sys_info.local.ip[2], sys_info.local.ip[3], 
                                            sys_info.local.mk[0], sys_info.local.mk[1], sys_info.local.mk[2], sys_info.local.mk[3], 
                                            sys_info.local.gw[0], sys_info.local.gw[1], sys_info.local.gw[2], sys_info.local.gw[3]);
#ifdef WIFI_MODE
    sea_printf("\nwifi key length and mode: %d, %d", sys_info.route.size, sys_info.route.mode);
    sea_printf("\nwifi ssid and security key: %s, %s", sys_info.route.ssid, sys_info.route.key);
#endif
    
    for (i = 0x00; i < PHSIZE; i ++)
    {
        if (sys_info.staff[i].name[0x00] && sys_info.staff[i].phone[0x00])
            sea_printf("\n%dth staff name and phone number %s, %s", i + 0x01, sys_info.staff[i].name, sys_info.staff[i].phone);
    }
    
#ifndef VEHICLE_MODE    
    sea_printf("\ncontrol stage as following:");
    for (j = 0x00; j < MAXMONTH; j ++)
    {
        for (i = 0x00; i < MAXSTAGE; i ++)
        {
            sect_t * ptr = &sys_info.stage[j].sect[i];
        
            if (sea_caltime(&ptr->open) == sea_caltime(&ptr->close))
                continue;
            sea_printf("\n%dth, open time %d:%d:%d, close time %d:%d:%d, which %d", j + 0x01, ptr->open.hour,
                                                                            ptr->open.min,
                                                                            ptr->open.sec,
                                                                            ptr->close.hour,
                                                                            ptr->close.min,
                                                                            ptr->close.sec,
                                                                            ptr->percent);
        }
    }
#else
    sea_printf("\ncontrol map of cards as following:\n");
    for (i = 0x00; i < sys_info.card.cnt; i ++)
    {
        if (sys_info.card.list[i] && sys_info.card.list[i] < 0x80)
            sea_printf("%02x ", sys_info.card.list[i]);
    }
#endif   // not VEHICLE_MODE    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printw108sysinfo ( void )
//* 功能        : print w108 system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printw108sysinfo ( void )
{    
    sea_printf("\n----w108 system config information------");
    sea_printf("\nroad id : %d", dyn_info.w108.ctrl.road);
    sea_printf("\napplication type : %d", dyn_info.w108.ctrl.app);
    sea_printf("\nmax. devices : %d", dyn_info.w108.ctrl.maxdev);
    sea_printf("\nlamp config : %02x", dyn_info.w108.ctrl.config);
    sea_printf("\nreport period : %d", dyn_info.w108.ctrl.period);
    sea_printf("\nbase application : %d", dyn_info.w108.ctrl.base);
    sea_printf("\nsoftware version : %d.%d", (dyn_info.w108.ctrl.ver >> 0x08), dyn_info.w108.ctrl.ver & 0xff);
#if 0
    u8 * tmp = (u8 *)&dyn_info.w108.dev.eui;
    u8 i;
    
    sea_printf("\ndevice type : %d", dyn_info.w108.dev.type);
    sea_printf("\nlamp number : %d", dyn_info.w108.dev.num);
    sea_printf("\ndevice physical address : ");
    for (i = EUI64_SIZE; i > 0x00; i --) 
        sea_printf("%02x", tmp[i - 0x01]);
#endif
    sea_printf("\nchannel mask : %08x", dyn_info.w108.channel.mask);
    sea_printf("\nchannel index : %d", dyn_info.w108.channel.index);
    sea_printf("\nnetwork radio pan id : %04x", dyn_info.w108.channel.panid);
    sea_printf("\nnetwork radio channel : %d", dyn_info.w108.channel.channel);
    sea_printf("\nradio tx power : %d", dyn_info.w108.channel.power);
    sea_printf("\nnetwork profile id : %04x", dyn_info.w108.channel.profile);
#if 0
    sea_printf("\nnetwork expanded id : %s", dyn_info.w108.channel.expanid);
    sea_printf("\nnetwork link codes : %s", dyn_info.w108.key.link);
    sea_printf("\nnetwork security codes : %s\n", dyn_info.w108.key.network);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : psys_info_t getsysinfo ( void )
//* 功能        : get system information structure body
//* 输入参数    : 无
//* 输出参数    : psys_info_t pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
psys_info_t getsysinfo ( void )
{
    return &sys_info;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageupdate ( time_t open, time_t close, u8 percent, u8 mon, u8 index )
//* 功能        : update lamp stage of open/close
//* 输入参数    : time_t open, time_t close, u8 percent, u8 mon, u8 index
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageupdate ( time_t open, time_t close, u8 percent, u8 mon, u8 index )
{
#ifndef VEHICLE_MODE    
    sect_t * ptr;
    
    if (index >= MAXSTAGE || sea_caltime(&open) == sea_caltime(&close))
          return;
    
    ptr = &sys_info.stage[mon].sect[index];
    memcpy(&ptr->open, &open, sizeof(time_t));
    memcpy(&ptr->close, &close, sizeof(time_t));
    ptr->percent = (percent < MINPERCENT || percent > MAXPERCENT) ? (MAXPERCENT) : (percent);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageinsert ( time_t open, time_t close, u8 percent, u8 mon )
//* 功能        : set lamp stage of open/close
//* 输入参数    : time_t open, time_t close, u8 percent, u8 mon
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageinsert ( time_t open, time_t close, u8 percent, u8 mon )
{
#ifndef VEHICLE_MODE    
    u8 i = 0x00;
    sect_t * ptr;
    
    if (sea_caltime(&open) == sea_caltime(&close))
        return;
    
    while (i < MAXSTAGE)
    {  
        ptr = &sys_info.stage[mon].sect[i ++];
        if (sea_caltime(&ptr->open) == sea_caltime(&ptr->close))  
        {
            sea_memcpy(&ptr->open, &open, sizeof(time_t));
            sea_memcpy(&ptr->close, &close, sizeof(time_t));
            ptr->percent = (percent < MINPERCENT || percent > MAXPERCENT) ? (MAXPERCENT) : (percent);
            return;
        }
    }

    ptr = &sys_info.stage[mon].sect[MAXSTAGE - 0x01];
    sea_memcpy(&ptr->open, &open, sizeof(time_t));
    sea_memcpy(&ptr->close, &close, sizeof(time_t));
    ptr->percent = (percent < MINPERCENT || percent > MAXPERCENT) ? (MAXPERCENT) : (percent);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_stageclear ( void )
//* 功能        : clear lamp stages
//* 输入参数    : None
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_stageclear ( void )
{
#ifndef VEHICLE_MODE    
    u8 i, j;

    for (j = 0x00; j < MAXMONTH; j ++)
    {
        for (i = 0x00; i < MAXSTAGE; i ++)
            memset(&sys_info.stage[j].sect[i], 0x00, sizeof(sect_t));
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_isopentime ( const systime_t * tm )
//* 功能        : is lamp lamp stage of open/close
//* 输入参数    : const systime_t * tm
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
u8 sea_isopentime ( const systime_t * tm )
{
#ifndef VEHICLE_MODE    
    time_t time;
    u8     i;
    u8     mon = tm->mon - 0x01;
    
    time.hour = tm->hour;
    time.min  = tm->min;
    time.sec  = tm->sec;
    
    for (i = 0x00; i < MAXSTAGE; i ++)
    {
        sect_t * ptr = &sys_info.stage[mon].sect[i];
        if (sea_caltime(&ptr->open) == sea_caltime(&ptr->close))
            continue;
        
        if (sea_caltime(&ptr->open) > sea_caltime(&ptr->close))  // pass middle night
        {
            if (sea_caltime(&time) > sea_caltime(&ptr->open) || sea_caltime(&time) < sea_caltime(&ptr->close))
                return ptr->percent;
        }
        else
        {
            if (sea_caltime(&time) > sea_caltime(&ptr->open) && sea_caltime(&time) < sea_caltime(&ptr->close))
                return ptr->percent;
        }
    }
#endif
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_timeventhandler ( void )
//* 功能        : time event handler
//* 输入参数    : None
//* 输出参数    : None
//* 修改记录    : None
//* 备注        : None
//*------------------------------------------------*/
void sea_timeventhandler ( void )
{
#ifdef VEHICLE_MODE
#ifdef TRAFFIC_ENABLE
    sea_memset(&traffic_info.state, 0x00, msg_info.cardsize);
    for (i = 0x00; i < sys_info.ctrl.maxdev; i ++)
    {
        if (isCarDevice(&rep_info.key[i]) && rep_info.key[i].vehicle.card)
            set_cardbitmap(traffic_info.state, rep_info.key[i].vehicle.card - 0x01);
    }
    for (i = 0x00; i < msg_info.cardsize; i ++)
    {
        if (!get_cardbitmap(traffic_info.state, i) && !traffic_info.ept(i + 0x01))
            traffic_info.clr(i + 0x01);
    }
    if (sea_memcomp(traffic_info.state, msg_info.state, msg_info.cardsize))
    {
        sea_memcpy(msg_info.state, traffic_info.state, msg_info.cardsize);
        msg_info.update = 0x01;
    }
#endif 
   
#else  // #else not VEHICLE_MODE                
    u8  percent;
    static u16 delaytime = 0x00;
    
    if (!get_ctrlmode(LAMP_AUTO))
    {
        if (sea_isopentime(sea_getsystime()))
        {
            if (delaytime ++ >= 60)   // 600 second, for waiting time 
            {
                set_ctrlmode(LAMP_AUTO);
                delaytime = 0x00;
            }
        }
        return;
    }
    
    if ((percent = sea_isopentime(sea_getsystime())) != 0x00) 
    {
        if (!get_ctrlmode(LAMP_POWER) || !sea_getpowerstate())
        {
            TurnOn_Power();  
            sea_printf("\npower on!");
        }

        if (dyn_info.state != LAMP_OPEN) 
        {
            sea_printf("\nopen all lamps on th road");
            Turn_On_All_Lamp(percent);
            dyn_info.percent = percent;
        }
        else if (percent != dyn_info.percent)
        {
            dyn_info.percent = percent;
            Turn_On_All_Lamp(percent);
        }
    }
    else
    {
        if (dyn_info.state != LAMP_CLOSE)
        {
            sea_printf("\nclose all lamps on th road");
            Turn_Off_All_Lamp();
        }

        if (get_ctrlmode(LAMP_POWER) || sea_getpowerstate())
        {
            TurnOff_Power();  
            sea_printf("\npower off!");
        }
    }
#endif   // endif VEHICLE_MODE                
}
////////////////////////////////////////////////////////////////////////////////
