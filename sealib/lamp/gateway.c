// *******************************************************************
//  gateway.c
// *******************************************************************
#include "gateway.h"
#include "common.h"
#include "serial.h"
#include "network.h"

//#ifdef ENABLE_GATEWAY

////////////////////////////////////////////////////////////////////////
// constants, static and globals
const  gw_frame_t  tframe1 = { ICHP_GW_TEST, 0x05, GWMAXCNT, TRUE, 0x01, 0xff, 0xfe, 0x04, 0x05, };

static gw_link_t  gw_link;
static gw_frame_t  rframe;
gw_frame_t       gw_frame;
gw_info_t         gw_info;

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pgw_list_t sea_getgwlist ( void )
//* 功能        : get the pointer of structure gw_list_t
//* 输入参数    : u8 which
//* 输出参数    :  pointer of gw_list_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pgw_list_t sea_getgwlist ( u8 which )
{
    return &gw_list1[which & 0x01];
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pgw_frame_t sea_getcurrecvframe ( u8 which )
//* 功能        : get current recv and send body of frame pointer
//* 输入参数    : u8 which
//* 输出参数    : pointer of frame_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pgw_frame_t sea_getcurrentframe ( u8 which )  
{
    pgw_frame_t ptr = NULL;
    
    if (which == RECVLIST)
        ptr = &rframe1[sea_getgwlist(which)->in];
    else if (which == SENDLIST)
        ptr = &sframe1[sea_getgwlist(which)->in];
    else
        return NULL;
    
    sea_gwreleaseframe(ptr);
    return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : pgw_frame_t sea_getframelist ( u8 which )
//* 功能        : get body of frame pointer
//* 输入参数    : u8 which
//* 输出参数    : pointer of frame_t
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
pgw_frame_t sea_getframelist ( u8 which )  
{
    pgw_list_t  ptr = sea_getgwlist(which);
    pgw_frame_t frm;

    if (!ptr->count || !ptr->frame[ptr->out]->state)
        return NULL;
    
//    sea_printf("\n[gw] get recv count %d, send count %d", sea_getgwlist(RECVLIST)->count, sea_getgwlist(SENDLIST)->count);
    ptr->count --;
    frm = ptr->frame[ptr->out];
    ptr->out ++;
    if (ptr->out >= GWMAXLIST)
        ptr->out = 0x00;
    return frm;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool sea_framelistisempty ( u8 which )
//* 功能        : is empty of list.
//* 输入参数    : u8 which
//* 输出参数    : boolean
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool sea_framelistisempty ( u8 which )  
{
    return (bool)(sea_getgwlist(which)->count == 0x00);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_putframelist ( u8 which )
//* 功能        : put body of frame pointer
//* 输入参数    : u8 which, pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_putframelist ( u8 which, pgw_frame_t frm )
{
    pgw_list_t ptr = sea_getgwlist(which);

//    if (!frm->state)   return;
    if (frm->state && ptr->count < GWMAXLIST)
    {
//        sea_printf("\n[gw] put recv count %d, send count %d", sea_getgwlist(RECVLIST)->count, sea_getgwlist(SENDLIST)->count);
        ptr->frame[ptr->in] = frm;
        ptr->count ++;
        ptr->in ++;
        if (ptr->in >= GWMAXLIST)
            ptr->in = 0x00;
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_gwislinkempty ( void )
//* 功能        : is empty of link_t
//* 输入参数    : 无
//* 输出参数    : true/false
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_gwislinkempty ( void )  
{
    return gw_link.count == 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8 sea_gwgetlink ( void )
//* 功能        : get ch from link_t
//* 输入参数    : 无
//* 输出参数    : char
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8 sea_gwgetlink ( void )  
{
    u8  ch;
    
    gw_link.count --;
    ch = gw_link.body[gw_link.out];
    gw_link.out ++;
    if (gw_link.out >= GWLINKSIZE)
        gw_link.out = 0x00;
    return ch;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwputlink ( u8 ch )
//* 功能        : put ch to link_t
//* 输入参数    : u8 ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwputlink ( u8 ch )
{
    gw_link.body[gw_link.in] = ch;
    gw_link.count ++;
    gw_link.in ++;
    if (gw_link.in >= GWLINKSIZE)
        gw_link.in = 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwreleaseframe ( pgw_frame_t ptr )
//* 功能        : release receiving frame 
//* 输入参数    : pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwreleaseframe ( pgw_frame_t ptr )
{
    ptr->state = FALSE;      // initial flag of receiving frame
    ptr->cnt   = GWMAXCNT;   // initial counter of receiving frame
}

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwreleaseframe ( pgw_frame_t ptr )
//* 功能        : release receiving frame 
//* 输入参数    : pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwframeready ( pgw_frame_t ptr )
{
    ptr->cnt   = GWMAXCNT;
    ptr->state = TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void GWY_Configuration ( void )
//* 功能        : initialize gateway information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void GWY_Configuration ( void )
{
#if 0  
    for (u8 i = 0x00; i < GWMAXLIST; i ++)
    {
        sea_memset(&rframe1[i], 0x00, sizeof(gw_frame_t)); 
        sea_gwreleaseframe(&rframe1[i]);
    
        sea_memset(&sframe1[i], 0x00, sizeof(gw_frame_t)); 
        sea_gwreleaseframe(&sframe1[i]);
    }
//    sfrm = rfrm = NULL;
    sea_memset(gw_list1, 0x00, sizeof(gw_list_t) * 0x02);
#endif    
    sea_memset(&gw_info, 0x00, sizeof(gw_info_t));
    sea_memset(&gw_link, 0x00, sizeof(gw_link_t));
    sea_gwreleaseframe(&rframe);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_gwparseframe ( pgw_frame_t ptr )
//* 功能        : parse received frame
//* 输入参数    : pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_gwparseframe ( pgw_frame_t ptr )
{
    if (ptr == NULL)
        return;
    
    if (ptr->state)
    {
        sea_gwmessagehandler(ptr);
        sea_gwreleaseframe(ptr);
    }
}

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_verifyrecvframe ( pgw_frame_t ptr )
//* 功能        : verify received frame
//* 输入参数    : pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_gwverifyrecvframe ( pgw_frame_t ptr )
{
    gw_frame_t frm;
    u8 i, j;
    
    sea_memcpy(&frm, ptr, sizeof(gw_frame_t));
    for (j = i = 0x00; i < ptr->len; i ++, j ++)
    {
        if (ptr->body[i] == SPECIALCHAR && ptr->body[i + 0x01] == SPECIALCHAR)
        {
            frm.body[j] = SPIEMPTY;
            frm.len --;
            i ++;
        }
        else
            frm.body[j] = ptr->body[i];
    }
    sea_memcpy(ptr, &frm, sizeof(gw_frame_t));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_gwmakeframe ( pgw_frame_t ptr )
//* 功能        : make a send frame with spi
//* 输入参数    : pgw_frame_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_gwverifysendframe ( pgw_frame_t ptr )
{
    gw_frame_t frm;
    u8 i, j;
    
    sea_memcpy(&frm, ptr, sizeof(gw_frame_t));
    for (j = i = 0x00; i < ptr->len; i ++, j ++)
    {
        if (ptr->body[i] == SPIEMPTY)
        {
            frm.body[j ++] = SPECIALCHAR;
            frm.body[j]    = SPECIALCHAR;
            frm.len ++;
        }
        else
            frm.body[j] = ptr->body[i];
    }
    sea_memcpy(ptr, &frm, sizeof(gw_frame_t));
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_gwmsendframe ( u8 id, u8 len, u8 * value )
//* 功能        : send a frame with spi
//* 输入参数    : u8 id, u8 len, u8 * value
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwmsendframe ( u8 id, u8 len, u8 * value )
{
#if 0  
    pgw_frame_t frm = sea_getcurrentframe(SENDLIST);
    
    sea_memset(frm, 0x00, sizeof(gw_frame_t));
    frm->cmd = id;
    frm->len = len;
    for (u8 i = 0x00; i < len; i ++)
        frm->body[i] = value[i];
    sea_gwverifysendframe(frm);
    sea_putframelist(SENDLIST, frm); 
#else
    sea_gwputlink(PREFIX);
    sea_gwputlink(id);
    sea_gwputlink(len);
    for (u8 i = 0x00; i < len; i ++)
    {
        if (value[i] == SPIEMPTY)
        {
            sea_gwputlink(SPECIALCHAR);
            sea_gwputlink(SPECIALCHAR);
        }
        else
            sea_gwputlink(value[i]);
    }
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void gwputframebyte ( u8 ch )
//* 功能        : send and receive a char by spi
//* 输入参数    : u8 ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void gwputframebyte ( u8 ch )
{
    if (rframe.state)
        return;
    
    if (ch == PREFIX && rframe.cnt == GWMAXCNT)
        rframe.cnt = 0x00;
    else if (rframe.cnt == 0x00)
    {
        rframe.cmd = ch;
        rframe.cnt ++;
    }
    else if (rframe.cnt == 0x01)
    {
        rframe.len = ch > GWDATASIZE ? GWDATASIZE : ch;
        rframe.cnt ++;
    }
    else // if (rfrm->cnt >= 0x02 && rfrm->cnt <= DATASIZE + 0x02)
    {
        if (rframe.cnt > 0x02)
        {
            if (ch == SPECIALCHAR && rframe.body[rframe.cnt - 0x03] == SPECIALCHAR)
            {
                rframe.cnt --;
                ch = SPIEMPTY;
            }
        }
        rframe.body[rframe.cnt - 0x02] = ch;
        if (rframe.cnt ++ > rframe.len)
        {
//            sea_gwverifyrecvframe(&rframe);
            rframe.state = TRUE;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_gwsendrecvframe ( void )
//* 功能        : send and receive a char by spi
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_gwsendrecvframe ( void )
{
#if 0  
    u8 temp;
    
    if (rfrm == NULL)
        rfrm = sea_getcurrentframe(RECVLIST);
    if (sfrm == NULL)
        sfrm = sea_getframelist(SENDLIST);
    
    if (sfrm)
    {
        if (sfrm->cnt == GWMAXCNT)
        {
            temp = spi_readwrite(PREFIX);
            sfrm->cnt = 0x00;
        }
        else if (sfrm->cnt == 0x00)
        {
            temp = spi_readwrite(sfrm->cmd);
            sfrm->cnt ++;
        }
        else if (sfrm->cnt == 0x01)
        {
            temp = spi_readwrite(sfrm->len);
            sfrm->cnt ++;
        }
        else
        {
            if (sfrm->cnt - 0x02 >= sfrm->len)
            {
                sea_gwreleaseframe(sfrm);
                sfrm = NULL;
            }
            else
            {
                temp = spi_readwrite(sfrm->body[sfrm->cnt - 0x02]);
                sfrm->cnt ++;
            }
        }
    }
    else
        temp = spi_readwrite(SPIEMPTY);
    
    if (temp != SPIEMPTY && rfrm->state == FALSE)
    { 
        if (temp == PREFIX && rfrm->cnt == GWMAXCNT)
            rfrm->cnt = 0x00;
        else if (rfrm->cnt == 0x00)
        {
            rfrm->cmd = temp;
            rfrm->cnt ++;
        }
        else if (rfrm->cnt == 0x01)
        {
            rfrm->len = temp > GWDATASIZE ? GWDATASIZE : temp;
            rfrm->cnt ++;
        }
        else // if (rfrm->cnt >= 0x02 && rfrm->cnt <= DATASIZE + 0x02)
        {
            if (rfrm->cnt - 0x02 >= rfrm->len)
            {
                sea_printf("\n[gw] recv frame id %02x, body:", rfrm->cmd);
                for (u8 i = 0x00; i < rfrm->len; i ++)
                    sea_printf(" %02x", rfrm->body[i]);
                sea_gwverifyrecvframe(rfrm);
                rfrm->state = TRUE;
                sea_putframelist(RECVLIST, rfrm); 
                rfrm        = NULL;
            }
            else
            {
                rfrm->body[rfrm->cnt - 0x02] = temp;
                rfrm->cnt ++;
            }
        }
    }
#else
    u8  temp = 0x00;

    while (temp != SPIEMPTY)
    {
        temp = sea_gwislinkempty() ? (spi_readwrite(SPIEMPTY)) : (spi_readwrite(sea_gwgetlink()));
        if (temp != SPIEMPTY)
            gwputframebyte(temp);  
        if (rframe.state == TRUE)
            break;
    }
        
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : sea_gwsendtestframe ( void )
//* 功能        : send a test frame with spi
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_gwsendtestframe ( void )
{
#if 0  // test version
    pgw_frame_t ptr = sea_getcurrentframe(SENDLIST);

    sea_memcpy(ptr, (void *)&tframe1, sizeof(gw_frame_t));
    sea_gwverifysendframe(ptr);
    sea_putframelist(SENDLIST, ptr); 
#else
    sea_gwmsendframe(tframe1.cmd, tframe1.len, (u8 *)tframe1.body);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gatewayTickHandler ( void )
//* 功能        : coordinatior and router tick handler in gateway
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void gatewayTickHandler ( void )
{
    if (network_info.type == EMBER_COORDINATOR)
    {
        ATOMIC
        (
            spi_start();
            sea_gwsendrecvframe();
            spi_end();
        )
    }
    else if (network_info.type == EMBER_ROUTER)
    {
        if (spi_isnotempty())
            ATOMIC(sea_gwsendrecvframe();)
    }

#if 0    
    if (!sea_gwislinkempty(RECVLIST))
        sea_printf(" %02x", sea_gwgetlink(RECVLIST));
      
    if (!sea_framelistisempty(RECVLIST))
        sea_gwparseframe(sea_getframelist(RECVLIST));
    
    while (!sea_gwislinkempty(RECVLIST))
    {
        u8 ch = sea_gwgetlink(RECVLIST);
        sea_printf(" %02x", ch);
        gwputframebyte(ch);  // sea_gwgetlink(RECVLIST));  
    }
#endif
    
    if (rframe.state)
    {
#if 0    
        sea_printf("\n[gw] recv frame id %02x, body:", rframe.cmd);
        for (u8 i = 0x00; i < rframe.len; i ++)
            sea_printf(" %02x", rframe.body[i]);
#endif        
        sea_gwparseframe(&rframe);
        sea_gwreleaseframe(&rframe);
    }
}
///////////////////////////////////////////////////////////////////////////
//#endif  // ENABLE_GATEWAY



///////////////////////////////////////////////////////////////////////////
// the following lines wrote by guo tailei
#if 0   // removed by renyu 2011/09/23
////////////////////////////////////////////////////////////////////////
// constants and globals
cmd_pkg_info_t  cmdpkg_info;
lamp_pkg_info_t lamppkg_info;

////////////////////////////////////////////////////////////////////////
// static decalaretion functions
static void CmdPkgHanlder(void);
static void LampPkgHanlder(void);

#if defined(SINK_APP) && defined(ENABLE_GATEWAY) 
  static void GateWay_CO_Init(void);
  static bool isCmdPkgSet(void);
  static void parseCmdPkg( void );
#elif defined(SENSOR_APP) && defined(ENABLE_GATEWAY) 
  static void GateWay_RT_Init(void);
  static void gwPollForLampData(void);
  static void forwardLampPkg( void );
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void GWY_Configuration ( void )
//* 功能        : initialize gateway information
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void GWY_Configuration ( void )
{
    sea_memset(&cmdpkg_info,  0x00, sizeof(cmd_pkg_info_t));
    sea_memset(&lamppkg_info, 0x00, sizeof(lamp_pkg_info_t));
  
    sea_memset(&rframe, 0x00, sizeof(frame_t)); 
    sea_gwreleaseframe(&rframe);
    
    sea_memset(&sframe, 0x00, sizeof(frame_t)); 
    sea_gwreleaseframe(&sframe);

#if defined(SINK_APP) && defined(ENABLE_GATEWAY)
    GateWay_CO_Init();
#elif defined(SENSOR_APP) && defined(ENABLE_GATEWAY)
    sea_printf("\nsample frame id %02x, len %02x, cnt %02x, state %02x, body ", tframe.cmd, tframe.len, tframe.cnt, tframe.state);
    for (u8 i = 0x00; i < tframe.len; i ++)
        sea_printf("%02x ", tframe.body[i]);
    GateWay_RT_Init();
#endif  
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void CmdPkgHanlder(void)
//* 功能        : 网关CO，需要根据情况转发，重组cmd_pkg；
//*             : 而对于网关ZR,仅需要填充本pkg就可，置BUSY位；
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void CmdPkgHanlder ( void )
{
    if (cmdpkg_info.status == BUSY)
    {
#if defined(SINK_APP) && defined(ENABLE_GATEWAY)     // 解析，然后广播出去
        parseCmdPkg(); 
        printGwPkg();
    
        sea_memset(&cmdpkg_info.cmd_pkg,0x00,sizeof(cmd_format_t));
        sea_printf("\n->cmdpkg_info reset");
    
#elif defined(SENSOR_APP) && defined(ENABLE_GATEWAY)
    //do nothing
#endif 
    
        cmdpkg_info.status = IDLE;
    }

#if defined(SINK_APP) && defined(ENABLE_GATEWAY)    // 主重发
    if (cmdpkg_info.status == ERR && cmdpkg_info.retry <= GWTIMEOUT)
    {
        gwQueryCmd();
        cmdpkg_info.retry++;
        sea_printf("\ngwQeryCmd() retry again!");
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void LampPkgHanlder(void)
//* 功能        : 网关CO，是自己的，则从list[]里复制过来；若不是（转发的），写入该区域；
//*             : 网关ZR，将该区域通过zigbee发给自身所在PAN的协调器；
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void LampPkgHanlder ( void )
{
    if (lamppkg_info.status == BUSY)
    {    
#if defined(SINK_APP) && defined(ENABLE_GATEWAY)                      //ZC    
        printGwPkg();
        gwSendLampData();
    
#elif defined(SENSOR_APP) && defined(ENABLE_GATEWAY)                    //ZR
        sea_printf("\n->RX a lamp pkg,print then forward unicast");
        printGwPkg();
        forwardLampPkg();    //unicast to 0x0000
#endif
    
        lamppkg_info.status = IDLE;
 
        sea_memset(&lamppkg_info, 0x00, sizeof(lamp_pkg_info_t));
        sea_printf("\n->lamppkg_info reset");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void printGwPkg(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void printGwPkg  ( void )
{
#if 1   
    int i;

    sea_printf("\n------------cmdpkg_info------------");
    sea_printf("\ncmdpkg_info.status  = %02x", cmdpkg_info.status);
    sea_printf("\ncmdpkg_info.count   = %02x", cmdpkg_info.count);
    sea_printf("\ncmdpkg_info.cmd_pkg:\n");
    for(i = 0x00; i < CMDNUM + 0x01; i ++)
        sea_printf("%02x ",*( (u8 *)(&cmdpkg_info.cmd_pkg) + i) );
  
    sea_printf("\n\n------------lamppkg_info-----------");
    sea_printf("\nlamppkg_info.status = %02x", lamppkg_info.status);
    sea_printf("\nlamppkg_info.count  = %02x", lamppkg_info.count);
    sea_printf("\nlamppkg_info.lamp_pkg:\n");
    for (i = 0x00; i < PKGSIZE; i ++)
        sea_printf("%02x ", lamppkg_info.lamp_pkg[i]);
  
    // sea_printf("\nPA3(SPI SSEL):GPIO_PAOUT is %x", GPIO_PAOUT_REG);
    // sea_printf("\nPA3: GPIO_PAIN is %x", GPIO_PAIN);
#ifdef  GateWay_ZR
    sea_printf("\nchoice: %d", choice);
#endif
    sea_printf("\n------------end----------------------");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//网关协调器部分
/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(SINK_APP) && defined(ENABLE_GATEWAY)
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void GateWay_CO_Init(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void GateWay_CO_Init(void)
{
  //初始化
  lamppkg_info.status = IDLE;
  lamppkg_info.count = 0;                               //lamppkg_info.count是spi传输计数时用；
  sea_memset(lamppkg_info.lamp_pkg,0x07,PKGSIZE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gwSendLampData(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 当lamppkg_info.status == BUSY 时，调用本函数
//*------------------------------------------------*/
void gwSendLampData( void )
{  
  spi_start();
  sea_printf("\n->gwSendLampData()");
  
  spi_writebyte(DATA);
  while(lamppkg_info.count < PKGSIZE)
  {
    spi_writebyte(lamppkg_info.lamp_pkg[lamppkg_info.count++]);
  }
  //delay(0xff);
  spi_end();
  lamppkg_info.count = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCmdPkgSet(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCmdPkgSet ( void )
{
    for (u8 i = 0x00; i < sizeof(cmd_format_t); i ++)
    {
        if (*( (u8 *)(&cmdpkg_info.cmd_pkg) + i) != 0x00)
            return (bool)TRUE;
    }
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gwQueryCmd(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 定时查询ZigBee_ZR有无命令
//*------------------------------------------------*/
void gwQueryCmd ( void )
{
    u8 temp;
  
    sea_printf("\ngwCmd :");
    cmdpkg_info.count = 0x00;
    
    spi_start();
    spi_writebyte(QUARY);
    while (cmdpkg_info.count < (sizeof(cmd_format_t)))
    {
        temp = spi_readwrite (FILL);
        if (temp != 0xff)
        {
            *((u8 *)(&cmdpkg_info.cmd_pkg) + cmdpkg_info.count ++ ) = temp;
            sea_printf(" %02x", temp);
        }
    }
    spi_end();
  
    if (isCmdPkgSet())
    {
        cmdpkg_info.status = BUSY;
        sea_printf("  :query a busy cmdpkg,parse and forward...");
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void fillLampPkg(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
void fillLampPkg( void )
{
  sea_printf("->fill LampPkg");
  if (lamppkg_info.status == IDLE)
  {
    extern road_t  road_info; 
    lamppkg_info.lamp_pkg[PREFIX_INDEX] = 0x68;
    lamppkg_info.lamp_pkg[ROADID_INDEX] = ROADID;
    lamppkg_info.lamp_pkg[SUFFIX_INDEX] = 0x43;
    
    //sea_memcpy((u8 *)(lamppkg_info.lamp_pkg)+2,(u8 *)(road_info.head),CURRENTLAMPS * sizeof(lamp_t));
    lamppkg_info.status = BUSY;
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////
//网关路由部分
/////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SENSOR_APP) && defined(ENABLE_GATEWAY) 
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void GateWay_RT_Init(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void GateWay_RT_Init(void)
{
  cmdpkg_info.status = IDLE;
  cmdpkg_info.count  = 0;                                   //cmdpkg_info.count是spi传输计数时用；  
  cmdpkg_info.retry  = 0;   
  //sea_memset(&cmdpkg_info.cmd_pkg,0x08,sizeof(cmd_format_t));
}

static u8 choice = 0;
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gwPollForLampData(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void gwPollForLampData ( void )
{
    u8 temp;
  
    while (spi_isnotempty())
    {
        switch (choice)
        {
            case 0: //选择
                temp = spi_readbyte();
                if (temp == QUARY)
                    choice = 1;
                else if (temp == DATA)
                    choice = 2;
                else
                {                                                           //SPI传输速度差异导致，一般多出三个，丢掉即可
//                  sea_printf("\n[rx] an unknown feature.choice = 0");
//                  sea_printf("\n temp = %d",temp);         // do nothing temporary
                }
                break;
            case 1: // 发送 cmd
                spi_readwrite(*((u8 *)(&cmdpkg_info.cmd_pkg) + cmdpkg_info.count ++ ));
                if (cmdpkg_info.count == sizeof(cmd_format_t) + 0x01)
                {
                    cmdpkg_info.status = IDLE;                                    //发送完之后，清空。
                    sea_memset(&cmdpkg_info.cmd_pkg, 0x05, sizeof(cmd_format_t));
                    cmdpkg_info.cmd_pkg.all_status  = 0x00;
                    cmdpkg_info.cmd_pkg.all_persent = 0x00;
                    cmdpkg_info.count = 0;
                    choice = 0;
                    sea_printf("\nfinish [QUARY] cmdpkg pass back..reset choice flag");
                }
                break;
            case 2:  // 接收lamp data
                temp = spi_readbyte();
                if (temp != 0xFF)
                    lamppkg_info.lamp_pkg[lamppkg_info.count++] = temp;
                if (lamppkg_info.count == 0x82)
                {
                    lamppkg_info.status = BUSY;
                    choice = 0;
                    sea_printf("\nfinish [DATA] Rx..reset choice flag");
                    //printGwPkg();
                }
                break;
            default:
                sea_printf("\nerror...choice is :%d",choice);
                break;
        } // switch
    }  // while
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void forwardLampPkg(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 网管ZR把网管ZC的信息发送给自己网内的ZC
//*------------------------------------------------*/
static void forwardLampPkg( void )
{
  if (lamppkg_info.status == BUSY)
  {
    sea_printf("\n->forward LampPkg to ZC");
    //unicasts to ZC
    
    lamppkg_info.status = IDLE;
    lamppkg_info.count  = 0;
    sea_memset(lamppkg_info.lamp_pkg,0x00,PKGSIZE);
          
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif





/////////////////////////////////////////////////////////////////////////////////////////////
//暂时未用，备份
#if 0
void gwPollForLampData()
{
#if 0  
  if (( (SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 1))
  {
      temp = spi_readbyte();
      switch (temp)
      {
      case QUARY://发送cmd
        sea_printf("\nrx a QUARY Feature");
        while( (SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 1)
        {
          spi_readwrite(*((u8 *)(&cmdpkg_info.cmd_pkg) + cmdpkg_info.count++ ));
          //if ((SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 0)
            //delay(0x07);
        }
        cmdpkg_info.status = IDLE;
        break;
      
      case DATA://接收lamp data
        sea_printf("\nrx a DATA Feature");
        while( (SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 1)
        {
          temp = spi_readbyte();
          if(temp != 0xFF && temp!= DATA)
            lamppkg_info.lamp_pkg[lamppkg_info.count++] = temp; 
          //if ((SC2_SPISTAT & SC_SPIRXVAL_MASK) >> SC_SPIRXVAL_BIT == 0)
            //delay(0x07);
        }
        lamppkg_info.status = BUSY;
        break;
      
      default:
        sea_printf("\n rx an unknown spi cmd");
      }
      sea_printf("\ncall poll");
  }
#endif
  
//SwapData
#if 0     
    if (cmdpkg_info.count < sizeof(cmd_format_t))
    {
      temp = spi_readwrite(*( (u8 *)(&cmdpkg_info.cmd_pkg) + cmdpkg_info.count++ ));
      lamppkg_info.lamp_pkg[lamppkg_info.count++] = temp;
    }
    else
    {
      temp = spi_readbyte();
      lamppkg_info.lamp_pkg[lamppkg_info.count++] = temp;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void gwSwapData(void)
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 当lamppkg_info.status == BUSY 时，调用本函数
//*------------------------------------------------*/
void gwSwapData(void)
{
  u8 temp;
    
  while(lamppkg_info.count < PKGSIZE)
  {
    //spi_writebyte(lamppkg_info.lamp_pkg[lamppkg_info.count++]);
    temp = spi_readwrite (lamppkg_info.lamp_pkg[lamppkg_info.count++]);
    if ((cmdpkg_info.count < sizeof(cmd_format_t)) && temp != 0xff)
      *( (u8 *)(&cmdpkg_info.cmd_pkg) + cmdpkg_info.count++ ) = temp;    
  }
  
  //delay(0xff);
  spi_end();
  
  if ( cmdpkg_info.status == IDLE )
     cmdpkg_info.status = BUSY;
  
  //任务完成，清零
  sea_memset(&lamppkg_info,0, sizeof(lamp_pkg_info_t));
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_gwsendrecvframe ( void )
//* 功能        : send and receive a char by spi
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : backup the function, commented by renyu 2011/10/18
//*------------------------------------------------*/
static void sea_gwsendrecvframe ( void )
{
#if 0   // test version
    u8 temp;
        
    if (sframe.state)
    {
        if (sframe.cnt == GWMAXCNT)
        {
            temp = spi_readwrite(PREFIX);
            sframe.cnt = 0x00;
        }
        else if (sframe.cnt == 0x00)
        {
            temp = spi_readwrite(sframe.cmd);
            sframe.cnt ++;
        }
        else if (sframe.cnt == 0x01)
        {
            temp = spi_readwrite(sframe.len);
            sframe.cnt ++;
        }
        else
        {
            temp = spi_readwrite(sframe.body[sframe.cnt - 0x02]);
            if (sframe.cnt ++ > sframe.len)
                sea_gwreleaseframe(&sframe);
        }
    }
    else
        temp = spi_readwrite(SPIEMPTY);
    if (temp != SPIEMPTY && rframe.state == FALSE)
    { 
        if (temp == PREFIX && rframe.cnt == GWMAXCNT)
            rframe.cnt = 0x00;
        else if (rframe.cnt == 0x00)
        {
            rframe.cmd = temp;
            rframe.cnt ++;
        }
        else if (rframe.cnt == 0x01)
        {
            rframe.len = temp > DATASIZE ? (DATASIZE) : (temp);
            rframe.cnt ++;
        }
        else
        {
            rframe.body[rframe.cnt - 0x02] = temp;
            if (rframe.cnt ++ > rframe.len)
            {
                sea_gwverifyrecvframe(&rframe);
                rframe.state = TRUE;
            }
        }               
    }
#else
    u8 temp;
    
    if (rfrm == NULL)
        rfrm = sea_getcurrentframe(RECVLIST);
    if (sfrm == NULL)
        sfrm = sea_getframelist(SENDLIST);
    
    if (sfrm)
    {
        if (sfrm->cnt == GWMAXCNT)
        {
            temp = spi_readwrite(PREFIX);
            sfrm->cnt = 0x00;
        }
        else if (sfrm->cnt == 0x00)
        {
            temp = spi_readwrite(sfrm->cmd);
            sfrm->cnt ++;
        }
        else if (sfrm->cnt == 0x01)
        {
            temp = spi_readwrite(sfrm->len);
            sfrm->cnt ++;
        }
        else
        {
            temp = spi_readwrite(sfrm->body[sfrm->cnt - 0x02]);
            if (sfrm->cnt ++ > sfrm->len)
            {
                sea_gwreleaseframe(sfrm);
                sfrm = NULL;
            }
        }
    }
    else
        temp = spi_readwrite(SPIEMPTY);
    
    if (temp != SPIEMPTY && rfrm->state == FALSE)
    { 
        if (temp == PREFIX && rfrm->cnt == GWMAXCNT)
            rfrm->cnt = 0x00;
        else if (rfrm->cnt == 0x00)
        {
            rfrm->cmd = temp;
            rfrm->cnt ++;
        }
        else if (rfrm->cnt == 0x01)
        {
            rfrm->len = temp > DATASIZE ? (DATASIZE) : (temp);
            rfrm->cnt ++;
        }
        else
        {
            rfrm->body[rfrm->cnt - 0x02] = temp;
            if (rfrm->cnt ++ > rfrm->len)
            {
                sea_gwverifyrecvframe(rfrm);
                rfrm->state = TRUE;
                sea_putframelist(RECVLIST, rfrm); 
                rfrm        = NULL;
            }
        }               
    }
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
#endif   // removed by renyu 2011/09/23
