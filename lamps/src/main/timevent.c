#include "network.h"
#include "sensor.h"
#include "sink.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void coordinatorTimeEventHandler ( void )
//* 功能        : sink作为路由实现开/关灯功能,开完后要修改自己的数据;
//*             : sink上报FULL信息；
//*             : sink上报KEY信息；
//*             : ++comment by ray:ONESECOND调用一次
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  coordinatorTimeEventHandler ( void )
{ 
#ifdef SINK_APP    
//    u8 period = sys_info.ctrl.period < 0x05 ? 0x05 : sys_info.ctrl.period;
    static u8 timer = 0x00;
    
    timer ++;
    if (timer > 220)            //200~240
    {
        timer = 0x00;
        Debug("\r\n[EVENT]: call Permit Joins"); 
        emberPermitJoining(JOINTIMEOUT);
#if EMBER_SECURITY_LEVEL == 5
        trustCenterPermitJoins(TRUE);
#endif       
    }
    
    if (network_info.appstate != EMBER_JOINED_NETWORK)
        return;

    //**comment by ray :6minute for coordinator,5minute for router
    if ((single_info.time.min % sys_info.ctrl.period) == 0x00 && single_info.min != single_info.time.min)    
    {
        Debug("\r\n[EVENT]: 5 minutes handler, send timer to all lamps."); 
        single_info.min = single_info.time.min;
#ifdef ENABLE_GATEWAY    
        sea_gwmsendframe(ICHP_GW_CHANNEL, sizeof(channel_t), (u8 *)&sys_info.channel);
#endif
    }
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void coordinatorMsEventHandler ( void )
//* 功能        : called to check application microsecend
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void coordinatorMsEventHandler ( void )
{
    if (network_info.appstate == EMBER_JOINED_NETWORK)
    {
        if (!sea_msgisempty(&send1))   
        {
            pmsg_t ptr = sea_getmsg(&send1);
            if (ptr)
            {
                ptr->dest ? network_info.unicast(ptr->id, ptr->dest, ptr->size, ptr->body) :
                            network_info.multicast(ptr->id, ptr->size, ptr->body);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void CoordinatorTick(void)
//* 功能        : called to check application timeouts, button events,
//              : and periodically flash LEDs
//              : 对时间不敏感的循环事件
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void CoordinatorTick ( void ) 
{ 
    extern void trustCenterPermitJoins ( boolean allow );
    
    network_info.stackstate = emberNetworkState();
    if(network_info.stackstate != EMBER_JOINED_NETWORK)
        return;
    
    if (!network_info.substate)                   // 执行一次 
    {
        network_info.substate = TRUE;             // 填充single_info   
        single_info.address   = emberGetNodeId(); // get logic short address
        Debug("\r\nnetwork status is %d, node id 0x%x",emberNetworkState(), single_info.address);
        
        emberPermitJoining(JOINTIMEOUT);          // 允许加入
#if EMBER_SECURITY_LEVEL == 5
        trustCenterPermitJoins(TRUE);
#endif       
        Debug( "\r\n[EVENT]: call PermitJoin");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void  routerTimeEventHandler ( void )
//* 功能        : 路由定时开/关灯，开完后要修改自己的数据
//*             : 自动开关灯模式；
//*             : ++comment by ray:ONESECOND调用一次
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void  routerTimeEventHandler ( void )
{  
    MobileTimeEventHandler();
    
    if (network_info.appstate != EMBER_JOINED_NETWORK)
        return;
    
    if ((single_info.time.min % sys_info.ctrl.period) == 0x00 && single_info.min != single_info.time.min)   // 5 minute
    {
        single_info.min = single_info.time.min;
        if (network_info.appstate == EMBER_JOINED_NETWORK)
        {
            Debug("\r\nsend lamp information to coordinator in 5(min).");
#ifdef ENABLE_GATEWAY
            sea_gwmsendframe(ICHP_GW_CHANNEL, sizeof(channel_t), (u8 *)&sys_info.channel);
#endif
            set_lampmode(LAMP_CHANGE);
        }
    }
    
    if ((single_info.time.sec % sys_info.ctrl.period) == 0x00 && single_info.sec != single_info.time.sec)
    {
        single_info.sec = single_info.time.sec;
#ifdef ENABLE_GATEWAY
        sea_gwmsendframe(ICHP_GW_NETWORK, 0x01, &network_info.state);
#endif
        if (network_info.appstate == EMBER_JOINED_NETWORK)
        {
            if (get_lampmode(LAMP_CHANGE))
            {
                sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_RPT, sizeof(lamp_t), &single_info.lamp); 
                clr_lampmode(LAMP_CHANGE);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void RouterTick ( void )
//* 功能        : router tick handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : called to check application timeouts, button events,
//                and periodically flash LEDs
//*------------------------------------------------*/
void RouterTick ( void )                           
{ 	
    if (network_info.appstate == EMBER_JOINED_NETWORK)
    {
        if (!network_info.substate)
        {
            network_info.substate = TRUE;
            single_info.address   = emberGetNodeId();                 // get logic short address
            set_lampmode(LAMP_NETWORK);
        }

        if (send1.wait)    // 50000 = 5s
        {
            if ((send1.wait % 5000) == 0x00)
            {
              Debug("\r\n wait %d", send1.wait);
            }
            send1.wait --;
        }
        else
        {
            if (!sea_msgisempty(&send1))         // 处理发送msg
            {
                pmsg_t ptr = sea_getmsg(&send1);
                network_info.unicast(ptr->id, COORDID, ptr->size, ptr->body);
                send1.wait = 3000;
            }
        }
    }       
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

