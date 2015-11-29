#include "network.h"
#include "flash.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
table_t   table_info;

/////////////////////////////////////////////////////////////////////////////////////////////
// �����ÿ���˵ĳ����в�ͬ������ţ���ֹ�������
// (19, 0x1010) Īѩ��ʵ��
// (19, 0x0505) ʵ����·������ʵ��
// (21, 0x0a0a) ʵ��������ʵ��������
// (23, 0x0f0f) ʵ��������ʵ�鸨����
// (17, 0x7777) ��̫��ʵ��
// (17, 0x7777) �ڶ��μ������ݵ������
const panid_t panid_info[] = { {11, 0x1212}, {12, 0x0f0f}, {13, 0x1010}, {14, 0x0808}, 
                               {15, 0x5858}, {16, 0x0a0a}, {23, 0x0101}, {18, 0x6868}, 
                               {19, 0xa0a0}, {20, 0x9090}, {21, 0x9898}, {22, 0x0505}, 
                               {17, 0x7070}, {24, 0x5050}, {25, 0xf0f0}, {26, 0x0606},
                               {17, 0x7676}, {19, 0x6767}, {18, 0x5a5a}, {23, 0xa5a5}, };

   
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_writedefaultconfig ( u8 app, u8 type, u8 index, u32 mask )
//* ����        : write default system information configuration
//* �������    : u8 app, u8 type, u8 index, u32 mask
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
        sys_info.dev.num = sys_info.ctrl.base + DEFALUTLAMPNUM;            // �Լ��������ţ����ֳ��豸�л�� 
    else
        sys_info.dev.num = DEFALUTLAMPNUM;            // �Լ��������ţ����ֳ��豸�л�� 
      
    while (retry --)
    {
        if (sea_flashwrite(0x00, (u16 *)&sys_info, sizeof(sys_info_t)) == EMBER_SUCCESS)
            break;;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
//* ����        : update channel information configuration
//* �������    : u8 num, u8 type, u8 index, u32 mask
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatechannel ( u8 num, u8 type, u8 index, u32 mask )
{
    u8  retry = 0x05; 

    sys_info.channel.index   = index;
    sys_info.channel.mask    = mask;                                         // 0x0000ffff, 0x10 channels
    sys_info.channel.panid   = panid_info[sys_info.channel.index].panid;     // DEF_PANID;
    sys_info.channel.channel = panid_info[sys_info.channel.index].channel;   // DEF_CHANNEL;
    sys_info.dev.type        = type;
    sys_info.dev.num         = num;                                          // �Լ��������ţ����ֳ��豸�л�� 
      
    while (retry --)
    {
        if (sea_flashwrite(0x00, (u16 *)&sys_info, sizeof(sys_info_t)) == EMBER_SUCCESS)
            break;;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatesysconfig ( void )
//* ����        : update system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : void sea_adjustchannel ( u8 index )
//* ����        : change radio channel and panid
//* �������    : u8 index
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static void sea_writetotable ( u32 offset, void * data, u16 size )
//* ����        : write data flash
//* �������    : u32 offset, u16 * data, u16 size
//* �������    : EmberStatus status
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : u8 sea_findchannelindex ( u8 channel, u16 pandid )
//* ����        : find index of radio channel and panid
//* �������    : u8 channel, u16 pandid 
//* �������    : index
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static void * sea_nextable ( u8 type )
//* ����        : get next device_t/passed_t in flash table
//* �������    : u8 type
//* �������    : pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static void * sea_lastable ( u8 type )
//* ����        : get last device_t/passed_t in flash table
//* �������    : u8 type
//* �������    : pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static void * sea_firstable ( void )
//* ����        : get first device_t/passed_t in flash table
//* �������    : ��
//* �������    : pointer
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static void * sea_firstable ( void )
//* ����        : get first device_t/passed_t in flash table
//* �������    : ��
//* �������    : EmberStatus
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : static u16 sea_getcount ( u8 type )
//* ����        : get count of device or passed in flash table
//* �������    : u8 type
//* �������    : count
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : void  init_tableinfo ( void )
//* ����        : initialize table body
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : u8 sea_isdeviceintable ( pdevice_t ptr )
//* ����        : find device in flash table
//* �������    : pdevice_t ptr
//* �������    : true/flase
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : pdevice_t sea_findphydevice ( EmberEUI64 * eui )
//* ����        : find the device_t in flash table
//* �������    : EmberEUI64 * eui
//* �������    : pointer of device_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : pdevice_t sea_findnumdevice ( u16 num, pdevice_t ptr )
//* ����        : find the device_t in flash table
//* �������    : u16 num, pdevice_t ptr
//* �������    : pointer of device_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : void sea_savedevice ( pdevice_t ptr )
//* ����        : save new device_t to flash table
//* �������    : pdevice_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : u8 sea_ispassedintable ( ppassed_t ptr )
//* ����        : is passed_t in flash table
//* �������    : ppassed_t ptr
//* �������    : true/flash
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : ppassed_t sea_finpassed ( u8 card )
//* ����        : find the last passed_t in flash table
//* �������    : u8 card
//* �������    : pointer of passed_t
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
//* ������      : void sea_writedefaultconfig ( void )
//* ����        : write default system information configuration
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
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
