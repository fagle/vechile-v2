#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// ������ȫ�ֱ���
const systime_t  def_time = { 2013, 0x0c, 0x10, 0x00, 0x00, 0x00, 0x00L};
single_t single_info; 

/////////////////////////////////////////////////////////////////////////////////////////////
//ray:�����Ǳ��ػ�����
//start:��/�صơ�����ʱ�䡢���µƺš���������ٷֱ�
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_openlamp ( void )
//* ����        : ·��ʵ�ֿ��ƹ���;�����Ҫ�޸��Լ�������
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void sea_openlamp ( void * ptr )
{
    if (single_info.lamp.light.status == LAMP_OPEN)
        return;
  
    single_info.lamp.light.percent = ptr == NULL ? MAXPERCENT : *((u8 *)ptr);
    single_info.lamp.light.status = LAMP_OPEN;
    sea_updatepercent(single_info.lamp.light.percent);       //  GPIO_PACLR_REG = (1<<PA4_BIT);  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_closelamp ( void )
//* ����        : ·��ʵ�ֹصƹ���
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void sea_closelamp ( void * ptr )
{
    if (single_info.lamp.light.status == LAMP_CLOSE)
        return;
    
    single_info.lamp.light.percent = ptr == NULL ? 0x00 : *((u8 *)ptr);
    single_info.lamp.light.status  = LAMP_CLOSE;
    sea_updatepercent(single_info.lamp.light.percent);       //  GPIO_PACLR_REG = (1<<PA4_BIT);  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : static void sea_togglelamp ( void )
//* ����        : 
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
static void sea_togglelamp ( void * ptr )
{
    (single_info.lamp.light.status == LAMP_CLOSE) ? (sea_openlamp(ptr)) : (sea_closelamp(ptr)); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatetime ( psystime_t tm )
//* ����        : update time of lamp
//* �������    : psystime_t ptm
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatetime ( psystime_t ptm )
{
    sea_memcpy(&single_info.time, ptm, sizeof(systime_t));
    
    if (single_info.fail(GETFAILBIT, LAMP_ER_TM))
        single_info.fail(CLRFAILBIT, LAMP_ER_TM); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_updatepercent ( u8 percent )
//* ����        : update output percent of lamp
//* �������    : u8 percent
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_updatepercent ( u8 percent )
{
    single_info.lamp.light.percent = (percent >= MAXPERCENT) ? (MAXPERCENT) : (percent <= MINPERCENT ? (0x00) : (percent));
    adjustPWM(single_info.lamp.light.percent * 10);
    set_lampmode(LAMP_CHANGE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void init_singleinfo ( void )
//* ����        : ��ʼ�����ص�״̬��Ϣ
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void init_singleinfo ( void )
{
    sea_memset(&single_info, 0x00, sizeof(single_t));
    if (network_info.type == EMBER_COORDINATOR && network_info.devid == COORDID)
        sea_memcpy(&single_info.time, (void *)&def_time, sizeof(systime_t));
    else
        sea_memset(&single_info.time, 0x00, sizeof(systime_t));
    
    single_info.avail                        = 0x06;    
    single_info.address                      = 0x01;                        // �̵�ַ    
    single_info.ctrl                         = 0x00;                        // control mode
    single_info.min                          = sys_info.ctrl.period;  
    single_info.sec                          = sys_info.ctrl.period;  
    single_info.count                        = 0x00;
    single_info.retry                        = 0x00;
    single_info.on                           = sea_openlamp;
    single_info.off                          = sea_closelamp;
    single_info.toggle                       = sea_togglelamp;
    
    single_info.lamp.light.manual            = TRUE;                        // ��ʼ��Ϊ�ֶ�״̬
    single_info.lamp.light.percent           = MAXPERCENT;
    single_info.lamp.light.cost              = 0x10;                        // ÿСʱ��1.2�ȵ�
    single_info.lamp.light.factor            = 0x08;
    single_info.lamp.light.voltage.in        = 200;                         // ��ѹֵ      
    single_info.lamp.light.voltage.out       = 200;
    single_info.lamp.light.current.in        = 0x0c;                        // ����ֵ
    single_info.lamp.light.current.out       = 0x0c;
    single_info.lamp.light.temperature.light = 0x36;
    single_info.lamp.light.temperature.trans = 0x0c;
    single_info.lamp.light.temperature.env   = 0x08;
    single_info.lamp.light.temperature.lux   = 0x30;
    single_info.lamp.light.fail              = LAMP_ER_NO;                  // no errors 
    single_info.lamp.light.number            = sys_info.dev.num;            // read from flash
}

/////////////////////////////////////////////////////////////////////////////////////

