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
extern frame_info_t consfrm1, w108frm1;
extern msg_info_t   msg_info;

/*******************************************************************************
* Function Name  : portBASE_TYPE xAreMiscTasksStillRunning ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
portBASE_TYPE xAreMiscTasksStillRunning ( void )
{
    portBASE_TYPE xReturn = 0x00;
    
    return xReturn;
}

/*******************************************************************************
* Function Name  : void MiscellaneaHandler ( void )
* Description    : None
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void vMiscellaneaTask ( void *pvParameters )
{
    vu32 ticker = 0x00L;
    u8          ch;
    frame_t     frm;
    
    while (1)
    {
        ticker ++;

        if (!sea_isempty(get_serialinfo(consfrm1.uart)))    
        {
            ch = sea_readbyte(get_serialinfo(consfrm1.uart));
            consfrm1.recv(&consfrm1, ch);
            if (consfrm1.state == PRT_IDLE)
                keyboardinput(ch);
        }
        
        if (consfrm1.get(&consfrm1, &frm) != NULL)
            ServerFrameCmdHandler(frm);
        
        if (w108frm1.get(&w108frm1, &frm) != NULL)  
             CoordFrameCmdHandler(frm);
        
        if ((ticker % 0x10000) == 0x00)           // ~2 minute handler
        {
	    for (ch = 0x00; ch < sys_info.ctrl.car; ch ++)
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
        
        if ((ticker % 0x1000) == 0x00)           // ~4 second
        {
            for (ch = 0x00; ch < sys_info.ctrl.call; ch ++)
            {
                pcall_t cal = (pcall_t)rep_info.goal[ch];
                if (cal->ack)
                {
                    cal->cnt        = 0x00;
                    cal->body[0x00] = cal->num;
                    cal->body[0x01] = cal->type;
                    cal->body[0x02] = cal->state;
                    cal->body[0x03] = cal->vehicle;
                    if (cal->logic[0x00])
                    {
                        cal->cnt ++;
                        w108frm1.put(&w108frm1, ICHP_SV_ASSIGN, 0x04, cal->logic[0x00], cal->body);
                    }
                    if (cal->logic[0x01])
                    {
                        cal->cnt ++;
                        w108frm1.put(&w108frm1, ICHP_SV_ASSIGN, 0x04, cal->logic[0x01], cal->body);
                    }
                }
            }
        }
        
#ifdef TRAFFIC_ENABLE
        if ((ticker % 0x800) == 0x00 && msg_info.update)
        {
            msg_info.update = 0x00;
	    for (ch = 0x01; ch <= MAXCARDS; ch ++)
            {
	        traffic_info.prt(ch);
                if (!traffic_info.ept(ch))
                {
                    u8 num, action;
                    ppath_t rut = NULL;
                    
                    num    = traffic_info.get(ch);
                    rut    = msg_info.find(num);
                    action = msg_info.action(rut, ch);
                    if (rut == NULL || action == 0x00)   continue;
                    if (action != CAR_STOP && rut->status != CAR_START && 
                        rep_info.key[num - 0x01].vehicle.status == CAR_STOP)
                    {
                        sea_sendsinglecommand(ICHP_SV_OPEN, num);
                    }
                }
            }
        }
#endif

        vTaskDelay(1 / portTICK_RATE_MS);      // 1ms delay
    }
}
