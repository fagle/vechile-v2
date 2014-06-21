#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"
#include "frame.h"
#include "network.h"
#include "cmd.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1;     // for console protocol

/*********************************************************
* 函数名      : void sea_initw108 ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
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
* 函数名      : void W108MsgHandler ( void )
* 功能        : 从W108接收到的数据，通过at命令转发给服务器
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*********************************************************/
void vW108MsgTask ( void *pvParameters )
{
    vu32 ticker  = 0x00L;
 
    dyn_info.ready = W108MASK | INITBIT;
    sea_resetw108();
    sea_printf("\nw108 is initializing, please waiting ...");
    reset_serialinfo(get_serialinfo(W108_COM));       // why ? 2014/01/17
    reset_serialinfo(get_serialinfo(CONSOLE_COM));    // why ? 2014/01/17
    
    while (1)
    {
        ticker ++;
        
        if ((ticker % 0x400) == 0x00)      // ~1 second
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
                TaskCmdHandler();                         // ~1 second handler
                if ((ticker % 0x10000) == 0x00)           // ~2 minute handler
                {
	            for (u8 ch = 0x00; ch < sys_info.ctrl.car; ch ++)
                    {
                        if (dyn_info.addr[sys_info.ctrl.base + ch].logic)
                        {
                            if (isCarActive(sys_info.ctrl.base + ch + 0x01))
                                set_bitmap(msg_info.act, ch);   // sea_printf("\n%dth vehicle is alive now.", sys_info.ctrl.base + ch + 0x01);
                            else if (get_bitmap(msg_info.act, ch))
                            {
                                dyn_info.buffer[0x00] = sys_info.ctrl.base + ch + 0x01;
                                w108frm1.put(&w108frm1, ICHP_SV_RPT, 0x01, 0x00, dyn_info.buffer);
                                clr_bitmap(msg_info.act, ch);   // sea_printf("\n%dth vehicle is power off now.", sys_info.ctrl.base + ch + 0x01);
                            }
                        }
                    }
                }
            }
        }

        vTaskDelay(0x01 / portTICK_RATE_MS);
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

