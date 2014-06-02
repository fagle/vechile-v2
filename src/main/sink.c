// *******************************************************************
//  sink.c
// *******************************************************************
#include "network.h"
#include "sink.h"
#include "security.h" 
#include "console.h"
#include "frame.h"
 
#ifdef SINK_APP
// *******************************************************************
//const and globles 常量或者全局变量
// Define the concentrator type for use when sending "many-to-one"
// route requests.
//
// In EmberZNet 2.x and earlier a node sending this type of request
// was referred to as an "aggregator". In the ZigBee specification and
// EmberZNet 3 it is referred to as a "concentrator".
// 
// EMBER_HIGH_RAM_CONCENTRATOR is used when the caller has enough
// memory to store source routes for the whole network.
// EMBER_LOW_RAM_CONCENTRATOR is used when the concentrator has
// insufficient RAM to store all outbound source routes.
//u16 concentratorType = EMBER_LOW_RAM_CONCENTRATOR;

#ifdef ENABLE_VEHICLE 
static u8      lamp_rept[((MAXCARDS >> 0x03) + 0x01)];         // for card state report bit maps
#else
static u8      lamp_rept[((MAXLAMPS >> 0x03) + 0x01)];        // for state report bit maps
#endif
static u16     lamp_addr[MAXLAMPS];                           // logic address list of lamps
road_t         road_info;                                     // sum information of each road 

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspCoordinatorMessageSentHandler
//* 功能        : Called when a message has completed transmission --
//                status indicates whether the message was successfully
//                transmitted or not.
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ezspCoordinatorMessageSentHandler ( EmberOutgoingMessageType type,
                                         u16 indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         EmberMessageBuffer message,
                                         EmberStatus status )
{
    if (apsFrame->clusterId == send1.curmsg->id)   // sequence sometime error, use cluster id to ensure
    {
        Debug("\r\n[coord]send status = %02x, id = %02x", status, apsFrame->clusterId);
        if (status != EMBER_SUCCESS)               // send the command fail.
        {
            if (++ single_info.retry <= 0x02)
                sea_sendmsg(&send1, MULTICAST, send1.curmsg->dest, send1.curmsg->id, send1.curmsg->size, send1.curmsg->body);   // send it again
        }
        else
            single_info.retry = 0x00;
    }
}

#ifdef USE_BOOTLOADER_LIB
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilQueryResponseHandler
//* 功能        : When a device sends out a bootloader query, the bootloader
//                query response messages are parsed by the bootloader-util
//                libray and and handled to this function. This application
//                simply prints out the EUI of the device sending the query
//                response.
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void bootloadUtilQueryResponseHandler ( boolean bootloaderActive,
                                        u16 manufacturerId,
                                        u8 *hardwareTag,
                                        EmberEUI64 targetEui,
                                        u8 bootloaderCapabilities,
                                        u8 platform,
                                        u8 micro,
                                        u8 phy,
                                        u16 blVersion )
{
    Debug("RX [BL QUERY RESP] eui: ");
    printEUI64((EmberEUI64*)targetEui);
    Debug(" running %p\r\n", bootloaderActive ? "bootloader" : "stack");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bootloadUtilLaunchRequestHandler
//* 功能        : This function is called by the bootloader-util library 
//                to ask the application if it is ok to start the bootloader.
//                This happens when the device is meant to be the target of
//                a bootload. The application could compare the manufacturerId 
//                and/or hardwareTag arguments to known values to enure that 
//                the correct image will be bootloaded to this device.
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
boolean bootloadUtilLaunchRequestHandler ( u16 manufacturerId,
                                           u8 *hardwareTag,
                                           EmberEUI64 sourceEui ) 
{
    // TODO: Compare arguments to known values.
    // TODO: Check for minimum required radio signal strength (RSSI).
    // TODO: Do not agree to launch the bootloader if any of the above conditions
    // are not met.  For now, always agree to launch the bootloader.
    return TRUE;
}
#endif // USE_BOOTLOADER_LIB

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_printlampstate ( void )
//* 功能        : print state of all lamps
//* 输入参数    : 无 
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_printlampstate ( void )
{
#ifndef ENABLE_GATEWAY    
    DBG("\r\nopen state of lamps:");
    for (u8 i = 0x00; i < road_info.map.size; i ++)
        Debug(" %02x", road_info.map.state[i]);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_reporthandler ( plamp_t ptr, u16 sender)
//* 功能        : lamp report handler
//* 输入参数    : plamp_t ptr, u16 sender
//* 输出参数    : true/flase
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  sea_reporthandler ( plamp_t ptr, u16 sender )
{
    u16 number;
    
    if (ptr == NULL)      return 0x00;
    
    if (sys_info.ctrl.app == eVehicle)
        number = ptr->vehicle.number;
    else
        number = ptr->light.number;
    
    if (number > sys_info.ctrl.maxdev || number == 0x00)
        return 0x00;
    
    number --;
    road_info.error(ptr, sender);
    
#ifndef ENABLE_VEHICLE     
    status = ptr->light.status ? set_lampbitmap(road_info.map.state, number) : clr_lampbitmap(road_info.map.state, number);
#endif
    road_info.update(number, sender);
    
    return 0x01;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void update_gatewaytable ( u16 address )
//* 功能        : update gateway address table
//* 输入参数    : u16 address
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void update_gatewaytable ( u16 address )             //！！
{
#ifdef ENABLE_GATEWAY
    for (u16 i = 0x00; i < sys_info.maxdev; i ++)
    {
        if (road_info.map.addr[i] == address)
        {
            u8 j;
            for (j = 0x00; j < MAXGWSIZE; j ++)
            {
                if (network_info.gwaddr[j] == address)
                    return;
                if (!network_info.gwaddr[j])
                    break;
            }
            if (j < MAXGWSIZE)
                network_info.gwaddr[j] = address;
            return;
        }
    }
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : ezspCoordinatorStackStatusHandler(EmberStatus status)
//* 功能        : this is called when the stack status changes
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void emberCoordinatorStackStatusHandler ( EmberStatus status )
{
    switch (status) 
    {
        case EMBER_NETWORK_UP:
            Debug("\r\nEVENT: stackStatus now EMBER_NETWORK_UP\r\n");
            {
                EmberStatus status;
                EmberNetworkParameters networkParams;
                status = emberGetNetworkParameters(&networkParams);
                if (status == EMBER_SUCCESS) 
                {
                    switch (network_info.method) 
                    {
                        case SINK_USE_NETWORK_INIT:
                            Debug("FORM : network started using network init\r\n");
                            break;
                        case SINK_FORM_NEW_NETWORK:
                            Debug("FORM : new network formed\r\n");
                            break;
                        case SINK_USE_SCAN_UTILS:
                            Debug("FORM : new network formed by scanning\r\n");
                            break;
                    }
                    Debug("\r\n     : channel 0x%02x, panid 0x%04x", networkParams.radioChannel, networkParams.panId);
                    Debug("\r\n     : expended id %s", networkParams.extendedPanId);
#ifdef ENABLE_GATEWAY    
                    sea_gwmsendframe(ICHP_GW_CHANNEL, sizeof(channel_t), (u8 *)&sys_info.channel);
#endif            
                }
            }
            sea_addmultigroup();  // Add the multicast group to the multicast table, this is done after the stack comes up
            break;
    
        case EMBER_NETWORK_DOWN:
            Debug("\r\nEVENT: stackStatus now EMBER_NETWORK_DOWN\r\n");
            break;

        case EMBER_JOIN_FAILED:
            Debug("\r\nEVENT: stackStatus now EMBER_JOIN_FAILED");
            break;

        default:
            Debug( "\r\nEVENT: stackStatus now 0x%02x\r\n", status);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sinkFormNetwork
//* 功能        : 协调器组网
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sinkFormNetwork ( void )
{
    // structure to store necessary network parameters of the node, (which are panId, enableRelay, radioTxPower, and radioChannel)
    EmberStatus status;

    // try and rejoin the network this node was previously a part of. if status is not EMBER_SUCCESS then the node didn't 
    // rejoin it's old network sink nodes need to be coordinators, so ensure we are a coordinator
    sensorCommonSetupSecurity();                               // Set the security keys and the security state - specific to this application, 
                                                               // all variants of this application (sink, sensor, sleepy-sensor, mobile-sensor) 
                                                               // need to use the same security setup. This function is in app/sensor/common.c. 
                                                               // This function should only be called when a network is formed as the act of setting 
                                                               // the key sets the frame counters to 0. On reset and networkInit this should not be called.
#ifdef USE_HARDCODED_NETWORK_SETTINGS                          // Bring the network up.++固化网络参数
    Debug("\r\nFORM : attempting to form network");         // tell the user what is going on
    Debug("\r\n     : using channel 0x%2x, panid 0x%4x, ", network_info.params.radioChannel, network_info.params.panId);
    Debug("\r\n     : expended id %s", network_info.params.extendedPanId);
    
    network_info.method = SINK_FORM_NEW_NETWORK;               // set the mode we are using
    status = emberFormNetwork(&network_info.params);           // attempt to form the network
    if (status != EMBER_SUCCESS) 
    {
        Debug("\r\nERROR: from emberFormNetwork: 0x%x", status);
        assert(FALSE);
    }
#else                                                           // ++扫描确定网络参数
    network_info.method = SINK_USE_SCAN_UTILS;                  // set the mode we are using
    Debug("\r\nFORM : scanning for an available channel and panid\r\n");  // tell the user what is going on
    emberScanForUnusedPanId(EMBER_ALL_802_15_4_CHANNELS_MASK, 0x05);         // Use a function from app/util/common/form-and-join.c that scans 
                                                                             // and selects a quiet channel to form on. Once a PAN id and channel
                                                                             // are chosen, calls emberUnusedPanIdFoundHandler.
#endif  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_lamperror ( plamp_t ptr, u16 sender )
//* 功能        : lamp error handler
//* 输入参数    : plamp_t ptr, u16 sender
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_lamperror ( plamp_t ptr, u16 sender )
{
    u8 fail;
    
    if (sys_info.ctrl.app == eVehicle)
    {
        fail = ptr->vehicle.fail;
        if (fail & WRONG_WAY)
        {
        }
        if (fail & MISS_CARD)
        {
        }
        if (fail & END_LINE)
        {
//            uartfrm.put(&uartfrm, ICHP_SV_END, sender, 0x01, &ptr->vehicle.card);
        }
        if (fail & MEET_OBSTACLE)
        {
        }
        if (fail & EMPTY_TABLE)
        {
        }
    }
    else
    {
        fail = ptr->light.fail;
        if (fail & LAMP_ER_OV)
        {
        }
        if (fail & LAMP_ER_OC)
        {
        }
        if (fail & LAMP_ER_PT)
        {
        }
        if (fail & LAMP_ER_SC)
        {
        }
        if (fail & LAMP_ER_CB)
        {
        }
    }
    if (fail & LAMP_ER_TM)
    {
        sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_DATE, sizeof(systime_t), &single_info.time);
    }
    if (fail & LAMP_ER_NP)
    {
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u16 sea_getaddress ( u16 number )
//* 功能        : get address
//* 输入参数    : u16 number
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u16 sea_getaddress ( u16 number )
{
    if (number == 0x00 || number > sys_info.ctrl.maxdev)
        return 0x00;
    
    return road_info.map.addr[number - 0x01];
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u16 sea_getnumber ( u16 address )
//* 功能        : get number
//* 输入参数    : u16 address
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u16 sea_getnumber ( u16 address )
{
    if (address == 0x00)
        return 0x00;
    
    for (u16 i = 0x00; i < sys_info.ctrl.maxdev; i ++)
        if (road_info.map.addr[i] == address)
            return i + 0x01;
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_updatelogic ( u16 number, u16 sender )
//* 功能        : update road state and address table
//* 输入参数    : u16 number, u16 sender
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_updatelogic ( u16 number, u16 sender )
{
    if (road_info.map.addr[number] != sender)
        road_info.map.addr[number] = sender;                 //it is a new node according to this net

    update_gatewaytable(sender);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sinkInit ( void )
//* 功能        : 初始化sink节点信息
//*             : 
//*             : 检查有无节点掉出
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 应用层：原来在Lamp_Configuration
//*------------------------------------------------*/
void sinkInit ( void ) 
{
    sea_memset(&road_info, 0x00, sizeof(road_t));
    sea_memset(lamp_addr, 0x00, sizeof(u16) * MAXLAMPS);
    
    road_info.map.max   = sys_info.ctrl.maxdev;
    road_info.map.addr  = lamp_addr;
#ifdef ENABLE_VEHICLE 
    road_info.map.size  = (MAXCARDS >> 0x03) + 0x01;
#else
    road_info.map.size  = ((sys_info.ctrl.maxdev / 0x08) + 0x01);
#endif    
    sea_memset(lamp_rept, 0x00, road_info.map.size); 
    road_info.map.state = lamp_rept;
    
    road_info.state     = OPEN_LAMPS;
    road_info.assign    = sea_lampnumberassign;                      // get lamp number by search and assign; 
    road_info.update    = sea_updatelogic;
    road_info.number    = sea_getnumber;
    road_info.address   = sea_getaddress;
    road_info.error     = sea_lamperror;
    
    sea_configprint();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sinkCommandHandler ( void )
//* 功能        : 处理串口信息
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sinkCommandHandler ( void ) 
{   
    u8 ch;
    frame_t   fr;
    
    if (!serial_info.ept(&serial_info))
    {
        ch = serial_info.get(&serial_info);
        uartfrm.recv(&uartfrm, ch);
        if (uartfrm.state == PRT_IDLE)
            sea_roadkeyboardinput(ch);     
    }

    if (uartfrm.get(&uartfrm, &fr) != NULL)
        sea_parseroadframe(&fr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u16 sea_lampassign ( void )
//* 功能        : assign lamp number by search empty number 
//* 输入参数    : 无
//* 输出参数    : number of lamp
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u16 sea_lampnumberassign ( void )
{
#ifndef ENABLE_GATEWAY    
    u16 num;
  
    for (num = 0x00; num < sys_info.ctrl.maxdev; num ++)
    {
        if (road_info.map.addr[num] == 0x00)
            return num + 0x01;
    }
    return 0xffff;
#else
    return DEFALUTLAMPNUM;
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif //SINK_APP

/////////////////////////////////////////////////////////////////////////////////////
