#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"
#include "frame.h"
#include "network.h"
#include "cmd.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1, tcpfrm1;
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

#ifdef LWIP_ENABLE     
        if (!sea_isempty(get_serialinfo(tcpfrm1.uart))) 
        {
            ch = sea_readbyte(get_serialinfo(tcpfrm1.uart));
            tcpfrm1.recv(&tcpfrm1, ch);  
            if (tcpfrm1.state == PRT_IDLE)
                keyboardinput(ch);
        }

        if (tcpfrm1.get(&tcpfrm1, &frm) != NULL)
            ServerFrameCmdHandler(frm);
#endif
        
        if (w108frm1.get(&w108frm1, &frm) != NULL)  
             CoordFrameCmdHandler(frm);
       
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
