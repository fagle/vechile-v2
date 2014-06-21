#include "network.h"
#include "lamp.h"

#define VWAITING  (0x01)
#define VONLINE   (0x02)
#define VARRIVAL  (0x03)

/////////////////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t consfrm1, w108frm1, tcpfrm1;     // for console protocol
extern rep_info_t   rep_info;

/////////////////////////////////////////////////////////////////////////////////////
//
traffic_info_t traffic_info;

#if 0
/*******************************************************************************
* Function Name  : u8 sea_findcard ( ppath_t ptr, u8 card )
* Description    : find card in route table
* Input          : ppath_t ptr, u8 card
* Output         : true/false
* Return         : None
*******************************************************************************/
u8 sea_findcard ( ppath_t ptr, u8 card )
{
    for (u8 i = 0x00; i < sys_info.card.cnt; i ++)
    {
        if (card == sys_info.card.list[i])
          return 0x01;
    }
    return 0x00;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//* 函数名      : static u8  sea_sendroutetable ( ppath_t ptr )
//* 功能        : parse route table to w108
//* 输入参数    : ppath_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u8  sea_parseroutetable ( ppath_t ptr )
{   
    u8   i, idx;
    u8   card[0x06];
    
    if (ptr == NULL || ptr->cnt == 0x00)
        return 0x00;
    
    for (idx = i = 0x00; i < ptr->cnt; i ++)
    {
        if (ptr->line[i].action == CAR_STOP)  
            card[idx ++] = ptr->line[i].id;
        if (!ptr->line[i].id || !ptr->line[i].action)
            return 0x00;
        if (idx >= 0x06)   
            return 0x00;
    }
    if (!idx)    return 0x00;       
    if (ptr->line[ptr->cnt - 0x01].action != CAR_STOP)
        return 0x00;
    
    sea_printf("\ntype %d, %dth vehicle's garage %d", sea_getcartype(ptr->num), ptr->num, card[idx - 0x01]);
    if (msg_info.garage[sea_getcartype(ptr->num) - CARIDST] != card[idx - 0x01])
        msg_info.garage[sea_getcartype(ptr->num) - CARIDST] = card[idx - 0x01];
       
#ifdef CARD_ENABLE
    card[0x00] = sys_info.card.cnt;
    for (i = 0x00; i < ptr->cnt; i ++)
    {
        for (idx = 0x00; idx < sys_info.card.cnt; idx ++)
        {
            if (ptr->line[i].id == sys_info.card.list[idx])
                break;
        }
        if (idx == sys_info.card.cnt)
        {
            sys_info.card.list[sys_info.card.cnt ++] = ptr->line[i].id;
            if (sys_info.card.cnt >= MAXCARDS)
                break;
        }
    }
    if (card[0x00] != sys_info.card.cnt)
    {
        sea_printf("\nupdate card's list.");
        sea_updatesysinfo();
    }
#endif
    return 0x01;
}

/************************************************
* 函数名      : void sea_sendacknowledge ( u8 cmd, u16 id, u8 num, u8 state )
* 功能        : frame command handler from server
* 输入参数    : u8 cmd, u16 id, u8 num, u8 state
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void sea_sendacknowledge ( u8 cmd, u16 id, u8 num, u8 state )
{
    u8 buf[0x05] = {0x00, 0x00,};
    
    if (dyn_info.report)
    {
        buf[0x00] = num;
        buf[0x01] = cmd;
        buf[0x02] = state;
        consfrm1.put(&consfrm1, ICHP_PC_RESPONSE, 0x03, id, buf);
#ifdef LWIP_ENABLE
        tcpfrm1.put(&tcpfrm1, ICHP_PC_RESPONSE, 0x03, id, buf);
#endif
    }
    else
    {
        sea_printf("\n%dth device's %02x command status %d", num, cmd, state);
#ifdef LWIP_ENABLE
#if 0
        tcpfrm1.print(&tcpfrm1, "\n%dth device's %02x command status %d", num, cmd, state);
#else
        buf[0x00] = num;
        buf[0x01] = cmd;
        buf[0x02] = state;
        tcpfrm1.put(&tcpfrm1, ICHP_PC_RESPONSE, 0x03, id, buf);
#endif
#endif
    }
}

#if 0
/************************************************
* 函数名      : void ServerFrameCmdHandler ( frame_t fr )
* 功能        : frame command handler from server
* 输入参数    : frame_t fr
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void ServerFrameCmdHandler ( frame_t fr )
{
    ppath_t rut = NULL;
    
    if (isCarDevice(CARIDST, fr.body[0x00]))
        rut = msg_info.find(fr.body[0x00]);
                
    switch (fr.cmd)
    {
        case ICHP_PC_ROUTE: 
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic && (fr.len & 0x01) == 0x00)
            {
                if (rep_info.key[fr.body[0x00] - 0x01].vehicle.status == CAR_STOP)
                {
                    if (msg_info.add(fr.body[0x00], fr.body[0x01], fr.body[0x03] * sizeof(action_t), &fr.body[0x04]))
                    {
                        rut = msg_info.find(fr.body[0x00]);
                        if (rut->cnt == fr.body[0x02])
                        {
                            if (sea_isvalidateroute(rut))
                            {
                                if (sea_parseroutetable(rut)) 
                                    sea_sendroutetable(rut); 
                                else 
                                    sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
                            }
                            else
                                sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
                        }
                    }
                    else
                        sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
               }
               else
                   sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
            }
            else
                sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
            break;
        case ICHP_SV_END:   
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic) 
            {
                dyn_info.buffer[0x00] = fr.body[0x00];
                w108frm1.put(&w108frm1, ICHP_SV_END, 0x01, dyn_info.addr[fr.body[0x00] - 0x01].logic, dyn_info.buffer);
            }
            break;
        case ICHP_SV_OPEN:                       
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic) 
            {
                dyn_info.buffer[0x00] = fr.body[0x00];
                w108frm1.put(&w108frm1, ICHP_SV_OPEN, 0x01, 0x00, dyn_info.buffer);
                rut->status = CAR_START;
                break;
            }
        case ICHP_SV_CLOSE:                   
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic) 
            {
                dyn_info.buffer[0x00] = fr.body[0x00];
                w108frm1.put(&w108frm1, ICHP_SV_CLOSE, 0x01, dyn_info.addr[fr.body[0x00] - 0x01].logic, dyn_info.buffer);
                rut->status = CAR_PAUSE;
            }
            break;
        case ICHP_SV_DATE:
            if (fr.len >= 0x06)
            {
                sea_getsystime()->year = 2000 + fr.body[0x00]; 
                sea_getsystime()->mon  = fr.body[0x01]; 
                sea_getsystime()->day  = fr.body[0x02]; 
                sea_getsystime()->hour = fr.body[0x03]; 
                sea_getsystime()->min  = fr.body[0x04]; 
                sea_getsystime()->sec  = fr.body[0x05]; 
                w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
            }
            break;
        case ICHP_SV_REBOOT:
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic) 
            {
                dyn_info.buffer[0x00] = fr.body[0x00];
                w108frm1.put(&w108frm1, ICHP_SV_REBOOT, 0x01, dyn_info.addr[fr.body[0x00] - 0x01].logic, dyn_info.buffer);
            }
            break;
        case ICHP_SV_ADJUST:
            if (dyn_info.addr[fr.body[0x00] - 0x01].logic) 
            {
                dyn_info.buffer[0x00] = fr.body[0x00];
                dyn_info.buffer[0x01] = fr.body[0x01];
                sea_printf("\n%dth device radio channel adjust to %d", fr.body[0x00], fr.body[0x01]);
                w108frm1.put(&w108frm1, ICHP_SV_ADJUST, 0x02, dyn_info.addr[fr.body[0x00] - 0x01].logic, dyn_info.buffer);
            }
            break;
        case ICHP_PC_ASSIGN:
            sea_printf("\nsend ICHP_PC_ASSIGN to caller %d call status %d, len %d", fr.body[0x00], fr.body[0x02], fr.len);
            if (isCallDevice(CALLIDST, fr.body[0x00]))
            {
                pcall_t cal = (pcall_t)rep_info.goal[fr.body[0x00] - 0x01];
                if (cal->state != fr.body[0x02])
                {
                    cal->type    = fr.body[0x01];
                    cal->ack     = 0x01;
                    cal->state   = fr.body[0x02];
                    cal->vehicle = fr.body[0x03];
                }
            }
            break;
        case 0x01: //write table
        {
              //fr.body[0x00] car id
              //fr.body[0x01] index
              //fr.body[0x02] total sum
              //fr.body[0x03] current sum
              u8 carid = fr.body[0x00];
              u8 index = fr.body[0x01];
              u8 csum = fr.body[0x03];
              
              if ((fr.len & 0x01) == 0x00)
              {
                  if (rep_info.key[carid - 0x01].vehicle.status == CAR_STOP)
                  {
                      msg_info.add(carid, index, csum * sizeof(action_t), &fr.body[0x04]);
                  }
                  else
                  {
                      consfrm1.print(&consfrm1,"status is not CAR_STOP");
                  }
              }
              break;
        }
        case 0x02: //show table
          {                        
              u8 carid = fr.body[0x00];
              ppath_t ptr = msg_info.find(carid);
              if(ptr == NULL)
              {
                  consfrm1.print(&consfrm1,"\ncan not find table");
              }
              else
              {
                  consfrm1.print(&consfrm1,"\nroute table of %d,cnt %d", carid,ptr->cnt);
                  consfrm1.print(&consfrm1,"\n");
                  for(u8 i = 0;i<ptr->cnt;i++)
                  {
                      consfrm1.print(&consfrm1,"%2x ",ptr->line[i].id);
                  }
                  consfrm1.print(&consfrm1,"\n");
                  for(u8 i = 0;i<ptr->cnt;i++)
                  {
                      consfrm1.print(&consfrm1,"%2x ",ptr->line[i].action);
                  }
              }
              break;
          }
        case 0x03: //send table
          {              
              u8 carid = fr.body[0x00];  //carid
              
              if (dyn_info.addr[carid - 0x01].logic)
              {
                  consfrm1.print(&consfrm1,"\nICHP_SV_ROUTE car id %d",carid);
                  ppath_t rut = msg_info.find(carid);
                  sea_sendroutetable(rut);
              }
              else
              {
                  consfrm1.print(&consfrm1,"\ncan not find car id",carid);
              }
              break;
          }
        case 0x05: //open
          {
              consfrm1.print(&consfrm1,"\nsending ICHP_SV_OPEN%d",fr.body[0x00]);
              w108frm1.put(&w108frm1, ICHP_SV_OPEN, 0x01, 0x00, fr.body);
              break;
          }
        case 0x06: //close
          {
              consfrm1.print(&consfrm1,"\nsending ICHP_SV_CLOSE%d",fr.body[0x00]);
              w108frm1.put(&w108frm1, ICHP_SV_CLOSE, 0x01, 0x00, fr.body);
              break;
          }
        case 0x07: //clear table
          {   
              u8 carid = fr.body[0x00];
              if (dyn_info.addr[carid - 0x01].logic) 
              {
                  consfrm1.print(&consfrm1,"\nsend end_line vehicle %d, logic address %d",carid, dyn_info.addr[carid - 0x01].logic);
                  w108frm1.put(&w108frm1, ICHP_SV_END, 0x01, 0x00, fr.body);
              }
              else
                  consfrm1.print(&consfrm1,"\ncan not find carid%d",fr.body[0x00]);
              break;
          }
        default:
            break;
    }
}
#else
/************************************************
* 函数名      : void ServerFrameCmdHandler ( frame_t fr )
* 功能        : frame command handler from server
* 输入参数    : frame_t fr
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void ServerFrameCmdHandler ( frame_t fr )
{
    pmsg_t  msg = (pmsg_t)&rep_info.send[fr.body[0x00] - 0x01];

    if (dyn_info.addr[fr.body[0x00] - 0x01].logic == 0x00 || msg->id != 0x00) 
    {
        sea_printf("\n%dth device is power off or send message is busy.", fr.body[0x00]);
        sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);   // command can't return from devices yet.
        return;
    }
    switch (fr.cmd)
    {
        case ICHP_PC_ROUTE: 
            if (rep_info.key[fr.body[0x00] - 0x01].vehicle.status == CAR_STOP && 
                (fr.len & 0x01) == 0x00 && isCarDevice(CARIDST, fr.body[0x00]))
            {
                if (msg_info.add(fr.body[0x00], fr.body[0x01], fr.body[0x03] * sizeof(action_t), &fr.body[0x04]))
                {
                    ppath_t rut = msg_info.find(fr.body[0x00]);
                    if (sea_parseroutetable(rut)) 
                    {
                        sea_printf("\nsend ICHP_PC_ROUTE to %dth vehicle index %d, size %d", fr.body[0x00], fr.body[0x01], fr.body[0x02]);
                        msg->id  = fr.cmd;
//                        sea_sendroutetable(rut); 
                    }
                    else 
                        sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
                }
                else
                    sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
           }
           else
               sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPERR);
            break;
        case ICHP_PC_ASSIGN:
            if (isCallDevice(CALLIDST, fr.body[0x00]))
            {
                pcall_t cal = (pcall_t)rep_info.goal[fr.body[0x00] - 0x01];
                if (cal->state != fr.body[0x02])
                {
                    sea_printf("\nsend ICHP_PC_ASSIGN to caller %d call status %d", fr.body[0x00], fr.body[0x02]);
                    msg->id      = fr.cmd;
                    cal->type    = fr.body[0x01];
                    cal->state   = fr.body[0x02];
                    cal->vehicle = fr.body[0x03];
                    if (isCarDevice(CARIDST, fr.body[0x03]))
                    {
                        rep_info.send[fr.body[0x03] - 0x01].id        = fr.cmd;
                        rep_info.send[fr.body[0x03] - 0x01].buf[0x00] = fr.body[0x00];
                    }
                }
            }
            break;
        case ICHP_SV_DATE:
            sea_getsystime()->year = 2000 + fr.body[0x00]; 
            sea_getsystime()->mon  = fr.body[0x01]; 
            sea_getsystime()->day  = fr.body[0x02]; 
            sea_getsystime()->hour = fr.body[0x03]; 
            sea_getsystime()->min  = fr.body[0x04]; 
            sea_getsystime()->sec  = fr.body[0x05]; 
            msg->id   = fr.cmd;
            break;
        case ICHP_SV_OPEN:                       
        case ICHP_SV_END:   
        case ICHP_SV_REBOOT:
        case ICHP_SV_CLOSE:                   
        case ICHP_SV_ADJUST:
            sea_printf("\nsend %02x command to %dth device", fr.cmd, fr.body[0x00]);
            msg->id        = fr.cmd;
            msg->buf[0x00] = fr.body[0x00];
            if (fr.cmd == ICHP_SV_ADJUST)
                msg->buf[0x01] = fr.body[0x01];
            break;
        default:
            sea_printf("\npc send unknow command %02x.", fr.cmd);
            break;
    }
}

/************************************************
* 函数名      : void TaskCmdHandler ( void )
* 功能        : taskcommand handler from server
* 输入参数    : 无
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void TaskCmdHandler ( void )
{
    for (u8 i = 0x00; i < MAXLAMPS; i ++)
    {
        if (dyn_info.addr[i].logic)
        {
            pmsg_t  msg = (pmsg_t)&rep_info.send[i];
            if (!msg->id)   continue;
            switch (msg->id)
            {
                case ICHP_PC_ROUTE: 
                    sea_sendroutetable(msg_info.find(i + 0x01)); 
                    msg->cnt ++;
                    break; 
                case ICHP_PC_ASSIGN:
                {
                    pcall_t cal = NULL;
                    u8  body[0x05];
                    
                    if (i > sys_info.ctrl.base)
                        cal = (pcall_t)rep_info.goal[msg->buf[0x00]];
                    else
                        cal = (pcall_t)rep_info.goal[i];
                    body[0x00] = cal->num;
                    body[0x01] = cal->type;
                    body[0x02] = cal->state;
                    body[0x03] = cal->vehicle;
                    if (isCallDevice(CALLIDST, i + 0x01))
                    {
                        if (cal->logic[0x00])
                            w108frm1.put(&w108frm1, ICHP_SV_ASSIGN, 0x04, cal->logic[0x00], body);
                        if (cal->logic[0x01])
                            w108frm1.put(&w108frm1, ICHP_SV_ASSIGN, 0x04, cal->logic[0x01], body);
                    }
                    else if (isCarDevice(CARIDST, i + 0x01))
                        w108frm1.put(&w108frm1, ICHP_SV_ASSIGN, 0x04, dyn_info.addr[i].logic, body);
                    msg->cnt ++;
                    break; 
                }
                case ICHP_SV_ADJUST:
                    w108frm1.put(&w108frm1, msg->id, 0x02, dyn_info.addr[i].logic, msg->buf);
                    msg->cnt ++;
                    break; 
                case ICHP_SV_DATE:
                    w108frm1.put(&w108frm1, msg->id, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
                    msg->cnt ++;
                    break; 
                default:
                    w108frm1.put(&w108frm1, msg->id, 0x01, dyn_info.addr[i].logic, msg->buf);
                    msg->cnt ++;
                    break;
            }
            
            if (msg->cnt > 0x01)    // retry too much.
            {
                msg->id   = 0x00;
                msg->cnt  = 0x00;
            }
            return;
        }
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_parsereport ( plamp_t ptr, u16 road )
//* 功能        : print lamp_t information
//* 输入参数    : plamp_t ptr, u16 road
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_parsereport ( plamp_t ptr, u16 road )
{
    if (!ptr || sys_info.ctrl.app != eVehicle)   return;
    
    if (ptr->vehicle.number > 0x00 && ptr->vehicle.number <= sys_info.ctrl.maxdev)
    {
        if (ptr->vehicle.type != CENTERIDST)   
        {
            time_t  tm;
            if (dyn_info.addr[ptr->vehicle.number - 0x01].logic != road)
                dyn_info.addr[ptr->vehicle.number - 0x01].logic = road;

            if (dyn_info.addr[ptr->vehicle.number - 0x01].dev.type != ptr->vehicle.type)
                dyn_info.addr[ptr->vehicle.number - 0x01].dev.type = ptr->vehicle.type;
              
            if (isCarDevice(ptr->vehicle.type, ptr->vehicle.number))   
            {
                msg_info.find(ptr->vehicle.number)->update = 0x01;
            }
            else if (isCallDevice(ptr->vehicle.type, ptr->vehicle.number)) 
            {
                pbeep_t bep = (pbeep_t)&ptr->vehicle;
                if (bep->call)
                {
                    pcall_t cal = (pcall_t)rep_info.goal[bep->number - 0x01];
                    if (cal->logic[beepidx(bep->call)] != road)
                        cal->logic[beepidx(bep->call)] = road;
                    cal->num    = bep->number;
                    cal->type   = bep->call;
                    cal->update = 0x01;
                }
            } 
            else
            {
                sea_printf("\nnumber %d, type %d are not allow devices in the application", ptr->vehicle.number, ptr->vehicle.type);
                ptr->vehicle.fail |= LAMP_ER_NP;
                if (ptr->vehicle.number >= sys_info.ctrl.maxdev || !ptr->vehicle.number)
                    ptr->vehicle.number = sys_info.ctrl.base;
            }
            sea_memcpy(&rep_info.key[ptr->vehicle.number - 0x01], ptr, sizeof(lamp_t));
            sea_memcpy(&rep_info.time[ptr->vehicle.number - 0x01], sea_getcurtime(&tm), sizeof(time_t));
        }
    }
      
    if (ptr->vehicle.fail & LAMP_ER_TM)
        w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
}

#if 0
/************************************************
* 函数名      : void CoordFrameCmdHandler ( frame_t fr )
* 功能        : frame command handler from coordinator
* 输入参数    : frame_t fr
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void CoordFrameCmdHandler ( frame_t fr )
{
    ppath_t rut = NULL;
    u8    num = sea_lookuplogic(fr.road);  
    
    if (isCarDevice(CARIDST, num))
        rut = msg_info.find(num);
    
    switch (fr.cmd)
    {
        case ICHP_SV_RPT:
            sea_parsereport((plamp_t)fr.body, fr.road);
            break;
        case ICHP_SV_DATE:
            w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
            break;
        case ICHP_SV_CTRL:     
            sea_memcpy(&dyn_info.w108.ctrl, fr.body, sizeof(ctrl_t));
            if (dyn_info.ready & CTRLBIT)
                dyn_info.ready &= ~CTRLBIT;
            break;
        case ICHP_SV_ADDRESS:                        // device_t body 
        {
            device_t dev;
            sea_memcpy(&dev, fr.body, sizeof(device_t));
            if (dev.type == CENTERIDST)
            {
                if (sea_memcomp(&sys_info.dev, &dev, sizeof(device_t)) != 0x00)
                {
                    sea_printf("\nupdate physic address of control center.");
                    sea_memcpy(&sys_info.dev, &dev.eui, sizeof(device_t));
                    sea_updatesysinfo();
                }
                if (dyn_info.ready & ADDRBIT)
                    dyn_info.ready &= ~ADDRBIT;
            }
            else if (isEUI64(&dev.eui))  
            {
                if (dev.num > 0x00 && dev.num <= sys_info.ctrl.maxdev)
                {
                    if (sea_memcomp(&dyn_info.addr[dev.num - 0x01].dev, &dev, sizeof(device_t)) != 0x00)
                    {
                        sea_printf("\n[sv_address]new device join.");
                        sea_memcpy(&dyn_info.addr[dev.num - 0x01].dev, &dev, sizeof(device_t));
                        dyn_info.addr[dev.num - 0x01].logic = fr.road;
                    }
                }
            }
            else
                sea_printf("\nchange num completed");
            break;
        }
        case ICHP_SV_CHANNEL:                        // channel body 
            sea_memcpy(&dyn_info.w108.channel, fr.body, sizeof(channel_t));
            if (dyn_info.ready & CHNBIT)
                dyn_info.ready &= ~CHNBIT;
            break;
        case ICHP_SV_SECKEY:                         // key_t body 
            sea_memcpy(&dyn_info.w108.key, fr.body, sizeof(key_t));
            if (dyn_info.ready & KEYBIT)
                dyn_info.ready &= ~KEYBIT;
            break;
        case ICHP_SV_JOIN:                           // logic and physic address 
        {
            u16 no;
            if (no = sea_lookuphysical((EmberEUI64 *)&fr.body[0x02]))
            {
                sea_printf("\n[co_join]change logic address %d to %d", dyn_info.addr[no - 0x01].logic, *((u16 *)fr.body));
                dyn_info.addr[no - 0x01].logic = *((u16 *)fr.body);
                if (isCarDevice(CARIDST, no))
                {
                    rut = msg_info.find(no);
                    rut->reboot = 0x01;
                }
            }
            break;
        }
        case ICHP_SV_ROUTE:  
            {
                u8 sum = 0x00;
                u8 size = fr.body[0x00] + RTSENDSIZE < rut->cnt ? (RTSENDSIZE) : (rut->cnt - fr.body[0x00]);
                for (u8 i = 0x00; i < size; i ++)
                    sum += rut->line[i + fr.body[0x00]].id + rut->line[i + fr.body[0x00]].action;
                if (sum != fr.body[0x01])
                {
                    sea_printf("\n%dth vehicle route table return fail, clear it", num);
                    dyn_info.buffer[0x00] = fr.body[0x00];
                    w108frm1.put(&w108frm1, ICHP_SV_END, 0x01, fr.road, dyn_info.buffer);
                }
            }
            sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPOK);
            break;
        case ICHP_SV_END:  
            sea_printf("\n%dth vehicle ends line", num);
            if (rut)    msg_info.clear(num);
            break;
        case ICHP_SV_OPEN:  
            sea_printf("\n%dth vehicle is running ...", num);
            if (rut)
            {
                if (rut->status == CAR_START)
                    rut->status = CAR_NONE;
            }
            sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPOK);
            break;
        case ICHP_SV_CLOSE: 
            sea_printf("\n%dth vehicle is stopped", num);
            if (rut)
            {
                if (rut->status == CAR_PAUSE)
                    rut->status = CAR_NONE;
            }
            sea_sendacknowledge(fr.cmd, fr.road, fr.body[0x00], RESPOK);
            break;
        case ICHP_SV_ASSIGN:
        {
            sea_printf("\nrec ICHP_SV_ASSIGN, len%d, body%d", fr.len, fr.body[0x00]);
            if (isCallDevice(CALLIDST, fr.body[0x00]))
            {
                pcall_t cal = (pcall_t)rep_info.goal[fr.body[0x00] - 0x01];
                if (cal->cnt)
                    cal->cnt --;
                if (cal->state == fr.body[0x02] && cal->cnt == 0x00)  
                    cal->ack = 0x00;
            }
            break;
        }
        case ICHP_SV_RESPONSE: 
        {
//            sea_printf("\n%dth vehicle's response, command %02x, result %d, len %d", fr.body[0x00], fr.body[0x01], fr.body[0x02], fr.len);
            switch (fr.body[0x01])
            {
                case ICHP_SV_ASSIGN:
                    if (isCallDevice(CALLIDST, fr.body[0x00]))
                    {
                        pcall_t cal = (pcall_t)rep_info.goal[fr.body[0x00] - 0x01];
//                        sea_printf("\n%d caller's ICHP_SV_ASSIGN", fr.body[0x00]);
                        if (cal->cnt)
                            cal->cnt --;
                        if (cal->state == fr.body[0x02] && cal->cnt == 0x00)  
                            cal->ack = 0x00;
                    }
                    else
                    {
                        for (u8 ch = 0x00; ch < sys_info.ctrl.call; ch ++)
                        {
                            pcall_t cal = (pcall_t)rep_info.goal[ch];
                            if (cal->vehicle == fr.body[0x00] && cal->ack)
                            {
                                cal->ack = 0x00;
                                break;
                            }
                        }
//                        sea_printf("\n%d vehicle's ICHP_SV_ASSIGN", fr.body[0x00]);
                    }
                    break;
                case ICHP_SV_CLOSE: 
                case ICHP_SV_OPEN:  
//                    sea_printf("\n%dth vehicle is running or stopped, status %d", num, fr.body[0x02]);
                    if (rut)
                    {
                        if (rut->status != CAR_NONE)
                            rut->status = CAR_NONE;
                    }
                    sea_sendacknowledge(fr.body[0x01], fr.road, fr.body[0x00], fr.body[0x02]);
                    break;
                case ICHP_SV_END:  
//                    sea_printf("\n%dth vehicle ends line, status %d", num, fr.body[0x02]);
                    if (rut)    msg_info.clear(num);
                    sea_sendacknowledge(fr.body[0x01], fr.road, fr.body[0x00], fr.body[0x02]);
                    break;
                case ICHP_SV_ROUTE:  
                {
                    u8 sum = 0x00;
//                    u8 size = fr.body[0x03] + RTSENDSIZE < rut->cnt ? (RTSENDSIZE) : (rut->cnt - fr.body[0x03]);
//                    sea_printf("\n%dth vehicle route table, sum %d", num, fr.body[0x02]);
//                    for (u8 i = 0x00; i < size; i ++)
//                        sum += rut->line[i + fr.body[0x03]].id + rut->line[i + fr.body[0x03]].action;
                    for (u8 i = 0x00; i < rut->cnt; i ++)
                        sum += rut->line[i].id + rut->line[i].action;
                    if (sum != fr.body[0x02])
                    {
                        sea_printf("\n%dth vehicle route table return fail, clear it", num);
                        dyn_info.buffer[0x00] = fr.body[0x00];
                        w108frm1.put(&w108frm1, ICHP_SV_END, 0x01, fr.road, dyn_info.buffer);
                    }
                    else
                        sea_sendacknowledge(fr.body[0x01], fr.road, fr.body[0x00], RESPOK);
                    break;
                }
            }
            break;
        }
        default:
        {
            sea_printf("\nreceived unknow id %02x", fr.cmd);
            break;
        }
    }
}
#else
/************************************************
* 函数名      : void CoordFrameCmdHandler ( frame_t fr )
* 功能        : frame command handler from coordinator
* 输入参数    : frame_t fr
* 输出参数    : 无
* 修改记录    : 无
* 备注        : 无
*************************************************/ 
void CoordFrameCmdHandler ( frame_t fr )
{
    u8  num = sea_lookuplogic(fr.road);  
    
    if (rep_info.send[num - 0x01].id)
        rep_info.send[num - 0x01].id = 0x00;
    
    switch (fr.cmd)
    {
        case ICHP_SV_RPT:
            sea_parsereport((plamp_t)fr.body, fr.road);
            break;
        case ICHP_SV_DATE:
            w108frm1.put(&w108frm1, ICHP_SV_DATE, sizeof(systime_t) - 0x04, sys_info.ctrl.road, (u8 *)sea_getsystime());
            break;
        case ICHP_SV_CTRL:     
            sea_memcpy(&dyn_info.w108.ctrl, fr.body, sizeof(ctrl_t));
            if (dyn_info.ready & CTRLBIT)
                dyn_info.ready &= ~CTRLBIT;
            break;
        case ICHP_SV_ADDRESS:                        // device_t body 
        {
            device_t dev;
            sea_memcpy(&dev, fr.body, sizeof(device_t));
            if (dev.type == CENTERIDST)
            {
                if (sea_memcomp(&sys_info.dev, &dev, sizeof(device_t)) != 0x00)
                {
                    sea_printf("\nupdate physic address of control center.");
                    sea_memcpy(&sys_info.dev, &dev.eui, sizeof(device_t));
                    sea_updatesysinfo();
                }
                if (dyn_info.ready & ADDRBIT)
                    dyn_info.ready &= ~ADDRBIT;
            }
            else if (isEUI64(&dev.eui))  
            {
                if (dev.num > 0x00 && dev.num <= sys_info.ctrl.maxdev)
                {
                    if (sea_memcomp(&dyn_info.addr[dev.num - 0x01].dev, &dev, sizeof(device_t)) != 0x00)
                    {
                        sea_printf("\n[sv_address]new device join.");
                        sea_memcpy(&dyn_info.addr[dev.num - 0x01].dev, &dev, sizeof(device_t));
                        dyn_info.addr[dev.num - 0x01].logic = fr.road;
                    }
                }
            }
            else
                sea_printf("\nchange num completed");
            break;
        }
        case ICHP_SV_CHANNEL:                        // channel body 
            sea_memcpy(&dyn_info.w108.channel, fr.body, sizeof(channel_t));
            if (dyn_info.ready & CHNBIT)
                dyn_info.ready &= ~CHNBIT;
            break;
        case ICHP_SV_SECKEY:                         // key_t body 
            sea_memcpy(&dyn_info.w108.key, fr.body, sizeof(key_t));
            if (dyn_info.ready & KEYBIT)
                dyn_info.ready &= ~KEYBIT;
            break;
        case ICHP_SV_JOIN:                           // logic and physic address 
        {
            u16 no;
            if (no = sea_lookuphysical((EmberEUI64 *)&fr.body[0x02]))
            {
                sea_printf("\n[co_join]change logic address %d to %d", dyn_info.addr[no - 0x01].logic, *((u16 *)fr.body));
                dyn_info.addr[no - 0x01].logic = *((u16 *)fr.body);
                if (isCarDevice(CARIDST, no))
                    rep_info.send[no - 0x01].id  = ICHP_PC_ROUTE;
                else if (isCarDevice(CALLIDST, no))
                    rep_info.send[no - 0x01].id  = ICHP_PC_ASSIGN;
            }
            break;
        }
        case ICHP_SV_RESPONSE: 
            switch (fr.body[0x01])
            {
                case ICHP_SV_ASSIGN:
                case ICHP_SV_CLOSE: 
                case ICHP_SV_OPEN:  
                case ICHP_SV_ROUTE:  
                case ICHP_SV_END:  
                    if (ICHP_SV_END == fr.body[0x01])   
                         msg_info.clear(num);
                    sea_sendacknowledge(fr.body[0x01], fr.road, fr.body[0x00], fr.body[0x02]);
                    break;
                default:
                    sea_printf("\n[coord]received unknow response %02x", fr.body[0x01]);
                    break;
            }
            break;
        default:
            sea_printf("\n[coord]received unknow id %02x", fr.cmd);
            break;
    }
}
#endif
//////////////////////////////////////////////////////////////////////////////////////




