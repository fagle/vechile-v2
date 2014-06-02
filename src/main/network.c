
#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
msg_info_t            send1; 
sys_info_t         sys_info;
network_info_t network_info;

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void NET_Configuration ( void )
//* 功能        : network information configuration
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void NET_Configuration ( void )
{
    sea_memset(&network_info, 0x00, sizeof(network_info_t));
    
    network_info.endpoint    = ENDPOINT;
    network_info.stackstate  = EMBER_NO_NETWORK;
    network_info.appstate    = EMBER_NO_NETWORK;
    network_info.intpstate   = 0x00;
    network_info.error       = EMBER_NOERROR;
    network_info.status      = EMBER_SUCCESS;
    network_info.method      = SINK_FORM_NEW_NETWORK;
    network_info.substate    = 0x00;
    
    network_info.ember       = emberTick;
    network_info.formAndJoin = emberFormAndJoinTick;
    network_info.unicast     = sea_sendunimsg;
    network_info.multicast   = sea_sendmultimsg;
    
#ifdef SINK_APP  
    network_info.type        = EMBER_COORDINATOR;
    network_info.devid       = COORDID;              
    network_info.app         = CoordinatorTick;
    network_info.cmd         = sinkCommandHandler;
    network_info.init        = sinkInit;
    network_info.incoming    = ezspCoordinatorIncomingMessageHandler;
    network_info.msgSent     = ezspCoordinatorMessageSentHandler;
    network_info.stackStatus = emberCoordinatorStackStatusHandler;
    network_info.exception   = coordinatorExceptionHandler;
    network_info.timevent    = coordinatorTimeEventHandler;
    network_info.msevent     = coordinatorMsEventHandler;
#else                    
    network_info.type        = EMBER_ROUTER;
    network_info.devid       = ROUTERID;           
    network_info.cmd         = sensorCommandHandler;
    network_info.init        = sensorInit;
    network_info.incoming    = ezspRouterIncomingMessageHandler;
    network_info.msgSent     = ezspRouterMessageSentHandler;
    network_info.stackStatus = emberRouterStackStatusHandler;
    network_info.exception   = routerExceptionHandler;
    #ifdef ENABLE_VEHICLE
        network_info.app         = vehicleAppHandler;
        network_info.timevent    = vehicleTimeEventHandler;
        network_info.msevent     = vehicleMsEventHandler;
    #else
        network_info.app         = RouterTick;
        network_info.timevent    = routerTimeEventHandler;
        network_info.msevent     = NULL;
    #endif
#endif
  
#ifdef ENABLE_GATEWAY 
    network_info.gateway     = gatewayTickHandler;
#else    
    network_info.gateway     = NULL;
#endif  // ENABLE_GATEWAY    
  
    init_msginfo();                 // initialize msg_info structure body, default function is router message incoming handler                   
    init_singleinfo();              // initialize single_info structre body
    init_tableinfo();               // initialize table_info structure body
    sea_initframe(&uartfrm);        // initialize frame_t structure body
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_makeuniapsframe ( u8 clusterId, EmberApsFrame * ptr )
//* 功能        : make aps frame norrmal format
//* 输入参数    : u8 clusterId, EmberApsFrame * ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_makeuniapsframe ( u8 clusterId, EmberApsFrame * ptr )
{
    ptr->profileId           = sys_info.channel.profile;                                          // profile unique to this app
    ptr->clusterId           = clusterId;                                                         // message type
    ptr->sourceEndpoint      = network_info.endpoint;                                             // sensor endpoint
    ptr->destinationEndpoint = network_info.endpoint;                                             // sensor endpoint
    ptr->options             = EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EMBER_APS_OPTION_RETRY;  // none for multicast
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_makemultiapsframe ( u8 clusterId, EmberApsFrame * ptr )
//* 功能        : make aps frame norrmal format
//* 输入参数    : u8 clusterId, EmberApsFrame * ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_makemultiapsframe ( u8 clusterId, EmberApsFrame * ptr )
{
    ptr->profileId           = sys_info.channel.profile;     // profile unique to this app
    ptr->clusterId           = clusterId;                    // message type
    ptr->sourceEndpoint      = network_info.endpoint;        // sensor endpoint
    ptr->destinationEndpoint = network_info.endpoint;        // sensor endpoint
    ptr->options             = EMBER_APS_OPTION_NONE;        // none for multicast
    ptr->groupId             = MULTICAST_ID;                 // multicast ID unique to this app 
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendunimsg ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
{
    EmberStatus   status;
    EmberApsFrame apsFrame;
    EmberMessageBuffer buffer;
  
    if (msg == NULL || len == 0x00)    { len = 0x01; msg = (void *)&status; }

    buffer = emberFillLinkedBuffers((u8*)msg, len);          // copy the data into a packet buffer
    if (buffer == EMBER_NULL_MESSAGE_BUFFER)                    // check to make sure a buffer is available
    {
        Debug("\r\nTX ERROR [unimsg], OUT OF BUFFERS");
        return EMBER_NULL_MESSAGE_BUFFER;
    }
  
    sea_makeuniapsframe(clusterId, &apsFrame);
    status = emberSendUnicast(EMBER_OUTGOING_DIRECT, dest, &apsFrame, buffer);

    emberReleaseMessageBuffer(buffer);                                                            // done with the packet buffer
    Debug("\r\nTX [unimsg], clusterId 0x%02x ,status 0x%02x", clusterId, status);
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendunimsg_m ( u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq )
//* 功能        : send unicast message with narmal format
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendunimsg_m ( u8 clusterId, EmberNodeId dest, u8 len, void * msg, u16 seq )
{
    EmberStatus   status;
    EmberApsFrame apsFrame;
    EmberMessageBuffer buffer;
  
    if (msg == NULL || len == 0x00)    { len = 0x01; msg = (void *)&status; }

    buffer = emberFillLinkedBuffers((u8*)msg, len);          // copy the data into a packet buffer
    if (buffer == EMBER_NULL_MESSAGE_BUFFER)                    // check to make sure a buffer is available
    {
        Debug("\r\nTX ERROR [unimsg], OUT OF BUFFERS");
        return EMBER_NULL_MESSAGE_BUFFER;
    }
  
    sea_makeuniapsframe(clusterId, &apsFrame);
    apsFrame.profileId = seq;
    status = emberSendUnicast(EMBER_OUTGOING_DIRECT, dest, &apsFrame, buffer);

    emberReleaseMessageBuffer(buffer);                                                            // done with the packet buffer
    Debug("\r\nTX [unimsg], clusterId 0x%02x ,status 0x%02x", clusterId, status);
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_sendmultimsg ( u8 clusterId, u8 len, void * msg )
//* 功能        : send multicast message with normal format
//* 输入参数    : 无
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_sendmultimsg ( u8 clusterId, u8 len, void * msg )
{
    EmberStatus   status;
    EmberApsFrame apsFrame;
    EmberMessageBuffer buffer;

    if (msg == NULL || len == 0x00)    { len = 0x01; msg = (void *)&status; }

    buffer = emberFillLinkedBuffers((u8*)msg, len);          // copy the data into a packet buffer
    if (buffer == EMBER_NULL_MESSAGE_BUFFER)                    // check to make sure a buffer is available
    {
        Debug("\r\nTX ERROR [multmsg], OUT OF BUFFERS");
        return EMBER_NULL_MESSAGE_BUFFER;
    }

    sea_makemultiapsframe(clusterId, &apsFrame);
    status = emberSendMulticast(&apsFrame, 0x00, 0x08, buffer); 
      
    emberReleaseMessageBuffer(buffer);                                                            // done with the packet buffer
   
    Debug("\r\nTX [multicast],clusterId 0x%02x ,status 0x%02x", clusterId,status);
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_fragmentunicast ( u8 clusterId, EmberNodeId dest, u8 len, void * msg )
//* 功能        : 传输大数据
//* 输入参数    : u8 clusterId, EmberNodeId dest, u8 len, void * msg
//* 输出参数    : Ember network status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_fragmentunicast ( u8 clusterId, EmberNodeId dest, u16 len, void * msg )
{
    EmberStatus   status;
    EmberApsFrame apsFrame;
    EmberMessageBuffer buffer = NULL;
    
    if (msg == NULL)    { len = 0x01; msg = (void *)&status; }

    buffer = emberFillLinkedBuffers((u8*)msg, len);          // copy the data into a packet buffer
    if (buffer == EMBER_NULL_MESSAGE_BUFFER)                    // check to make sure a buffer is available
    {
        Debug("\r\nTX ERROR [fragment], OUT OF BUFFERS");
        return EMBER_NULL_MESSAGE_BUFFER;
    }
  
    sea_makeuniapsframe(clusterId, &apsFrame);
    status = emberFragmentSendUnicast(EMBER_OUTGOING_DIRECT, dest, &apsFrame, buffer, len / MAX_TOTAL_BLOCKS);
    emberReleaseMessageBuffer(buffer);
    
    if (status != EMBER_SUCCESS)
        sea_sendmsg(&send1, FRAGMENT, dest, clusterId, len, msg);
    Debug("\r\nTX [fragment], clusterId 0x%02x ,sent status 0x%02x", clusterId, status);
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_addmultigroup ( void )
//* 功能        : 调用者加入某group
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_addmultigroup ( void ) 
{
    EmberMulticastTableEntry *entry = &emberMulticastTable[MULTICAST_TABLE_INDEX];
    
    entry->multicastId = MULTICAST_ID;
    entry->endpoint    = network_info.endpoint;
    Debug("\r\nEVENT: setting multicast table entry");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_SendInterpanUnicast ( u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg )
//* 功能        : send interpan unicast
//* 输入参数    : u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg
//* 输出参数    : EmberStatus
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
EmberStatus sea_SendInterpanUnicast ( u16 addr, u16 paid, u16 dest, u16 cmd, u16 id, EmberEUI64 eui, u8 len, u8 * msg )
{
    InterPanHeader header;
    EmberMessageBuffer message;
    EmberMessageBuffer payload;
    EmberStatus status;

    header.shortAddress   = 0x00;                 // setup the interPAN header
    header.panId          = paid;
    header.hasLongAddress = FALSE;
    header.messageType    = INTER_PAN_UNICAST;    // INTER_PAN_BROADCAST; INTER_PAN_UNICAST
    header.profileId      = dest;
    header.clusterId      = cmd;
     
    if (id != 0x00)                               // if this is mcast set that up
    {
        header.messageType = INTER_PAN_MULTICAST;
        header.groupId     = id;
    }
  
    if (eui != NULL)                              // if this uses a long ID set that up
    {
        header.hasLongAddress = TRUE;
        MEMCOPY(header.longAddress, eui, EUI64_SIZE);
    }

    payload = emberFillLinkedBuffers(msg, len);   // put the payload into a message buffer
    if (payload == EMBER_NULL_MESSAGE_BUFFER) 
    {
        Debug("TX Error: NO BUFFs!\r\n"); 
        return EMBER_NO_BUFFERS;
    }

    message = makeInterPanMessage(&header, payload);  // make the interPAN message and send it
    status = emberSendRawMessage(message);
    emberReleaseMessageBuffer(message);           // release the buffer
    emberReleaseMessageBuffer(payload);

    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus sea_sendInterPanMsg(u8 type,u16 paid,u16 dest,u16 cmd, EmberEUI64* eui,u8 len,u8* msg)
//* 功能        : 子网之间传递信息
//* 输入参数    : u8 type,u16 paid, u16 dest, u16 cmd,  EmberEUI64* eui, u8 len,  u8* msg
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*-------------------------------------------*/
EmberStatus sea_SendInterPanMsg ( u8 type, u16 paid, u16 dest, u16 cmd, EmberEUI64 eui, u8 len, void * msg )
{
    InterPanHeader header;
    EmberMessageBuffer message;
    EmberMessageBuffer payload;
    EmberStatus status;

    header.shortAddress   = (type == INTER_PAN_BROADCAST) ? 0xffff : 0x00;   // setup the interPAN header
    header.panId          = paid;
    header.hasLongAddress = FALSE;
    header.messageType    = type;
    header.profileId      = dest;
    header.clusterId      = cmd;
  
    if (eui != NULL)                                  // if this uses a long ID set that up
    {
        header.hasLongAddress = TRUE;
        MEMCOPY(header.longAddress, eui, EUI64_SIZE);
    }
 
    payload = emberFillLinkedBuffers(msg, len);       // put the payload into a message buffer
    if (payload == EMBER_NULL_MESSAGE_BUFFER) 
    {
        Debug("TX Error: NO BUFFs!\r\n"); 
        return EMBER_NO_BUFFERS;
    }
 
    message = makeInterPanMessage(&header, payload);   // make the interPAN message and send it
    status = emberSendRawMessage(message);
    emberReleaseMessageBuffer(message);                // release the buffer
    emberReleaseMessageBuffer(payload);

    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_feedwatchdog ( void )
//* 功能        : feed watch dog
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_feedwatchdog ( void )
{
#ifndef DISABLE_WATCHDOG
    halResetWatchdog();
#endif    
}

////////////////////////////////////////////////////////////////////////////////
