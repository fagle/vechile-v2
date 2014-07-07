
#include "network.h"
#include "flash.h"

const panid_t panid_infox[16] = { {11, 0x1212}, {12, 0x0f0f}, {13, 0x1010}, {14, 0x0808}, 
                               {15, 0x5858}, {16, 0x0a0a}, {23, 0x0101}, {18, 0x6868}, 
                               {19, 0xa0a0}, {20, 0x9090}, {21, 0x9898}, {22, 0x0505}, 
                               {17, 0x7070}, {24, 0x5050}, {25, 0xf0f0}, {26, 0x0606} };

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_defaultconfig ( void )
//* 功能        : default system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_defaultconfig ( void )
{
    u8  app, type, index;
    u32 mask;
    
    app  = eVehicle;                       // agv application, added 2013/12/14
#ifdef SINK_APP  
    #ifdef ENABLE_GATEWAY
        type  = GWIDST;                    // GATEWAYID;
        mask  = DEFALUTGWCH;               // 0x00008000, gateway radio channel
        index = 0x0f;
    #else    
        type  = CENTERIDST;                // CENTERID;
        mask  = DEFALUTCOORDCH;            // 0x000000001, coordinator radio channel
        index = CHANNAL_INDEX;
    #endif  // ENABLE_GATEWAY    
#else
    index = CHANNAL_INDEX;
    #ifdef ENABLE_GATEWAY
        type  = GWIDST;                    // GATEWAYID;
        mask  = DEFALUTGWCH;               // ~0x00008000, gateway'router radio channel
    #else    
        if (app == eVehicle)
            type = CARIDST;                // MOBILEAID;
        else if (app == eCondition)
            type = AIRIDST;                // CONDITIONID;
        else
            type = LAMPIDST;               // LAMPID;
        mask  = DEFALUTMASK;               // ~0x0000ffff, device's radio channel
    #endif  // ENABLE_GATEWAY    
#endif  // SINK_APP       
    sea_writedefaultconfig(app, type, index, mask);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void SYS_Configuration ( void )
//* 功能        : system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void SYS_Configuration ( void )
{
    sea_flashread(0x00, sizeof(sys_info_t), &sys_info);
    if (sys_info.mark != SYSMARK || sys_info.size != sizeof(sys_info_t))  
        sea_defaultconfig();
    else if(sys_info.channel.channel < 11 || sys_info.channel.channel > 26 || sys_info.channel.index > MAXCHANNEL)
        sea_defaultconfig();
    else if (sys_info.dev.type != CENTERIDST && sys_info.dev.type < CALLIDST)   // before software 2.0
        sea_defaultconfig();
    else
        Debug("\r\nread system information from flash.");
    
#if defined(USE_HARDCODED_NETWORK_SETTINGS) && defined(SENSOR_APP)
    sys_info.channel.index       = 0x0f;
    sys_info.channel.panid   = panid_infox[sys_info.channel.index].panid;     // DEF_PANID;
    sys_info.channel.channel = panid_infox[sys_info.channel.index].channel;   // DEF_CHANNEL;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_configprint ( void )
//* 功能        : config sea_printf and DBG 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_configprint ( void )
{
#ifdef VEHICLE_RELEASE     // def  1 for RELEASE, 0 DEBUG
    if (!sys_info.ctrl.release)
    {
        sys_info.ctrl.release = 0x01;
        sea_updatesysconfig();
    }
#else
    if (sys_info.ctrl.release)
    {
        sys_info.ctrl.release = 0x00;
        sea_updatesysconfig();
    }
#endif
    serial_info.enable = sys_info.ctrl.release ? 0x00 : 0x01;   
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printdevice ( pdevice_t ptr )
//* 功能        : print device_t information 
//* 输入参数    : pdevice_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printdevice ( pdevice_t ptr )
{
    if (!ptr)     return;

    DBG("\r\ndevice number %d, type %d, ", ptr->num, ptr->type);
    DBG("physical address ");
    printEUI64((EmberEUI64*)ptr->eui);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printdevicetable ( void )
//* 功能        : print device_t information in table
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printdevicetable ( void )
{
    pdevice_t  ptr;
    
    ptr = (pdevice_t)table_info.first();
    while (ptr)
    {
        sea_printdevice(ptr);
        ptr = (pdevice_t)table_info.next(TBDEVICE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printchannel ( pchannel_t ptr )
//* 功能        : print channel_t information 
//* 输入参数    : pchannel_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printchannel ( pchannel_t ptr )
{
    if (!ptr)     return;

    DBG("\r\nchannel mask %08x", ptr->mask);
    DBG("\r\nnetwork radio index %d", ptr->index);
    DBG("\r\nnetwork radio pan id %04x", ptr->panid);
    DBG("\r\nnetwork radio profile %04x", ptr->profile);
    DBG("\r\nnetwork radio channel %d", ptr->channel);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printcontrol ( pctrl_t ptr )
//* 功能        : print ctrl_t information 
//* 输入参数    : pctrl_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printcontrol ( pctrl_t ptr )
{
    if (!ptr)     return;

    DBG("\r\nmax. devices %d, road id %d, version %d.%d", ptr->maxdev, ptr->road, ptr->ver >> 0x08, ptr->ver & 0xff);
    DBG("\r\nbase %d, period %d, control %02x, application %d", ptr->base, ptr->period, ptr->config, ptr->app);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printsysconfig ( void )
//* 功能        : print system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printsysconfig ( void )
{    
    char*  ptr="";
    EmberNetworkParameters networkParams;
    
    DBG("\r\n----system config information------");
    DBG("\r\nroad id : %d", sys_info.ctrl.road);
    DBG("\r\nmax. devices : %d", sys_info.ctrl.maxdev);
    DBG("\r\ndevice id : %d", sys_info.dev.type);
    if (sys_info.dev.type >= CENTERIDST && sys_info.dev.type < CENTERIDST + CENTERIDCNT)
        ptr = ", control center.";
    else if (sys_info.dev.type >= CALLIDST && sys_info.dev.type < CALLIDST + CALLIDCNT)
        ptr = ", agv caller devices.";
    else if (sys_info.dev.type >= CARIDST && sys_info.dev.type < CARIDST + CARIDCNT)
        ptr = ", agv vehicle devices.";
    else if (sys_info.dev.type >= MOVEIDST && sys_info.dev.type < MOVEIDST + MOVEIDCNT)
        ptr = ", agv moving devices.";
    else if (sys_info.dev.type >= LAMPIDST && sys_info.dev.type < LAMPIDST + LAMPIDCNT)
        ptr = ", led lamp devices.";
    else if (sys_info.dev.type >= AIRIDST && sys_info.dev.type < AIRIDST + AIRIDCNT)
        ptr = ", air condition device.";
    DBG("%s", ptr);
    DBG("\r\nlamp config : %02x", sys_info.ctrl.config);
    if (network_info.type == EMBER_ROUTER)
        DBG("\r\nlamp number : %d", sys_info.dev.num);
    DBG("\r\nsoftware running in %s mode", sys_info.ctrl.release ? "release" : "debug");
    DBG("\r\nmax. vehicles work in the application: %d", sys_info.ctrl.car);
    DBG("\r\nmax. callers work in the application: %d", sys_info.ctrl.call);
    DBG("\r\nmax. types of vehicle work in the application: %d", sys_info.ctrl.type);
    DBG("\r\ndevice physical address : ");
    printEUI64((EmberEUI64*)sys_info.dev.eui);
    DBG("\r\nreport period : %d", sys_info.ctrl.period);
    DBG("\r\nbase application number: %d", sys_info.ctrl.base);
    DBG("\r\nchannel mask : %08x", sys_info.channel.mask);
    DBG("\r\nchannel index : %d", sys_info.channel.index);
    DBG("\r\nsoftware version : %d.%02d", (sys_info.ctrl.ver >> 0x08), sys_info.ctrl.ver & 0xff);
    if (emberGetNetworkParameters(&networkParams) == EMBER_SUCCESS)  // ensure that printf the right network parameters
    {
        DBG("\r\nnetwork radio pan id : %04x", networkParams.panId);
        DBG("\r\nnetwork radio channel : %d", networkParams.radioChannel);
    }
    else
    {
        DBG("\r\nnetwork radio pan id : %04x", sys_info.channel.panid);
        DBG("\r\nnetwork radio channel : %d", sys_info.channel.channel);
    }
    DBG("\r\nradio tx power : %d", sys_info.channel.power);
    DBG("\r\nnetwork profile id : %04x", sys_info.channel.profile); 
    DBG("\r\ncurrent system date %d-%d-%d, time %d:%d:%d", single_info.time.year,
                                                           single_info.time.mon,
                                                           single_info.time.day,
                                                           single_info.time.hour,
                                                           single_info.time.min,
                                                           single_info.time.sec);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printlamp ( plamp_t ptr )
//* 功能        : print lamp_t information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printlamp ( plamp_t ptr )
{
    if (!ptr)     return;
    
    if (sys_info.ctrl.app == eLights)
    {
        DBG("\r\nnumber is %d", ptr->light.number);
        DBG("\r\nfailure is %02x", ptr->light.fail);
        DBG("\r\nlamp state is %s", (ptr->light.status == LAMP_OPEN) ? ("Open\0") : ("Close\0"));
        DBG("\r\nlamp automatic control state %s", (ptr->light.manual == TRUE) ? ("No\0") : ("Yes\0"));
        DBG("\r\npower factor is %d", ptr->light.factor);
        DBG("\r\ntotal cost is %d", ptr->light.cost);
        DBG("\r\nlight temperature is %d", ptr->light.temperature.light);
        DBG("\r\ntransformer temperature is %d", ptr->light.temperature.trans);
        DBG("\r\nenvironment temperature is %d", ptr->light.temperature.env);
        DBG("\r\nilluminance is %d", ptr->light.temperature.lux);
        DBG("\r\noutput percent is %d", ptr->light.percent);
        DBG("\r\ncurrent in is %d", ptr->light.current.in);
        DBG("\r\ncurrent out is %d", ptr->light.current.out);
        DBG("\r\nvoltage in is %d", ptr->light.voltage.in);
        DBG("\r\nvoltage out is %d", ptr->light.voltage.out);
    }
    else if (sys_info.ctrl.app == eVehicle)
    {
         if (isCarDevice())
         {
             DBG("\r\ntype number status fail obstacle card magic index count speed");
             DBG("\r\n%4d   %4d   %4x %4x     %4x %4d  %4x  %4d  %4d  %4d", ptr->vehicle.type,
                                                            ptr->vehicle.number,
                                                            ptr->vehicle.status,
                                                            ptr->vehicle.fail,
                                                            ptr->vehicle.obstacle,
                                                            ptr->vehicle.card,
                                                            ptr->vehicle.magic,
                                                            ptr->vehicle.index,
                                                            ptr->vehicle.count,
                                                            ptr->vehicle.speed);
         }
         else if (isCallDevice())
         {
             pbeep_t cal = (pbeep_t)&ptr->vehicle;
             DBG("\r\ntype number status  car  call fail times last assign tick");
             DBG("\r\n%4d   %4d   %4d %4d  %4d %4d  %4d %4d   %4d %4d", cal->type,
                                                            cal->number,
                                                            cal->status,
                                                            cal->car,
                                                            cal->call,
                                                            cal->fail,
                                                            cal->times,
                                                            cal->last,
                                                            cal->assign,
                                                            cal->tick);
         }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printsingle ( void )
//* 功能        : print single_info_t information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printsingle ( void )
{
    DBG("\r\n------------lamp_info---------------");
    DBG("\r\nlogic address: %04x", single_info.address);
    DBG("\r\nlamp current control: %02x", single_info.ctrl);
    DBG("\r\nsink control mode: %02x", sys_info.ctrl.config);
    DBG("\r\ndate time: %d-%d-%d %d:%d:%d", single_info.time.year, single_info.time.mon, single_info.time.day, 
                                                   single_info.time.hour, single_info.time.min, single_info.time.sec); 
    sea_printlamp(&single_info.lamp);
    DBG("\r\n------------end---------------------");
}

////////////////////////////////////////////////////////////////////////////////
