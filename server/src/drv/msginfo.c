#include "network.h"
#include "lamp.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;     // for console protocol
extern rep_info_t   rep_info;

/////////////////////////////////////////////////////////////////////////////////////
//
msg_info_t         msg_info;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ptime_t  sea_getcurtime ( ptime_t tm )
//* 功能        : get current time
//* 输入参数    : ptime_t tm
//* 输出参数    : time_t pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
ptime_t  sea_getcurtime ( ptime_t tm )
{
    tm->hour = sea_getsystime()->hour;
    tm->min  = sea_getsystime()->min;
    tm->sec  = sea_getsystime()->sec;
    return tm;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCallDevice ( u8 type, u8 num )
//* 功能        : is caller device
//* 输入参数    : u8 type, u8 num
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCallDevice ( u8 type, u8 num )
{
    if (type >= CALLIDST && type < CALLIDST + CALLIDCNT &&
        num && num <= sys_info.ctrl.call)
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCarDevice ( u8 type, u8 num )
//* 功能        : is car device
//* 输入参数    : u8 type, u8 num
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCarDevice ( u8 type, u8 num )
{
    if (type >= CARIDST && type < CARIDST + CARIDCNT &&
        num > sys_info.ctrl.base && num <= sys_info.ctrl.base + sys_info.ctrl.car)
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCarActive ( u8 num )
//* 功能        : is car active
//* 输入参数    : u8 num
//* 输出参数    : ture/false
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCarActive ( u8 num )
{
    if (isCarDevice(CARIDST, num))
    {
        time_t tm;
        if (rep_info.key[num - 0x01].vehicle.status == 0x00)
            return (bool)FALSE;
        else 
        {
            sea_getcurtime(&tm);
            if (sea_caltime(&rep_info.time[num - 0x01]) + 60 * 0x02 >= sea_caltime(&tm))  // 2 minute
                return (bool)TRUE;
        }
    }
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_getcartype ( u8 num )
//* 功能        : get car type
//* 输入参数    : u8 num
//* 输出参数    : type
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_getcartype ( u8 num )
{
    if (isCarDevice(CARIDST, num))
        return rep_info.key[num - 0x01].vehicle.type;
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_getcurrentcard ( u8 num )
//* 功能        : get car current card
//* 输入参数    : u8 num
//* 输出参数    : card
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_getcurrentcard ( u8 num )
{
    if (isCarDevice(CARIDST, num))
        return rep_info.key[num - 0x01].vehicle.card;
    return 0x00;
}

/*******************************************************************************
* Function Name  : static void sea_routefind ( u8 num ) 
* Description    : find route table
* Input          : u8 num
* Output         : path pointer
* Return         : None
*******************************************************************************/
static ppath_t sea_routefind ( u8 num )      
{
    if (num < sys_info.ctrl.base || num > sys_info.ctrl.maxdev)
        return NULL;
    
    return (ppath_t)rep_info.goal[num - 0x01];
}

/*******************************************************************************
* Function Name  : static u8  sea_findaction ( ppath_t ptr, u8 card )  
* Description    : find action on card id
* Input          : ppath_t ptr, u8 card
* Output         : action
* Return         : None
*******************************************************************************/
static u8  sea_findaction ( ppath_t ptr, u8 card )      
{
    if (ptr == NULL)
        return 0x00;
    
    for (u8 i = 0x00; i < ptr->cnt; i ++)
    {
        if (ptr->line[i].id == card)
            return ptr->line[i].action;
    }
    return 0x00;
}

/*******************************************************************************
* Function Name  : s8  sea_findindex ( ppath_t ptr, u8 card )   
* Description    : find action on card id
* Input          : ppath_t ptr, u8 card
* Output         : index
* Return         : None
*******************************************************************************/
s8  sea_findindex ( ppath_t ptr, u8 card )      
{
    if (ptr == NULL)
        return 0xff;
    
    for (u8 i = 0x00; i < ptr->cnt; i ++)
    {
        if (ptr->line[i].id == card)
            return i;
    }
    return 0xff;
}

/*******************************************************************************
* Function Name  : ppath_t sea_findsend ( void )
* Description    : find send route table
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ppath_t sea_findsend ( void )      
{
    for (u8 i = 0x00; i < sys_info.ctrl.car; i ++)
    {
        if (msg_info.route[i].send)
            return &msg_info.route[i];
    }
    return NULL;
}

/*******************************************************************************
* Function Name  : static u8 sea_routeadd ( u8 num, u8 index, u8 size, u8 * ptr ) 
* Description    : add route table
* Input          : u8 num, u8 index, u8 size, u8 * ptr
* Output         : true/false
* Return         : None
*******************************************************************************/
static u8 sea_routeadd ( u8 num, u8 index, u8 size, u8 * data )      
{
    ppath_t ptr = msg_info.find(num);
    u8      i;

    if (ptr == NULL || size == 0x00)
        return 0x00;
    
    for (i = 0x00; i < size; i ++)
    {
        if (!data[i])
            return 0x00;
#if 0        
        if (i & 0x01)
        {
            if (data[i] == CAR_RIGHT)
                data[i] = CAR_LEFT;
            else if (data[i] == CAR_LEFT)
                data[i] = CAR_RIGHT;
        }
#endif
    }

    if (index == 0x00)
        msg_info.clear(num);

    sea_memcpy(&ptr->line[index], data, size);
    ptr->cnt = index + size / sizeof(action_t);
    
    return 0x01;
}

/*******************************************************************************
* Function Name  : static void sea_routeadd ( u8 num ) 
* Description    : clear route table
* Input          : u8 num
* Output         : None
* Return         : None
*******************************************************************************/
static void sea_routeclear ( u8 num )      
{
    for (u8 i = 0x00; i < sys_info.ctrl.car; i ++)
    {
        if (msg_info.route[i].num == num)
        {
            msg_info.route[i].cnt    = 0x00;
            msg_info.route[i].send   = 0x00;
            msg_info.route[i].reboot = 0x00;
            msg_info.route[i].status = 0x00;
            sea_memset(msg_info.route[i].line, 0x00, MAXPATH * sizeof(action_t));
            return;
        }
    }
}

/*******************************************************************************
* Function Name  : static void sea_routeprint ( u8 num ) 
* Description    : print route table
* Input          : u8 num
* Output         : None
* Return         : None
*******************************************************************************/
static void sea_routeprint ( u8 num )      
{
    u8 i;
    ppath_t ptr = msg_info.find(num);
    
    if (ptr == NULL || ptr->cnt == 0x00)
        return;
    
    sea_printf("\nvechicle %d, route size %d\n    id:", num, ptr->cnt);
    for (i = 0x00; i < ptr->cnt; i ++)
        sea_printf("%2x ", ptr->line[i].id);
    sea_printf("\naction:");
    for (i = 0x00; i < ptr->cnt; i ++)
        sea_printf("%2x ", ptr->line[i].action);
}

/*******************************************************************************
* Function Name  : void sea_initmsg ( void )
* Description    : initialize msg_info structure body 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_initmsg ( void )
{
    sea_memset(&msg_info, 0x00, sizeof(msg_info_t));
    msg_info.maxcard  = MAXCARDS;
      
    msg_info.find   = sea_routefind;
    msg_info.add    = sea_routeadd;
    msg_info.clear  = sea_routeclear;
    msg_info.print  = sea_routeprint;
    msg_info.action = sea_findaction;
    
    sea_initraffic();
    sea_initreport();
    
    for (u8 i = 0x00; i < MAXLAMPS; i ++)
    {
        if (i < CALLERDEVS)
        {
            msg_info.call[i].num = i + 0x01;
            rep_info.goal[i] = &msg_info.call[i];
        }
        else if (i >= sys_info.ctrl.base && i < sys_info.ctrl.base + MOBILEDEVS)
        {
            msg_info.route[i - sys_info.ctrl.base].num = i + 0x01;
            rep_info.goal[i] = &msg_info.route[i - sys_info.ctrl.base];
        }
        else
            rep_info.goal[i] = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Function Name  : void sea_reboot ( const char * msg )
* Description    : display system error message and reboot
* Input          : const char * msg
* Output         : None
* Return         : None
*******************************************************************************/
void sea_reboot ( const char * msg )
{
   #ifndef USE_TOOL_KEIL
   extern void __iar_program_start ( void );
   
   sea_printf(msg);
   __iar_program_start();
   #else
       extern void Reset_Handler(void);
       sea_printf(msg);
	   Reset_Handler();
   #endif
}

/*******************************************************************************
* Function Name  : void sea_resetw108 ( void )
* Description    : w108 reboot
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void sea_resetw108 ( void )
{
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);       // reset w108
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);   
    delay(0x01);
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);   
}

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : void sea_sendroutetable ( ppath_t ptr )
//* 功能        : send route table to w108
//* 输入参数    : ppath_t ptr
//* 输出参数    : true/flase
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  sea_sendroutetable ( ppath_t ptr )   
{
    u8  buffer[(RTSENDSIZE + 0x01) * 0x02];
    u8  size = RTSENDSIZE;
    u16 addr;
    
    if (ptr == NULL || ptr->cnt == 0x00)
        return 0x00;
    
    if ((addr = dyn_info.addr[ptr->num - 0x01].logic) == 0x00) 
        return 0x00;

    for (u8 i = 0x00; i < ptr->cnt; i += size)
    {
        if (ptr->cnt < RTSENDSIZE + i)
            size = ptr->cnt - i;
        buffer[0x00] = ptr->num;
        buffer[0x01] = i;
        sea_memcpy(&buffer[0x02], &ptr->line[i], size * sizeof(action_t));
        w108frm1.put(&w108frm1, ICHP_SV_ROUTE, (size + 0x01) * 0x02, addr, buffer);
    }
    
    if (ptr->send)
        ptr->send = 0x00;
    return 0x01;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printEUI64 ( EmberEUI64 * eui )
//* 功能        : utility for printing EUI64 addresses 
//* 输入参数    : EmberEUI64 * eui
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printEUI64 ( EmberEUI64 * eui ) 
{
    u8 * ptr = (u8 *)eui;
    
    for (u8 i = EUI64_SIZE; i > 0x00; i --) 
        sea_printf("%02x", ptr[i - 0x01]);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8  isEUI64 ( EmberEUI64 * eui ) 
//* 功能        : is EUI64 addresses 
//* 输入参数    : EmberEUI64 * eui
//* 输出参数    : true/false
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  isEUI64 ( EmberEUI64 * eui ) 
{
    u8 * ptr = (u8 *)eui;
    u8   i, cnt;
    
    for (cnt = i = 0x00; i < EUI64_SIZE; i ++) 
    {
        if (ptr[i] == 0x00)
            cnt ++;
    }
    return cnt <= 0x04;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_lookuphysical ( EmberEUI64 * eui )
//* 功能        : look up number from physical address
//* 输入参数    : EmberEUI64 * eui
//* 输出参数    : number
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_lookuphysical ( EmberEUI64 * eui )
{
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
    {
        if (sea_memcomp(&dyn_info.addr[i].dev.eui, eui, EUI64_SIZE) == 0x00)
            return i + 0x01;
    }
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_lookuplogic ( u16 logic )
//* 功能        : look up number from logic address
//* 输入参数    : EmberEUI64 * eui
//* 输出参数    : number
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_lookuplogic ( u16 logic )
{
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
    {
        if (dyn_info.addr[i].logic == logic)
            return i + 0x01;
    }
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printaddress ( void )
//* 功能        : print address information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printaddress ( void )
{
    sea_printf("\ntype number  logic  physical");
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
    {
        if (dyn_info.addr[i].logic && dyn_info.addr[i].dev.num && dyn_info.addr[i].dev.type)
        {
            sea_printf("\n%4d   %4d  %5d  ", dyn_info.addr[i].dev.type, dyn_info.addr[i].dev.num, dyn_info.addr[i].logic);
            printEUI64(&dyn_info.addr[i].dev.eui);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sea_printreport( plamp_t ptr )
//* 功能        : print report information
//* 输入参数    : plamp_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printreport ( plamp_t ptr )
{
    if (sys_info.ctrl.app == eLights)
    {
        sea_printf("\nnumber is %d", ptr->light.number);
        sea_printf("\nfailure is %02x", ptr->light.fail);
        sea_printf("\nlamp state is %s", (ptr->light.status == LAMP_OPEN) ? ("Open\0") : ("Close\0"));
        sea_printf("\nlamp automatic control state %s", (ptr->light.manual == TRUE) ? ("No\0") : ("Yes\0"));
        sea_printf("\npower factor is %d", ptr->light.factor);
        sea_printf("\ntotal cost is %d", ptr->light.cost);
        sea_printf("\nlight temperature is %d", ptr->light.temperature.light);
        sea_printf("\ntransformer temperature is %d", ptr->light.temperature.trans);
        sea_printf("\nenvironment temperature is %d", ptr->light.temperature.env);
        sea_printf("\nilluminance is %d", ptr->light.temperature.lux);
        sea_printf("\noutput percent is %d", ptr->light.percent);
        sea_printf("\ncurrent in is %d", ptr->light.current.in);
        sea_printf("\ncurrent out is %d", ptr->light.current.out);
        sea_printf("\nvoltage in is %d", ptr->light.voltage.in);
        sea_printf("\nvoltage out is %d", ptr->light.voltage.out);
    }
    else if (sys_info.ctrl.app == eVehicle)
    {
        if (dyn_info.addr[ptr->vehicle.number - 0x01].logic && 
            ptr->vehicle.number > 0x00 && ptr->vehicle.number <= sys_info.ctrl.maxdev)
        {
            sea_printf("\n%4d   %4d   %4x %4x     %4x %4d  %4x  %4d  %4d  %4d", ptr->vehicle.type,
                                                            ptr->vehicle.number,
                                                            ptr->vehicle.status,
                                                            ptr->vehicle.fail,
                                                            ptr->vehicle.obstacle,
                                                            ptr->vehicle.card,
                                                            ptr->vehicle.magic,
                                                            ptr->vehicle.index,
                                                            ptr->vehicle.count,
                                                            ptr->vehicle.speed);
            sea_printf(" %2d:%2d", sea_getsystime()->min, sea_getsystime()->sec);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////




