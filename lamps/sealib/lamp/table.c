#include "network.h"
#include "flash.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
table_t   table_info;

/////////////////////////////////////////////////////////////////////////////////////////////
// 分配给每个人的程序有不同的网络号，防止分配错误。
// (19, 0x1010) 莫雪娟实验
// (19, 0x0505) 实验室路灯组网实验
// (21, 0x0a0a) 实验室网关实验主网络
// (23, 0x0f0f) 实验室网关实验辅网络
// (17, 0x7777) 郭太磊实验
// (17, 0x7777) 第二次寄望湖州的网络号
const panid_t panid_info[] = { {11, 0x1212}, {12, 0x0f0f}, {13, 0x1010}, {14, 0x0808}, 
                               {15, 0x5858}, {16, 0x0a0a}, {23, 0x0101}, {18, 0x6868}, 
                               {19, 0xa0a0}, {20, 0x9090}, {21, 0x9898}, {22, 0x0505}, 
                               {17, 0x7070}, {24, 0x5050}, {25, 0xf0f0}, {26, 0x0606},
                               {17, 0x7676}, {19, 0x6767}, {18, 0x5a5a}, {23, 0xa5a5}, };

   
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask )
//* 功能        : write default system information configuration
//* 输入参数    : u8 app, u8 type, u8 index, u32 mask
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask )
{
    u8  retry = 0x05; 

    Debug("\r\nwrite default system information to flash.");
    sea_memset(&sys_info, 0x00, sizeof(sys_info_t));
    
    sys_info.channel.index   = index;
    sys_info.channel.mask    = mask;                                         // 0x0000ffff, 0x10 channels
    sys_info.channel.panid   = panid_info[sys_info.channel.index].panid;     // DEF_PANID;
    sys_info.channel.channel = panid_info[sys_info.channel.index].channel;   // DEF_CHANNEL;
    sys_info.channel.power   = DEF_POWER;
    sys_info.channel.profile = DEF_PROFILE;
    sea_memcpy(sys_info.key.link, DEF_LINKKEY, EMBER_ENCRYPTION_KEY_SIZE);
    sea_memcpy(sys_info.key.network, DEF_NETWORKKEY, EMBER_ENCRYPTION_KEY_SIZE);
    sea_memcpy(sys_info.channel.expanid, DEF_EXPAN, EXTENDED_PAN_ID_SIZE);
    
    sys_info.mark         = SYSMARK;
    sys_info.size         = sizeof(sys_info_t);
    sys_info.dev.type     = type;
    sys_info.ctrl.maxdev  = EMBER_MAXDEVS;
    sys_info.ctrl.road    = DEFALUTROADID;
    sys_info.ctrl.ver     = SWVERSION;
    sys_info.ctrl.app     = app;                       // agv application, added 2013/12/14
    sys_info.ctrl.period  = DEFALUTPERIOD;             // report lamp information time(sec)  
    sys_info.ctrl.config  = LAMP_CF_OPEN;              // default config with auto mode
    sys_info.ctrl.base    = MOBILEBASE;                // base number of vehicles
    sys_info.ctrl.car     = MOBILEDEVS;
    sys_info.ctrl.call    = CALLERDEVS;
    sys_info.ctrl.type    = 0x04;                      // 5,6,7,8 types
    sys_info.ctrl.release = 0x00;                      // debug version
    if (app == eVehicle && type >= CARIDST && type < CARIDST + CARIDCNT)
        sys_info.dev.num = sys_info.ctrl.base + DEFALUTLAMPNUM;            // 自己的物理编号，从手持设备中获得 
    else
        sys_info.dev.num = DEFALUTLAMPNUM;            // 自己的物理编号，从手持设备中获得 
      
    while (retry --)
    {
        if (sea_flashwrite(0x00, (u16 *)&sys_info, sizeof(sys_info_t)) == EMBER_SUCCESS)
            break;;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
//* 功能        : update channel information configuration
//* 输入参数    : u8 num, u8 type, u8 index, u32 mask
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
{
    u8  retry = 0x05; 

    sys_info.channel.index   = index;
    sys_info.channel.mask    = mask;                                         // 0x0000ffff, 0x10 channels
    sys_info.channel.panid   = panid_info[sys_info.channel.index].panid;     // DEF_PANID;
    sys_info.channel.channel = panid_info[sys_info.channel.index].channel;   // DEF_CHANNEL;
    sys_info.dev.type        = type;
    sys_info.dev.num         = num;                                          // 自己的物理编号，从手持设备中获得 
      
    while (retry --)
    {
        if (sea_flashwrite(0x00, (u16 *)&sys_info, sizeof(sys_info_t)) == EMBER_SUCCESS)
            break;;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_updatesysconfig ( void )
//* 功能        : update system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_updatesysconfig ( void )
{
    sys_info_t ptr;
    u8  retry = 0x05;
    
    if (table_info.clear)
        table_info.clear();

    sea_flashread(0x00, sizeof(sys_info_t), &ptr);
    if (sea_memcomp(&sys_info, &ptr, sizeof(sys_info_t)) != 0x00)
    {
        while (retry --)
        {
            if (sea_flashwrite(0x00, (u16 *)&sys_info, sizeof(sys_info_t)) == EMBER_SUCCESS)
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_adjustchannel ( u8 index )
//* 功能        : change radio channel and panid
//* 输入参数    : u8 index
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_adjustchannel ( u8 index )
{
    if (sys_info.channel.index >= MAXCHANNEL)
        return;
    sys_info.channel.index   = index;
    sys_info.channel.mask    = 0x01 << sys_info.channel.index;
    sys_info.channel.panid   = panid_info[sys_info.channel.index].panid;   
    sys_info.channel.channel = panid_info[sys_info.channel.index].channel; 
    sea_updatesysconfig();
   
    EZSP_Configuration();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_writetotable ( u32 offset, void * data, u16 size )
//* 功能        : write data flash
//* 输入参数    : u32 offset, u16 * data, u16 size
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_writetotable ( u32 addr, void * data, u16 size )
{
    u8  retry = 0x05;
    
    while (retry --)
    {
        if (sea_tableflashwrite(addr, data, size) == EMBER_SUCCESS)
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_findchannelindex ( u8 channel, u16 pandid )
//* 功能        : find index of radio channel and panid
//* 输入参数    : u8 channel, u16 pandid 
//* 输出参数    : index
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_findchannelindex ( u8 channel, u16 panid )
{
    for (u8 i = 0x00; i < MAXCHANNEL; i ++)
    {
        if (panid == panid_info[i].panid && channel == panid_info[i].channel)
            return i; 
    }
    return 0xff;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void * sea_nextable ( u8 type )
//* 功能        : get next device_t/passed_t in flash table
//* 输入参数    : u8 type
//* 输出参数    : pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void * sea_nextable ( u8 type )
{
    u16 addr = 0x00;
    
    table_info.index ++;
    if (table_info.index >= table_info.size)
    {
        table_info.index = table_info.size;
        return NULL;
    }
    
    if (type == TBDEVICE)
        addr += sizeof(device_t) * table_info.index;
    else if (type == TBPASSED)
        addr += sizeof(passed_t) * table_info.index;
    
    if (sea_flashreadshort(TABLEOFFSET + addr) == ENDTABLE)
        return NULL;
    return (void *)(STUSERADDR + TABLEOFFSET + addr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void * sea_lastable ( u8 type )
//* 功能        : get last device_t/passed_t in flash table
//* 输入参数    : u8 type
//* 输出参数    : pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void * sea_lastable ( u8 type )
{
    u16 addr = 0x00;
    
    if (table_info.index)
        table_info.index --;
    else 
        return NULL;
    
    if (type == TBDEVICE)
        addr += sizeof(device_t) * table_info.index;
    else if (type == TBPASSED)
        addr += sizeof(passed_t) * table_info.index;
    
    if (sea_flashreadshort(TABLEOFFSET + addr) == ENDTABLE)
        return NULL;
    return (void *)(STUSERADDR + TABLEOFFSET + addr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void * sea_firstable ( void )
//* 功能        : get first device_t/passed_t in flash table
//* 输入参数    : 无
//* 输出参数    : pointer
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void * sea_firstable ( void )
{
    table_info.index = 0x00;

    if (sea_flashreadshort(TABLEOFFSET) == ENDTABLE)
        return NULL;
    return (void *)(STUSERADDR + TABLEOFFSET);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void * sea_firstable ( void )
//* 功能        : get first device_t/passed_t in flash table
//* 输入参数    : 无
//* 输出参数    : EmberStatus
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static EmberStatus sea_cleartable ( void )
{
    u16   addr;
    
    table_info.index = 0x00;
    table_info.cnt   = 0x00;
    for (addr = 0x00; addr < TABLESIZE; addr += PAGESIZE)
    {
        if (sea_tableflasherase(addr + STUSERADDR + TABLEOFFSET) != EMBER_SUCCESS)
            return EMBER_ERR_FLASH_PROG_FAIL;
    }
    return EMBER_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u16 sea_getcount ( u8 type )
//* 功能        : get count of device or passed in flash table
//* 输入参数    : u8 type
//* 输出参数    : count
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u16 sea_getcount ( u8 type )
{
    u32  addr = (STUSERADDR + TABLEOFFSET);
    u16  cnt = 0x00;
    u8   size;
    
    size = (type == TBDEVICE) ? sizeof(device_t) : sizeof(passed_t);
    while (addr < STUSERADDR + TABLEOFFSET + TABLESIZE)
    {
        if (*(u16 *)addr == ENDTABLE)
            return cnt;
        cnt ++;
        addr += size;
    }
    return cnt;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void  init_tableinfo ( void )
//* 功能        : initialize table body
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  init_tableinfo ( void )
{
    sea_memset(&table_info, 0x00, sizeof(table_t));
    table_info.first = sea_firstable;
    table_info.next  = sea_nextable;
    table_info.last  = sea_lastable;
    table_info.clear = sea_cleartable;
    table_info.count = sea_getcount;
    
    table_info.index = 0x00;
//    if (sys_info.dev.type == CENTERID)
    if (sys_info.dev.type >= CENTERIDST && sys_info.dev.type <= CENTERIDCNT)
    {
        table_info.size  = TABLESIZE / sizeof(pdevice_t);
        table_info.cnt   = table_info.count(TBDEVICE);
    }
    else
    {
        table_info.size  = TABLESIZE / sizeof(passed_t);
        table_info.cnt   = table_info.count(TBPASSED);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_isdeviceintable ( pdevice_t ptr )
//* 功能        : find device in flash table
//* 输入参数    : pdevice_t ptr
//* 输出参数    : true/flase
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_isdeviceintable ( pdevice_t ptr )
{
    pdevice_t  tmp = (pdevice_t)(STUSERADDR + TABLEOFFSET);

    for (u16 i = 0x00; i < table_info.size; i ++, tmp ++)
    {
        if (sea_memcomp(tmp, ptr, sizeof(device_t)) == 0x00)
            return 0x01;
    }
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pdevice_t sea_findphydevice ( EmberEUI64 * eui )
//* 功能        : find the device_t in flash table
//* 输入参数    : EmberEUI64 * eui
//* 输出参数    : pointer of device_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pdevice_t sea_findphydevice ( EmberEUI64 * eui, pdevice_t ptr )
{
    pdevice_t  tmp = (pdevice_t)(STUSERADDR + TABLEOFFSET);

    for (u16 i = 0x00; i < table_info.size; i ++, tmp ++)
    {
        if (sea_memcomp(tmp->eui, eui, EUI64_SIZE) == 0x00)
        {
            sea_memcpy(ptr, tmp, sizeof(device_t));
            return ptr;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr )
//* 功能        : find the device_t in flash table
//* 输入参数    : u16 num, pdevice_t ptr
//* 输出参数    : pointer of device_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr )
{
    pdevice_t  tmp = (pdevice_t)(STUSERADDR + TABLEOFFSET);

    for (u16 i = 0x00; i < table_info.size; i ++, tmp ++)
    {
        if (tmp->num == num)
        {
            sea_memcpy(ptr, tmp, sizeof(device_t));
            return ptr;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_savedevice ( pdevice_t ptr )
//* 功能        : save new device_t to flash table
//* 输入参数    : pdevice_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_savedevice ( pdevice_t ptr )
{
    pdevice_t  tmp = (pdevice_t)(STUSERADDR + TABLEOFFSET);

    tmp += table_info.cnt;
    sea_writetotable((u32)tmp, ptr, sizeof(device_t));
    table_info.cnt ++;
    if (table_info.cnt >= table_info.size)
        table_info.cnt = 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_ispassedintable ( ppassed_t ptr )
//* 功能        : is passed_t in flash table
//* 输入参数    : ppassed_t ptr
//* 输出参数    : true/flash
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_ispassedintable ( ppassed_t ptr )
{
    ppassed_t  tmp = (ppassed_t)(STUSERADDR + TABLEOFFSET);
    
    for (u16 i = 0x00; i < table_info.size; i ++, tmp ++)
    {
        if (sea_memcomp(tmp, ptr, sizeof(passed_t)) == 0x00)
            return 0x01;
        if (*(u16 *)tmp == ENDTABLE)
            break;
    }
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ppassed_t sea_finpassed ( u8 card )
//* 功能        : find the last passed_t in flash table
//* 输入参数    : u8 card
//* 输出参数    : pointer of passed_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
ppassed_t sea_finpassed ( u8 card )
{
    ppassed_t  tmp = (ppassed_t)(STUSERADDR + TABLEOFFSET);
    ppassed_t  qtr = NULL;
    
    for (u16 i = 0x00; i < table_info.size; i ++, tmp ++)
    {
        if (tmp->act.id == card)
            qtr = tmp;
        if (*(u16 *)tmp == ENDTABLE)
            break;
    }
    return qtr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_writedefaultconfig ( void )
//* 功能        : write default system information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_savepassed ( ppassed_t ptr )
{
    ppassed_t  tmp = (ppassed_t)(STUSERADDR + TABLEOFFSET);

    tmp += table_info.cnt;
    sea_writetotable((u32)tmp, ptr, sizeof(passed_t));
    table_info.cnt ++;
    if (table_info.cnt >= table_info.size)
        table_info.cnt = 0x00;
}

//////////////////////////////////////////////////////////////////////////////////////
