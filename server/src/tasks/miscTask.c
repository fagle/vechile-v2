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
    frame_t     frm;
    u8          ch;
    
    while (1)
    {
        ticker ++;

        if (consfrm1.get(&consfrm1, &frm) != NULL)
            ServerFrameCmdHandler(frm);

        if (w108frm1.get(&w108frm1, &frm) != NULL)  
             CoordFrameCmdHandler(frm);
       
#ifdef LWIP_ENABLE     
        if (tcpfrm1.get(&tcpfrm1, &frm) != NULL)
            ServerFrameCmdHandler(frm);
#endif

#ifdef TRAFFIC_ENABLE
        if ((ticker % 0x800) == 0x00 && msg_info.update)
        {
            msg_info.update = 0x00;
	    for (u8 ch = 0x01; ch <= MAXCARDS; ch ++)
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
        sea_canhandler(ticker);

#ifdef LWIP_ENABLE
        if ((ticker % 0x200) == 0x00)           // ~1 second
        {
            report_t  report1;
            sea_memset(&report1, 0x00, sizeof(report_t));
            report1.carid    = 205;
            report1.cardid   = 0x05;
            report1.type     = 0x20;
            report1.status   = 0x53;
            report1.obstacle = 0x00;
            report1.step     = 0x01;
            report1.count    = 0x05;
            report1.fail     = 0x00;
            report1.speed    = 0x50;
            report1.magic    = 0x03f0;
            tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
        }
#endif
        
        if ((ticker % 0x200) == 0x00)           // ~0.5 second
        {
//	    for (ch = 0x01; ch <= sys_info.ctrl.car && client.send.len == 0x00; ch ++)
	    for (ch = 0x01; ch <= sys_info.ctrl.car; ch ++)
            {
                ppath_t rut = msg_info.find(sys_info.ctrl.base + ch);
                if (rut->update)
                {
                    report_t  report1;
                    plamp_t ptr = (plamp_t)&rep_info.key[sys_info.ctrl.base + ch - 0x01];
                    if (dyn_info.report)
                    {
                        report1.carid    = ptr->vehicle.number;
                        report1.cardid   = ptr->vehicle.card;
                        report1.type     = ptr->vehicle.type;
                        report1.status   = ptr->vehicle.status;
                        report1.obstacle = ptr->vehicle.obstacle;
                        report1.step     = ptr->vehicle.index;
                        report1.count    = ptr->vehicle.count;
                        report1.fail     = ptr->vehicle.fail;
                        report1.speed    = ptr->vehicle.speed;
                        report1.magic    = ptr->vehicle.magic;
                        consfrm1.put(&consfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
#ifdef LWIP_ENABLE
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
#endif
                    }
                    else
                    {
                        sea_printreport(ptr);
#ifdef LWIP_ENABLE
#if 0
                        tcpfrm1.print(&tcpfrm1, "\n%4d %4d %4x %4x %4x %4d %4x %4d %4d %4d, %2d:%2d", 
                                                            ptr->vehicle.type,
                                                            ptr->vehicle.number,
                                                            ptr->vehicle.status,
                                                            ptr->vehicle.fail,
                                                            ptr->vehicle.obstacle,
                                                            ptr->vehicle.card,
                                                            ptr->vehicle.magic,
                                                            ptr->vehicle.index,
                                                            ptr->vehicle.count,
                                                            ptr->vehicle.speed,
                                                            sea_getsystime()->min, sea_getsystime()->sec);
#else
                        report1.carid    = ptr->vehicle.number;
                        report1.cardid   = ptr->vehicle.card;
                        report1.type     = ptr->vehicle.type;
                        report1.status   = ptr->vehicle.status;
                        report1.obstacle = ptr->vehicle.obstacle;
                        report1.step     = ptr->vehicle.index;
                        report1.count    = ptr->vehicle.count;
                        report1.fail     = ptr->vehicle.fail;
                        report1.speed    = ptr->vehicle.speed;
                        report1.magic    = ptr->vehicle.magic;
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTCAR, sizeof(report_t), sys_info.ctrl.road, (u8 *)&report1);
#endif
#endif
                    }
                    rut->update = 0x00;
                    break;
                }
            }
            
//            for (ch = 0x00; ch < sys_info.ctrl.call && client.send.len == 0x00; ch ++)
            for (ch = 0x00; ch < sys_info.ctrl.call; ch ++)
            {
                pcall_t cal = (pcall_t)rep_info.goal[ch];
                if (cal->update)
                {
                    if (dyn_info.report)
                    {
                        dyn_info.buffer[0x00] = cal->num;
                        dyn_info.buffer[0x01] = cal->type;
                        consfrm1.put(&consfrm1, ICHP_PC_RPTBEEP, 0x02, sys_info.ctrl.road, dyn_info.buffer);
#ifdef LWIP_ENABLE
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTBEEP, 0x02, sys_info.ctrl.road, dyn_info.buffer);
#endif
                    }
                    else
                    {
                        sea_printf("\ncall vehicle type %d from station %02x", cal->type, cal->num);
#ifdef LWIP_ENABLE
#if 0
                        tcpfrm1.print(&tcpfrm1, "\ncall vehicle type %d from station %02x", cal->type, cal->num);
#else
                        dyn_info.buffer[0x00] = cal->num;
                        dyn_info.buffer[0x01] = cal->type;
                        tcpfrm1.put(&tcpfrm1, ICHP_PC_RPTBEEP, 0x02, sys_info.ctrl.road, dyn_info.buffer);
#endif
#endif
                    }
                    cal->update = 0x00;
                    break;
                }
            }
        }
        
#ifdef LWIP_ENABLE
        if (client.count >= 0x200 && client.conn)
            client.conn = 0x00;
#endif
        
        vTaskDelay(1 / portTICK_RATE_MS);      // 1ms delay
    }
}
