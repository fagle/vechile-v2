
#include "network.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
int8u emberEndpointCount = 0x01;   // not removed, for zigbee-device.o(zigbee-pro-stack.a)
EmberEndpointDescription PGM endpointDescription = { DEF_PROFILE, 0x00, };
EmberEndpoint emberEndpoints[]  = {{ ENDPOINT, &endpointDescription },};

EmberEventControl millTickEvent = { 0x00, 0x00 };
EmberEventControl  secTickEvent = { 0x00, 0x00 };
EmberEventData     lampEvents[] = { {&millTickEvent, millTickHook},                                 
                                    {&secTickEvent, secTickHook},
                                    {NULL, NULL} };
/////////////////////////////////////////////////////////////////////////////////////////////
//
uart_t  serial_info; 

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : EmberStatus EZSP_Configuration ( void )
//* 功能        : ember network configuration
//* 输入参数    : 无
//* 输出参数    : EmberStatus status
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void EZSP_Configuration ( void )
{
    EmberStatus status;
  
    sea_memset(&network_info.params, 0x00, sizeof(EmberNetworkParameters));
    network_info.params.panId        = sys_info.channel.panid;    
    network_info.params.radioTxPower = sys_info.channel.power;
    network_info.params.radioChannel = sys_info.channel.channel;
    sea_memcpy(network_info.params.extendedPanId, sys_info.channel.expanid, EXTENDED_PAN_ID_SIZE);
    
    status = emberInit();                                 // emberInit must be called before other EmberNet stack functions
    if (status != EMBER_SUCCESS) 
    {
        Debug("\r\nERROR: emberInit 0x%x", status);       // the app can choose what to do here, if the app is running another device 
                                                          // then it could stay running and report the error visually for example. This app asserts.
        assert(FALSE);
    } 
    else 
        Debug( "\r\nEVENT: emberInit passed and succeed");
    
#ifdef SINK_APP             // SINK_APP  
    sinkFormNetwork();
#else                       // SENSOR_APP
    //sensorJoinNetwork();
    //sensorRejoinNetwork
#endif
  
#ifdef EMZ3118  
    emberSetTxPowerMode(EMBER_TX_POWER_MODE_BOOST_AND_ALTERNATE);
#endif  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void hourTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void hourTimeEventHandler ( void )
{
    if (network_info.type == EMBER_COORDINATOR && network_info.devid == COORDID)
        sea_sendmsg(&send1, MULTICAST, 0x00, ICHP_SV_DATE, sizeof(systime_t), &single_info.time);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void msTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void msTimeEventHandler ( void )
{
    if (network_info.msevent)
        network_info.msevent();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void secTimeEventHandler ( void )
//* 功能        : handler of time events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void secTimeEventHandler ( void )
{
    network_info.timevent();
    network_info.exception();                         //注释掉就不会加网
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void MiscellaneaHandler ( void )
//* 功能        : handler of user miscellanea events
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void MiscellaneaHandler ( void )
{
    emberFragmentTick();
    emberRunEvents(lampEvents);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void ApplicationHandler ( void )
//* 功能        : handler of user Application
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void ApplicationHandler ( void )
{
    network_info.ember(); 
    network_info.formAndJoin();
    network_info.app();                

    if (network_info.gateway)
        network_info.gateway();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : UART_Configuration
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void UART_Configuration ( void )
{
    // inititialize the serial port good to do this before emberInit, that way any errors that occur can be printed to the serial port.
//    if (SerialInit(APP_SERIAL, BAUD_19200, PARITY_NONE, 0x01) != EMBER_SUCCESS) 
#ifdef SINK_APP
    if (SerialInit(APP_SERIAL, BAUD_115200, PARITY_NONE, 0x01) != EMBER_SUCCESS) 
#else
#ifdef VEHICLE_RELEASE
    if (SerialInit(APP_SERIAL, BAUD_19200, PARITY_NONE, 0x01) != EMBER_SUCCESS) 
#else
    if (SerialInit(APP_SERIAL, BAUD_115200, PARITY_NONE, 0x01) != EMBER_SUCCESS) 
#endif
#endif
        SerialInit(APP_SERIAL, BAUD_19200, PARITY_NONE, 0x01);
    Debug( "reset: 0x%02x\r\n", (PGM_P)halGetResetString());    // print the reason for the reset
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : LED_Configuration
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void LED_Configuration ( void )
{
#ifdef SINK_APP  
    //halToggleLed(BOARDLED1);     // say hello with LEDs
    //halToggleLed(BOARDLED3);
#else   
    
    //appSetLEDsToInitialState();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void ShowCopyrights ( void )
//* 功能        : show copyrights
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void ShowCopyrights ( void )
{
    sea_memcpy(sys_info.dev.eui, emberGetEui64(), EUI64_SIZE);
    DBG("\r\n%s", network_info.type == EMBER_COORDINATOR ? "--coordinator--" : "--router--");  // print a startup message
  
#ifdef EMZ3118
    Debug("\r\npower amplify configuration:");
    Debug("\r\nGPIO_PAOUT is %x", GPIO_PAOUT_REG);
    Debug("\r\nGPIO_PACFGH is %x", GPIO_PACFGH);
    Debug("\r\nGPIO_PACFGL is %x", GPIO_PACFGL);
    Debug("\r\nGPIO_PCCFGH is %x", GPIO_PCCFGH);   
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void DEV_Configuration ( void )
//* 功能        : devices initialize
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void DEV_Configuration ( void )
{
    emberMacPassthroughFlags = EMBER_MAC_PASSTHROUGH_APPLICATION; // EMBER_MAC_PASSTHROUGH_SE_INTERPAN;
#ifdef MOBILE_APP
    emberMacPassthroughFlags = EMBER_MAC_PASSTHROUGH_SE_INTERPAN; // EMBER_MAC_PASSTHROUGH_SE_INTERPAN;
#endif
    network_info.init();
    ShowCopyrights();
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : Security_Configuration
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void Security_Configuration ( void )
{
    // when acting as a trust center, a portion of the address table is needed to keep track of shortID-to-EUI mappings. The mapping is
    // needed during the key exchanges that happen immediately after a join. This function is found in app/util/security/security-address-cache.c.
    // The sink allocates address table entries from 0 to (SINK_ADDRESS_TABLE_SIZE-1) for the use of the sink app. It allocates 
    // entries from SINK_ADDRESS_TABLE_SIZE to (SINK_ADDRESS_TABLE_SIZE +  SINK_TRUST_CENTER_ADDRESS_CACHE_SIZE) for the trust center use. 
    // The defines used here are defined in app/sensor/sensor-configuration.h
#ifdef SINK_APP
    securityAddressCacheInit(SINK_ADDRESS_TABLE_SIZE, SINK_TRUST_CENTER_ADDRESS_CACHE_SIZE);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void Bootloader_Configuration ( void )
//* 功能        : bootloader configuration initialize
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void Bootloader_Configuration ( void )
{
#ifdef USE_BOOTLOADER_LIB
    // Using the same port for application serial print and passthru bootloading.  User must be careful not to print anything to the port
    // while doing passthru bootload since that can interfere with the data stream. Also the port's baud rate will be set to 115200 kbps 
    // in order to maximize bootload performance.
    bootloadUtilInit(APP_SERIAL, APP_SERIAL);
#endif // USE_BOOTLOADER_LIB
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void main ( void )
//* 功能        : main programme
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
int main ( void )
{
    HAL_Configuration();      // Initialize the hal    
    UART_Configuration();     // inititialize serial interface 
    LED_Configuration();
    
#ifdef SENSOR_APP
    ADC_Configuration();      // ADC初始化   
#endif  // SENSOR_APP    
    PAM_Configuration();
    TICK_Configuration();     // time events initialize
    SYS_Configuration();      // initialize system information
    NET_Configuration();      // initialize net information 
    PWM_Configuration();      // Init pwm 
#ifdef ENABLE_GATEWAY
    SPI_Configuration(); 
    GWY_Configuration();
#endif  // ENABLE_GATEWAY    
  
    Security_Configuration(); // Security Config    
    Bootloader_Configuration();
    EZSP_Configuration();
    DEV_Configuration();      // init application state(APP初始化)

    while (TRUE) 
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
        network_info.tick ++;

#ifndef DISABLE_WATCHDOG
        halResetWatchdog();
#endif    
        network_info.cmd();  
        ApplicationHandler();
        MiscellaneaHandler();
    }
}

////////////////////////////////////////////////////////////////////////////////

