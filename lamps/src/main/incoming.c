#include "network.h"
#include "sensor.h"
#include "sink.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
#ifdef SINK_APP
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : emberCoordinatorMessageHandler
//* 功能        : callback
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ezspCoordinatorIncomingMessageHandler ( EmberIncomingMessageType type,
                                             EmberApsFrame *apsFrame,
                                             EmberMessageBuffer message )
{
    EmberNodeId sender = emberGetSender();
    u8   length = emberMessageBufferLength(message);
    u8 * ptr    = emberGetLinkedBuffersPointer(message, 0x00);

//    DBG("\r\n[REC]msg id%2x seq %d", apsFrame->clusterId, apsFrame->profileId);
#ifdef ENABLE_GATEWAY
    if (sender)
    {
        sea_gwmsendframe(apsFrame->clusterId, length, ptr);
        Debug("\r\n[gw] send frame to spi 0x%2x", apsFrame->clusterId);
    }
#endif // ENABLE_GATEWAY
    if (!sender)
        return;
    
    switch (apsFrame->clusterId) 
    {
        case ICHP_SV_CTRL:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_CTRL, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_SECKEY:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_SECKEY, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_CHANNEL:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_CHANNEL, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_ADDRESS:
            if (length == sizeof(device_t))
            {
                device_t dev;
                sea_memcpy(&dev, ptr, sizeof(device_t));
                road_info.update(dev.num - 0x01, sender);
                if (sys_info.ctrl.release)
                    uartfrm.put(&uartfrm, ICHP_SV_ADDRESS, sender, sizeof(device_t), (u8 *)&dev);
                else
                    DBG("\r\n[coord]lamp address body length = %d", length);
            }
            break;
        case ICHP_SV_RESPONSE:     
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_RESPONSE, sender, length, ptr);
            else
                DBG("\r\n[coord]%02x cmd response of %dth vehicle status %d", ptr[0x01], ptr[0x00], ptr[0x02]);
            break;
        case ICHP_SV_RPT:
            sea_reporthandler((plamp_t)ptr, sender);
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_RPT, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle report", road_info.number(sender));
            break;
        case ICHP_SV_JOIN:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_JOIN, sender, 10, &ptr[0x01]);
            else
                DBG("\r\n[coord]new device join, logic address %04x", sender);
            sea_sendmsg(&send1, UNICAST, sender, ICHP_SV_ADDRESS, 0x00, NULL);
            break;
#if 0
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  not use   2014/05/31         
        case ICHP_SV_END:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_END, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_CLOSE:     
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_CLOSE, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_ADJUST:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_ADJUST, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_OPEN:     
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_OPEN, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_ROUTE:     
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_ROUTE, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_ASSIGN:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_ASSIGN, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle response %02x, body length %d", road_info.number(sender), apsFrame->clusterId, length);
            break;
        case ICHP_SV_DATE:     
            sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_DATE, sizeof(systime_t), &single_info.time);
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_DATE, sender, length, ptr);
            break;
        case ICHP_SV_ACKNOWLEDGE:     
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, ICHP_SV_ACKNOWLEDGE, sender, length, ptr);
            else
                DBG("\r\n[coord]%dth vehicle acknowledge return", road_info.number(sender));  
            break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
#endif
        default:
            if (sys_info.ctrl.release)
                uartfrm.put(&uartfrm, apsFrame->clusterId, sender, length, ptr);
            break;
    } //end switch 
}

#else   // end of SINK_APP
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspRouterIncomingMessageHandler
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : ++ray:重复
//*------------------------------------------------*/
void ezspRouterIncomingMessageHandler ( EmberIncomingMessageType type, EmberApsFrame *apsFrame, EmberMessageBuffer message )
{
    u8          length = emberMessageBufferLength(message);
    EmberNodeId sender = emberGetSender();
    u8    *     ptr = emberGetLinkedBuffersPointer(message, 0x00);
    static  u8  table[0x05];

//    DBG("\r\n[REC]msg id%2x len %d", apsFrame->clusterId, length);
    table[0x00] = sys_info.dev.num;
    table[0x01] = apsFrame->clusterId;
    table[0x02] = 0xff;
    
    switch (apsFrame->clusterId) 
    {
        case ICHP_SV_ASSIGN:
            if (ptr[0x00] == sys_info.dev.num || ptr[0x03] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_ASSIGN %d, beeper status %d, %d", ptr[0x00], ptr[0x02], ptr[0x03]);
                if (isCallDevice())
                {
                    carInfo.beep->assign = 0x00;
                    if (carInfo.beep->status != ptr[0x02])
                        carInfo.beep->status = ptr[0x02];
                    if (carInfo.beep->call)
                        carInfo.beep->call = 0x00; 
                    if (ptr[0x02] == ASSIGNCAR)
                        carInfo.beep->car = ptr[0x03];
                    else
                        carInfo.beep->car = 0x00;
                    table[0x02] = carInfo.beep->status;
                }
                else if (isCarDevice())             // ptr[0x00]--caller, ptr[0x01]--type, ptr[0x02]--status, ptr[0x03]--vehicle's number
                {
                    if (ptr[0x02] == ASSIGNCAR)
                        carInfo.key = ptr[0x00];    // caller's station number
                    else
                        carInfo.key = 0x00;
                    table[0x02] = 0x00;
                }
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_ROUTE:      // carID exist, can check error
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_ROUTE %d, index %d", ptr[0x00], ptr[0x01]);
                table[0x01] = ptr[0x01];                           // index
                table[0x02] = (length - 0x02) / sizeof(action_t);  // route sum
                table[0x02] = vehicleRouteTable(table[0x01], table[0x02], (paction_t)&ptr[0x02]);  
                table[0x01] = apsFrame->clusterId;
                table[0x03] = ptr[0x01];
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x04, table);    // special
            break;
        case ICHP_SV_OPEN:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_OPEN %d", ptr[0x00]);
                single_info.on(&ptr[0x00]);
                table[0x02] = 0x00;
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_CLOSE:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_CLOSE %d", ptr[0x00]);
                single_info.off(&ptr[0x00]);
                table[0x02] = 0x00;
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_ADJUST:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_ADJUST to %d", ptr[0x01]);
                sea_adjustchannel(ptr[0x01]);
                table[0x02] = 0x00;
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_END:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_END %d", ptr[0x00]);
                carInfo.off();
                carClearRouteTable();
                table[0x02] = 0x00;
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_CTRL:
            if (length == sizeof(ctrl_t))
            {
                Debug("\r\n[route]receive ICHP_SV_CTRL change ctrl_t");
                if (sea_memcomp(&sys_info.ctrl, ptr, sizeof(ctrl_t)) != 0x00)
                {
                    sea_memcpy(&sys_info.ctrl, ptr, sizeof(ctrl_t));
                    sea_updatesysconfig();
                }
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_CTRL, sizeof(ctrl_t), &sys_info.ctrl); 
            break;
        case ICHP_SV_SECKEY:
            if (length == sizeof(key_t))
            {
                DBG("\r\n[route]receive ICHP_SV_SECKEY change key_t");
                if (sea_memcomp(&sys_info.key, ptr, sizeof(key_t)) != 0x00)
                {
                    sea_memcpy(&sys_info.key, ptr, sizeof(key_t));
                    sea_updatesysconfig();
                }
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_SECKEY, sizeof(key_t), &sys_info.key); 
            break;
        case ICHP_SV_CHANNEL:
            if (length == sizeof(channel_t))
            {
                DBG("\r\n[route]receive ICHP_SV_CHANNEL change channel_t");
                if (sea_memcomp(&sys_info.channel, ptr, sizeof(channel_t)) != 0x00)
                {
                    sea_memcpy(&sys_info.channel, ptr, sizeof(channel_t));
                    if (sys_info.channel.index < MAXCHANNEL)
                    {
                        sys_info.channel.mask  = DEFALUTMASK;    // 0x01 << sys_info.channel.index;
                        sea_updatesysconfig();
                    }
                }
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_CHANNEL, sizeof(channel_t), &sys_info.channel); 
            break;
        case ICHP_SV_ADDRESS:
        {
            pdevice_t tmp = (pdevice_t)ptr;
            if (tmp->num > 0x00 && tmp->num < sys_info.ctrl.maxdev && length == sizeof(device_t))
            {
                DBG("\r\n[route]receive ICHP_SV_ADDRESS change device_t");
                sys_info.dev.num  = tmp->num;
                sys_info.dev.type = tmp->type;
                sea_updatesysconfig();
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_ADDRESS, sizeof(device_t), &sys_info.dev); 
            break;
        }
        case ICHP_SV_DEFAULT:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_DEFAULT save the system default");
                sea_defaultconfig();
                table[0x02] = 0x00;
            }
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_REBOOT:
            if (ptr[0x00] == sys_info.dev.num)
            {
                DBG("\r\n[route]receive ICHP_SV_REBOOT, system will reboot just now");
                table[0x02] = 0x00;
                sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
                halReboot();
            }
            else
                sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_ACKNOWLEDGE: 
            sea_memcpy(carInfo.state, ptr, length);
            table[0x02] = 0x00;
            for (u8 i = 0x00; i < length; i ++)
                table[0x02] += carInfo.state[i];  
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_RPT:
            if (isCarDevice())
                set_lampmode(LAMP_CHANGE);
            else if (isCallDevice())
            {
                if (carInfo.beep->assign)
                    set_lampmode(LAMP_FORCE);
            }
            break;
        case ICHP_SV_DATE:
            sea_updatetime((psystime_t)ptr);  
            if (single_info.fail(GETFAILBIT, LAMP_ER_TM))
                single_info.fail(CLRFAILBIT, LAMP_ER_TM);
            table[0x02] = 0x00;
            sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_RESPONSE, 0x03, table); 
            break;
        case ICHP_SV_JOIN:
            break;
        default: 
            if (apsFrame->clusterId != ICHP_SV_JOIN)
            {
                DBG("\r\n[route]unknown cluster:0x%x from 0x%x;", apsFrame->clusterId, sender);
                table[0x02] = 0xfe;
                sea_sendmsg(&send1, UNICAST, COORDID, apsFrame->clusterId, 0x03, table); 
            }
            break;
    } 
    
#ifdef ENABLE_GATEWAY
    Debug("\r\n[gw] send frame to spi 0x%2x", apsFrame->clusterId);
    //Debug("\r\n[sensor] send  ", apsFrame->clusterId);
    sea_gwmsendframe(apsFrame->clusterId, length, ptr);
#endif // ENABLE_GATEWAY
}
#endif  // end of SENSOR_APP

//////////////////////////////////////////////////////////////////////////////////////////////////////


