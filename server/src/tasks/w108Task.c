#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"
#include "frame.h"
#include "network.h"
#include "gprs.h"
#include "cmd.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;     // for console protocol

/*********************************************************
* ������      : void sea_initw108 ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void sea_initw108 ( void )
{
    vu32 ticker = 0x00L;
    vu8  retry  = 0x00;
    
    dyn_info.ready |= W108MASK;
    sea_resetw108();
    sea_printf("\nw108 is initializing, please waiting ...");
    while (dyn_info.ready & W108MASK)     
    {
        ticker ++;
        if (ticker & 0x1000)
        {
            if (dyn_info.ready & ADDRBIT)
                w108frm1.put(&w108frm1, ICHP_SV_ADDRESS, 0x01, sys_info.ctrl.road, dyn_info.buffer); 
            if (dyn_info.ready & CTRLBIT)
                w108frm1.put(&w108frm1, ICHP_SV_CTRL, 0x01, sys_info.ctrl.road, dyn_info.buffer);      
            if (dyn_info.ready & CHNBIT)
                w108frm1.put(&w108frm1, ICHP_SV_CHANNEL, 0x01, sys_info.ctrl.road, dyn_info.buffer);
            if (dyn_info.ready & KEYBIT)
                w108frm1.put(&w108frm1, ICHP_SV_SECKEY, 0x01, sys_info.ctrl.road, dyn_info.buffer);
            if (retry ++ >= 0x05)
                break;
        }
        
        vTaskDelay(0x05 / portTICK_RATE_MS);
    }
    if (dyn_info.ready & W108MASK)
        sea_printf("\nw108 initailze fail %02x", dyn_info.ready & W108MASK);
    else
        sea_printf("\nw108 is initialized, w108 is ready.");
    w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
    reset_serialinfo(get_serialinfo(W108_COM));       // why ? 2014/01/17
    reset_serialinfo(get_serialinfo(CONSOLE_COM));    // why ? 2014/01/17
}

/*********************************************************
* ������      : void W108MsgHandler ( void )
* ����        : ��W108���յ������ݣ�ͨ��at����ת����������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*********************************************************/
void vW108MsgTask ( void *pvParameters )
{
    vu32 ticker  = 0x00L;
    ppath_t      rut;
 
    dyn_info.ready = W108MASK | INITBIT;
    sea_resetw108();
    sea_printf("\nw108 is initializing, please waiting ...");
    reset_serialinfo(get_serialinfo(W108_COM));       // why ? 2014/01/17
    reset_serialinfo(get_serialinfo(CONSOLE_COM));    // why ? 2014/01/17
    
    while (1)
    {
        ticker ++;
        
        if ((ticker % 0x100) == 0x00)
        {
            if (dyn_info.ready & W108MASK)     
            {
                if (dyn_info.ready & ADDRBIT)
                    w108frm1.put(&w108frm1, ICHP_SV_ADDRESS, 0x01, sys_info.ctrl.road, dyn_info.buffer);
                else if (dyn_info.ready & CTRLBIT)
                    w108frm1.put(&w108frm1, ICHP_SV_CTRL, 0x01, sys_info.ctrl.road, dyn_info.buffer);
                else if (dyn_info.ready & CHNBIT)
                    w108frm1.put(&w108frm1, ICHP_SV_CHANNEL, 0x01, sys_info.ctrl.road, dyn_info.buffer);
                else if (dyn_info.ready & KEYBIT)
                    w108frm1.put(&w108frm1, ICHP_SV_SECKEY, 0x01, sys_info.ctrl.road, dyn_info.buffer);
            }
            else if (dyn_info.ready & INITBIT)
            {
                w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
                sea_printf("\nw108 is initialized, w108 is ready.");
                dyn_info.ready &= ~INITBIT;
            }
            else
            {
                if ((rut = sea_findsend()) != NULL)
                    sea_sendroutetable(rut); 
            }
        }

        vTaskDelay(0x5 / portTICK_RATE_MS);
    }
} 

/*******************************************************************************
* Function Name  : portBASE_TYPE xAreW108MsgTasksStillRunning ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
portBASE_TYPE xAreW108MsgTasksStillRunning ( void )
{
    portBASE_TYPE xReturn = 0x00;

    return xReturn;
}

