/*
 * car.c
 * implement the obtain car function
 *
 */

#include "network.h"
#include "vehicle.h"
#include "serial.h"

#define SNEDTIMES     (0x04)
#define getime(m, s)  (m * 60 + s)

////////////////////////////////////////////////////////////////////////////////////
//
car_info_t carInfo = { 0x00, 0x00, 0x00, };
call_info_t call = {0};
#ifdef SENSOR_APP
static rs485_cmd_t rs485_cmd;
static vehicle_t   plc;
static u8  card_state[((MAXLAMPS >> 0x03) + 0x01)];        // for card state bit maps
    
////////////////////////////////////////////////////////////////////////////////////
//
#ifndef VEHICLE_RELEASE   
static frame_t sample;
#endif
static turn_t  st_turn;

///////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void rs485Init ( void )
//* 功能        : rs485 initialize
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void rs485Init ( void )
{
    halGpioConfig(GPIO_485_TX, GPIOCFG_OUT);         // ?PA2??????
    halClearGPIO(GPIO_485_TX);
}
         

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void delay ( void )
//* 功能        : 
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 
//*------------------------------------------------*/
static void rs485delay ( void )
{
    for (vu16 i = 0x00; i < 0x0fff; i ++) 
        i = i;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void rs485sendbyte ( u8 ch )
//* 功能        : send byte to the car
//* 输入参数    : u8 ch
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void rs485sendbyte ( u8 ch )
{
#ifdef  VEHICLE_RELEASE
    u8 retry = 0x05;
    
    halSetGPIO(GPIO_485_TX);
    rs485delay(); 
    while (!sea_uartsendbyte(ch) && retry --)  ;
    halClearGPIO(GPIO_485_TX);    
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void rs485sendstring ( prs485_t ptr )
//* 功能        : send command to the car
//* 输入参数    : prs485_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void rs485sendstring ( prs485_t ptr )
{
    if (isCallDevice())
        return;
    
    rs485sendbyte(ptr->prefix);
    rs485sendbyte(ptr->cmd);
    for (u8 i = 0x00; i < CARPLCCMDLEN - 1; i ++)
    {
        rs485sendbyte(ptr->buf[i]);        
    }
    
    DBG("\r\n%2x %2x", ptr->prefix, ptr->cmd);    
    for (u8 i = 0x00; i < CARPLCCMDLEN - 1; i ++)
    {        
        DBG(" %2x", ptr->buf[i]);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_putrs485 ( u8 cmd, u16 left, u16 right )
//* 功能        : put command to list
//* 输入参数    : u8 cmd, u16 left, u16 right
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_putrs485 ( u8 cmd, u16 left, u16 right )
{
    rs485_cmd.list[rs485_cmd.in].prefix = CAR_CMD_PREFIX;
    rs485_cmd.list[rs485_cmd.in].cmd    = cmd;
    sea_memcpy(rs485_cmd.list[rs485_cmd.in].buf, &left, 0x02);
    sea_memcpy(rs485_cmd.list[rs485_cmd.in].buf + 0x02, &right, 0x02);
    rs485_cmd.list[rs485_cmd.in].buf[4] = carInfo.plc.number;
    rs485_cmd.list[rs485_cmd.in].buf[5] = call.type;
    rs485_cmd.list[rs485_cmd.in].buf[6] = call.station;
      
    rs485_cmd.in ++;
    if (rs485_cmd.in >= RS485CMDSIZE)
        rs485_cmd.in = 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static prs485_t sea_getrs485 ( void )
//* 功能        : get command from list
//* 输入参数    : 无
//* 输出参数    : pointer of command
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static prs485_t sea_getrs485 ( void )
{
    prs485_t ptr = NULL;
    
    if (rs485_cmd.out != rs485_cmd.in)
    {
        ptr = &rs485_cmd.list[rs485_cmd.out];
        rs485_cmd.out ++;
        if (rs485_cmd.out >= RS485CMDSIZE)
            rs485_cmd.out = 0x00;
    }
    
    return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void ledOn ( void )
//* 功能        : led on
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void ledOn ( void )
{
    DBG("\r\n[car]led on");
    halClearGPIO(GPIO_LED);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void ledOff ( void )
//* 功能        : led off
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void ledOff ( void )
{
    DBG("\r\n[car]led off");
    halSetGPIO(GPIO_LED);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void ledToggle ( void )
//* 功能        : led toggle
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void ledToggle ( void )
{
    DBG("\r\n[car]led toggle");
    if (GPIO_PAOUT & PA1)
        halClearGPIO(GPIO_LED);
    else
        halSetGPIO(GPIO_LED);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_startcar ( void * ptr )
//* 功能        : start run the car
//* 输入参数    : void * ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_startcar ( void * ptr )
{
    if (carInfo.waiting)  return;
    
    DBG("\r\n[car]car running");
    for (u8 i = 0x00; i < SNEDTIMES; i ++)
        carInfo.put(CAR_RUN, 0x00, 0x00);
    carInfo.status = CAR_RUN;
    set_lampmode(LAMP_UPDATE);
    if (carInfo.plc.fail)
        carInfo.plc.fail = NO_ERROR;
#ifndef VEHICLE_RELEASE 
    if (carInfo.plc.status != carInfo.status)
        carInfo.plc.status = carInfo.status;
    sample.body[0x00] = carInfo.status;
    recvPlcStateFrm(&sample);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_stopcar ( void * ptr )
//* 功能        : stop the car
//* 输入参数    : void * ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_stopcar ( void * ptr )
{
    DBG("\r\n[car]car stop");
    for (u8 i = 0x00; i < SNEDTIMES; i ++)
        carInfo.put(CAR_STOP, 0x00, 0x00);
    carInfo.status = CAR_STOP;
    set_lampmode(LAMP_UPDATE);
#ifndef VEHICLE_RELEASE 
    if (carInfo.plc.status != carInfo.status)
        carInfo.plc.status = carInfo.status;
    sample.body[0x00] = carInfo.status;
    recvPlcStateFrm(&sample);
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void sea_turncar ( void * ptr )
//* 功能        : turn car to left or right
//* 输入参数    : void * ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void sea_turncar ( void * ptr )
{
    pturn_t tmp = (pturn_t)ptr;
    u8  sendtimes = 1;
    
    if (ptr == NULL)
        return;
    
    if (tmp->cmd == CAR_LEFT) 
        DBG("\r\n[car]car turn left");
    else if (tmp->cmd == CAR_RIGHT) 
        DBG("\r\n[car]car turn right");
    else if(tmp->cmd == CAR_STRAIGHT)
        DBG("\r\n[car]car go straight");

    if(tmp->cmd != CAR_REQ_MSG)
    {
        carInfo.status = tmp->cmd; 
        sendtimes = SNEDTIMES;
    }
    for (u8 i = 0x00; i < sendtimes; i ++)
        carInfo.put(tmp->cmd, tmp->left, tmp->right);
    set_lampmode(LAMP_UPDATE);
#ifndef VEHICLE_RELEASE 
    if (carInfo.plc.status != carInfo.status)
        carInfo.plc.status = carInfo.status;
    sample.body[0x00] = carInfo.status;
#endif    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static pmagic_t sea_carmagic ( u16 magic, pmagic_t ptr )
//* 功能        : car direction on the road
//* 输入参数    : u16 magic, pmagic_t ptr
//* 输出参数    : direction, center and count
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static pmagic_t sea_carmagic ( u16 magic, pmagic_t ptr )
{
    u8  i, lcnt, rcnt;
    
    for (rcnt = lcnt = i = 0x00; i < 0x08; i ++, magic >>= 0x01)
    {
        if (magic & 0x0001)
            lcnt ++;
        if (magic & 0x0100)
            rcnt ++;
    }
    ptr->dir = lcnt - rcnt;
    ptr->cnt = lcnt + rcnt;
    return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleInit ( void )
//* 功能        : initialize car information, configure the car table
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleInit ( void ) 
{
    sea_memset(&carInfo, 0x00, sizeof(car_info_t));
    sea_memset(&rs485_cmd, 0x00, sizeof(rs485_cmd_t));
    sea_memset(&single_info.lamp, 0x00, sizeof(lamp_t));
    sea_memset(&plc, 0x00, sizeof(vehicle_t));
    sea_memset(card_state, 0x00, ((MAXLAMPS >> 0x03) + 0x01));

    single_info.lamp.vehicle.number = sys_info.dev.num;
    single_info.lamp.vehicle.type   = sys_info.dev.type;
    single_info.on      = sea_startcar;
    single_info.off     = sea_stopcar;
    single_info.toggle  = sea_turncar;

    carInfo.plc.number  = sys_info.dev.num;
    carInfo.plc.type    = sys_info.dev.type;
    carInfo.plc.status  = CAR_STOP;
    carInfo.delay       = DEFMSTICK;
    carInfo.state       = card_state;
    carInfo.put         = sea_putrs485;
    carInfo.get         = sea_getrs485;
    carInfo.on          = ledOn;
    carInfo.off         = ledOff;
    carInfo.toggle      = ledToggle;
    carInfo.magic       = sea_carmagic;
    carInfo.turn        = &st_turn;
    
    plc.number          = sys_info.dev.num;
    plc.type            = sys_info.dev.type;
    
    call.station = 123;
    call.type   = 111;    
        
    rs485Init();
    sea_configprint();

#ifndef VEHICLE_RELEASE
    carInfo.plc.magic   = 0x07e0; 
    sea_memset(&sample, 0x00, sizeof(frame_t));
    pplc_t ptr  = (pplc_t)sample.body;
    sample.len  = sizeof(plc_t);
    sample.cmd  = CAR_REQ_MSG;
    sample.road = single_info.address;
    ptr->status = carInfo.plc.status;
    ptr->card   = carInfo.card;
    ptr->magic  = carInfo.plc.magic;
#endif
    
    if (isCallDevice())
    {
        halGpioConfig(GPIO_LED, GPIOCFG_OUT);         // 将PA1设为推挽输出
        halSetGPIO(GPIO_LED);
        halGpioConfig(GPIO_BUTTONA, GPIOCFG_IN_PUD);
        halGpioConfig(GPIO_BUTTONB, GPIOCFG_IN_PUD);
        halSetGPIO(GPIO_BUTTONA);                     // pull up
        halSetGPIO(GPIO_BUTTONB);                     // pull up
        carInfo.beep = (pbeep_t)&single_info.lamp.vehicle;
        carInfo.beep->number  = sys_info.dev.num;
        carInfo.beep->type    = sys_info.dev.type;
        carInfo.beep->assign  = 0x00;
        carInfo.beep->call    = 0x00;
        carInfo.beep->status  = 0x00;
        carInfo.beep->car     = 0x00;
        carInfo.beep->fail    = 0x00;
        carInfo.beep->last    = 0x00;
        carInfo.beep->tick    = 0x00;
        carInfo.beep->times   = 9800;
    }
    else if (isCarDevice())
    {
        u8 i;
        sea_memset(carInfo.home, 0x00, sizeof(action_t) * HOMESIZE);
        sea_flashread(TABLEOFFSET, sizeof(action_t) * HOMESIZE, carInfo.home);
        sea_printf("\ngo home route table.");
        for (i = 0x00; i < HOMESIZE; i ++)
        {
            if (carInfo.home[i].id == 0x00 || carInfo.home[i].action == 0x00)
                break;
            if (carInfo.home[i].id == 0xff || carInfo.home[i].action == 0xff)
                break;
            sea_printf("\nid %02x, action 02x", carInfo.home[i].id, carInfo.home[i].action);
        }
        carInfo.homeway = i;
        for (i = carInfo.homeway; i < HOMESIZE; i ++)
        {
            carInfo.home[i].id     = 0x00;
            carInfo.home[i].action = 0x00;
        }
        vehicleRouteTable(0x00, carInfo.homeway, carInfo.home);
        carInfo.beep = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCallDevice ( void )
//* 功能        : is caller device
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCallDevice ( void )
{
    if (sys_info.dev.type >= CALLIDST && sys_info.dev.type < CALLIDST + CALLIDCNT)
        return (bool)TRUE;
    return (bool)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : bool isCarDevice ( void )
//* 功能        : is car device
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
bool isCarDevice ( void )
{
    if (sys_info.dev.type >= CARIDST && sys_info.dev.type < CARIDST + CARIDCNT)
        return (bool)TRUE;
    return (bool)FALSE;
}

#if 0
/** 
 * @param: void
 * @return: void
 */
/*void showCarSate ( void )
{
    DBG("[car state] "); 
    char * ptr="";
    switch (carInfo.plc.status) 
    {
        case CARSTOP:  //car stop        
            ptr = "stop";
            break;
        case CARGOLEFT: //car go left                                
            ptr = "turning left";
            break;
        case CARGORIGHT: //car go right       
            ptr = "turning right";
            break;
        case CARGOSTRAIGHT: //car go straight        
            ptr = "heading ahead";
            break;
        case CARRUN:        //car run
            ptr = "start of run";
            break; 
    }
    DBG("movement: %s, ", ptr);
    DBG("obstacle: %d, ", carInfo.plc.obstacle); 
    DBG("cardID: 0x%04x, ", carInfo.plc.card);
    DBG("speed: left %d, right %d\r\n", carInfo.plc.left, carInfo.plc.right);
}
*/
// carSetState
// accrding to the car state to choose the direction of the car
// @param carStat the car state
// @param leftWheelRate the left wheel rate
// @param rightWheelRate the right wheel rate
/*void carSetState ( u8 carStat, u16 leftWheelRate, u16 rightWheelRate ) 
{
    carInfo.buffer[0] = CAR_CMD_PREFIX;
    switch (carStat) 
    {
        case CARSTOP:  //car stop
            carInfo.buffer[1] = CAR_STOP;
            break;
        case CARGOLEFT: //car go left
            carInfo.buffer[1] = GO_LEFT;
            sea_memcpy(carInfo.buffer + 0x02, &leftWheelRate, 0x02);
            sea_memcpy(carInfo.buffer + 0x04, &rightWheelRate, 0x02);
            break;
        case CARGORIGHT: //car go right
            carInfo.buffer[1] = GO_RIGHT;
            sea_memcpy(carInfo.buffer + 0x02, &leftWheelRate, 0x02);
            sea_memcpy(carInfo.buffer + 0x04, &rightWheelRate, 0x02);
            break;
        case CARGOSTRAIGHT: //car go straight
            carInfo.buffer[1] = GO_STRAIGHT;
            break;
        case CARRUN:        //car run
            carInfo.buffer[1] = CAR_RUN;
            break;
        default:
            return; 
    }
    carInfo.status = carStat;
}*/

// carRouteEventHandler
// according to the router table,
// the car run
// @param void
/*void carRouteEventHandler ( void ) 
{
    if ((carInfo.status != CARFREE) && (carInfo.status != CAR_POS_ERROR) && 
        (carInfo.plc.status > 0x00) && (carInfo.plc.status != CARSTOP)) 
    {
        carInfo.status = carInfo.plc.status;
    }
    
    if (carInfo.status != CARFREE)   
    { 
        if ((carInfo.route.line[carInfo.route.index].id) == carInfo.plc.card) 
        {
            if (carInfo.route.line[carInfo.route.index].action == CARGOLEFT) 
            {
                carInfo.put(CARGOLEFT, 3500, 3750);          // carSetState(CARGOLEFT, 3500, 3750);
            } 
            else if (carInfo.route.line[carInfo.route.index].action == CARGORIGHT) 
            {
                carInfo.put(CARGORIGHT, 3750, 3500);         // carSetState(CARGORIGHT, 3750, 3500);
            } 
            else if (carInfo.route.line[carInfo.route.index].action == CARSTOP) 
            {
                carInfo.put(CARSTOP, 0x00, 0x00);            // carSetState(CARSTOP, 0x00, 0x00);
            } 
            else 
            {
                carInfo.put(carInfo.route.line[carInfo.route.index].action, 0x00, 0x00);   // carSetState(carInfo.route.line[carInfo.route.index].action, 0x00, 0x00);
            }
            carInfo.status = carInfo.route.line[carInfo.route.index].action;
    
            carInfo.route.index ++;
            if (carInfo.route.index == carInfo.route.count) 
            {
                carInfo.status = CARFREE;
                carInfo.route.index  = 0x00;
                carInfo.route.count  = 0x00;
            }
        } 
        else     // while error,car stops
        {             
            carInfo.put(CARSTOP, 0x00, 0x00);    // carSetState(CARSTOP, 0x00, 0x00);
            carInfo.status = CAR_POS_ERROR;       // 向PC报告自己的位置，以得到进一步的规划
            carInfo.route.index  = 0x00;
            carInfo.route.count = 0x00;
        }
        carInfo.echo = TRUE;
    }
    else
    {
        if ((carInfo.status == CARRUN) && (carInfo.plc.status != CAR_OBSTACLE_PAUSE))
        {
            if (carInfo.plc.status == CARSTOP)
                carInfo.put(CARRUN, 0x00, 0x00);   // carSetState(CARRUN, 0x00, 0x00);
        }         
    }
}*/
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u8 carFindCardInTable ( u8 card )
//* 功能        : find card in the router table
//* 输入参数    : 无
//* 输出参数    : index
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u8 carFindCardInTable ( u8 card )
{
    for (u8 i = 0x00; i < carInfo.route.count; i ++)    
    {
        if (carInfo.route.line[i].id == card)
            return i;
    }
    return 0xff;
}

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u8 carFindActionInTable ( u8 card )
//* 功能        : find card action in the router table
//* 输入参数    : 无
//* 输出参数    : action
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
/*static u8 carFindActionInTable ( u8 card )
{
    for (u8 i = 0x00; i < carInfo.route.count; i ++)    
    {
        if (carInfo.route.line[i].id == card)
            return carInfo.route.line[i].action;
    }
    return 0x00;
}*/
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void carReportHandler ( void )
//* 功能        : car report handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void carReportHandler ( void )
{
    if (network_info.appstate == EMBER_JOINED_NETWORK)
    {
        if (isCallDevice())
        {
            if (get_lampmode(LAMP_FORCE))
            {
                DBG("\r\n[call]send call, carInfo.beep->caller's vehicle type %d", carInfo.beep->call);
                sea_sendmsg(&send1, UNICAST, 0x00, ICHP_SV_RPT, sizeof(vehicle_t), carInfo.beep); 
                clr_lampmode(LAMP_FORCE);
            }
        }
        else
        {
            if (get_lampmode(LAMP_CHANGE))
            {
                if (sea_memcomp(&single_info.lamp, &carInfo.plc, sizeof(vehicle_t)))
                {
                    sea_memcpy(&single_info.lamp, &carInfo.plc, sizeof(vehicle_t));
                    DBG("\r\n[plc]send report");
                    DBG("\r\ncard %d step %d count %d", carInfo.plc.card, carInfo.plc.index, carInfo.plc.count);
                    sea_sendmsg(&send1, UNICAST, 0x00, ICHP_SV_RPT, sizeof(lamp_t), &single_info.lamp); 
                }
                else if ((single_info.time.sec % sys_info.ctrl.period) == 0x00 && single_info.sec != single_info.time.sec)
                {
                    single_info.sec = single_info.time.sec;   
                    DBG("\r\n[plc]send report 1 second once");
                    sea_sendmsg(&send1, UNICAST, 0x00, ICHP_SV_RPT, sizeof(lamp_t), &single_info.lamp); 
                }
                clr_lampmode(LAMP_CHANGE);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void carClearRouteTable ( void ) 
//* 功能        : clear the vehicle route table
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void carClearRouteTable ( void ) 
{
    DBG("\r\n[car]clear this vehicle route table");
    sea_memset(&carInfo.route, 0x00, sizeof(route_t));
    carInfo.action    = 0x00;
    plc.fail          = NO_ERROR;
    carInfo.plc.fail  = NO_ERROR;
    single_info.lamp.vehicle.fail = NO_ERROR;
//    carInfo.route.count = 0x00;
    carInfo.plc.count = carInfo.route.count;
    
    set_lampmode(LAMP_CHANGE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void carRouteEventHandler ( void )
//* 功能        : according to the router table, the car run
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void carRouteEventHandler ( void ) 
{   
    u8 index;
    
    if (!get_lampmode(LAMP_UPDATE))
        return;
    
    if (isCallDevice())
    {
        clr_lampmode(LAMP_UPDATE);
        set_lampmode(LAMP_CHANGE);
        return;
    }
    
    if (carInfo.plc.card && carInfo.plc.card != carInfo.card)      // meet new card on the road
    {
        if (carInfo.time == getime(single_info.time.min, single_info.time.sec))   // in 1 second, not report. 
        {
            clr_lampmode(LAMP_UPDATE);
            return;
        }
        carInfo.card = carInfo.plc.card;
        carInfo.time = getime(single_info.time.min, single_info.time.sec);
        index = carFindCardInTable(carInfo.card);
#ifdef CLASH_ENABLE
        if (carInfo.card)
        {
            if (get_lampbitmap(carInfo.state, carInfo.card - 0x01) && carInfo.status != CAR_STOP)
            {
                DBG("\r\nmany vehicles on the same line, vehicle need waiting ...");  
                single_info.off(NULL);
                carInfo.plc.fail |= MEET_CLASH;
                carInfo.clash = 0x01;
            }
        }
#endif
        if (index > carInfo.route.count)     // 0x80, 
        {
            carInfo.plc.fail |= (carInfo.route.count ? WRONG_WAY : EMPTY_TABLE);
            DBG("\r\nnot found %02x card in route table, count %d", carInfo.card, carInfo.route.count);
        }
        else
        {
            //passed_t pass;
            if (index != carInfo.route.index)
            {
                carInfo.route.index = index;
                carInfo.plc.fail |= MISS_CARD;
            }
            //pass.act.action = carInfo.route.line[carInfo.route.index].action;
            //pass.act.id     = carInfo.route.line[carInfo.route.index].id;
            //pass.min        = single_info.time.min;
            //pass.sec        = single_info.time.sec;
            //sea_savepassed(&pass);
            carInfo.action = carInfo.route.line[carInfo.route.index].action;
            
            carInfo.turn->cmd   = carInfo.action;   
            carInfo.turn->left  = carInfo.turn->cmd == CAR_LEFT ? (LEFTSPEED) : (carInfo.turn->cmd == CAR_RIGHT ? (RIGHTSPEED) : (0x00));
            carInfo.turn->right = carInfo.turn->cmd == CAR_LEFT ? (RIGHTSPEED) : (carInfo.turn->cmd == CAR_RIGHT ? (LEFTSPEED) : (0x00));
#ifdef CLASH_ENABLE
            if (!carInfo.clash)
                single_info.toggle(carInfo.turn);
#else
            single_info.toggle(carInfo.turn);
#endif            
            if (carInfo.action == CAR_STOP)
                carInfo.waiting = sys_info.ctrl.period * 20;
            DBG("\r\nid %02x, action %02x, index %d", carInfo.route.line[carInfo.route.index].id, carInfo.turn->cmd, carInfo.route.index);
            carInfo.plc.index = carInfo.route.index;
            carInfo.route.index ++;
            if (carInfo.route.index >= carInfo.route.count && carInfo.route.count) 
            {
                single_info.off(NULL); 
                carClearRouteTable();
                carInfo.plc.fail |= END_LINE;
                sea_sendmsg(&send1, UNICAST, COORDID, ICHP_SV_END, sizeof(lamp_t), &single_info.lamp);
            }
    
        } 
        set_lampmode(LAMP_CHANGE);
    }
    else 
    {
        if (carInfo.action == CAR_STOP && carInfo.plc.status != CAR_STOP)
        {
            single_info.off(NULL);   
            carInfo.plc.fail = NO_ERROR;
            set_lampmode(LAMP_CHANGE);
        }
        else if (sea_memcomp(&single_info.lamp.vehicle, &carInfo.plc, sizeof(vehicle_t)))
            set_lampmode(LAMP_CHANGE);
#ifdef CALMAGIC_ENABLE        
        switch (carInfo.action)
        {
            case CAR_LEFT:
            case CAR_RIGHT:
                if (plc.magic > 0x60)
                {
                    carInfo.turn->cmd   = carInfo.action;   
                    carInfo.turn->left  = carInfo.turn->cmd == CAR_LEFT ? (LEFTSPEED) : (RIGHTSPEED);
                    carInfo.turn->right = carInfo.turn->cmd == CAR_LEFT ? (RIGHTSPEED) : (LEFTSPEED);
                    single_info.toggle(carInfo.turn);
                }
                break;
        }
#endif        
    }
    clr_lampmode(LAMP_UPDATE);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : u8  vehicleRouteTable ( u8 index, u8 length, action_t * ptr )
//* 功能        : the message max length is 31,the first is the car start or
//                if the router table length is more than 31,the first is the 0xFF,
//                meaning that there is more data
//* 输入参数    : @param length: the received message length
//                @param index: the table index
//                @param ptr: the message
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
u8  vehicleRouteTable ( u8 index, u8 length, action_t * ptr ) 
{
    u8  i, sum = 0x00;
    
    if (index + length >= MAXPATH)
        return sum;
    
    if (carInfo.status != CAR_STOP)
       single_info.off(NULL); 

    if (index == 0x00 && length == 0x00)
    {
        carClearRouteTable();
        return sum;
    }
    
    sea_printf("\n[car]route index %d, length %d", index, length);
    for (i = index; i < length + index; i ++, ptr ++)   // carInfo.route.index < thisLen; carInfo.route.index ++) 
    {
        carInfo.route.line[i].id     = ptr->id; 
        carInfo.route.line[i].action = ptr->action;
        sum += ptr->id + ptr->action;
        sea_printf("\nindex %d, card id %02x, action %02x", i, ptr->id, ptr->action);
    }
    
    if (carInfo.route.count < length + index)
        carInfo.route.count = index + length;
    carInfo.plc.count = carInfo.route.count;

    if (carInfo.route.count < HOMESIZE && index == 0x00)
    {
        u8 j;
        for (j = i = 0x00; i < carInfo.route.count; i ++)
        {
            if (carInfo.route.line[i].action == CAR_STOP)
                j ++;
        }
        if (j == 0x01)     // home way
        {
            carInfo.homeway = carInfo.route.count;
            if (sea_memcomp(carInfo.route.line, carInfo.home, carInfo.route.count * sizeof(action_t)))
            {
                sea_printf("\nupdate go home route table, count %d.", carInfo.route.count);
                sea_memset(carInfo.home, 0x00, sizeof(action_t) * HOMESIZE);
                for (i = 0x00; i < carInfo.route.count; i ++)
                {
                    carInfo.home[i].id     = carInfo.route.line[i].id;
                    carInfo.home[i].action = carInfo.route.line[i].action;
                    sea_printf(" %d, %d, ", carInfo.home[i].id, carInfo.home[i].action);
                }
                sea_flashwrite(TABLEOFFSET, carInfo.home, sizeof(action_t) * HOMESIZE);
            }
        }
    }
    
    set_lampmode(LAMP_CHANGE);
    return sum & 0xff;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleAppHandler ( void )
//* 功能        : real time application handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleAppHandler ( void ) 
{
    if (network_info.appstate == EMBER_JOINED_NETWORK)
    {
        if (!network_info.substate)
        {
            network_info.substate = TRUE;
            single_info.address   = emberGetNodeId();                 // get logic short address
            set_lampmode(LAMP_NETWORK);
        }
        carReportHandler();
    }       
    carRouteEventHandler();
    
    if (isCallDevice())
    {
        if (carInfo.beep->last != carInfo.beep->status)
        {
            carInfo.beep->last = carInfo.beep->status;
            if (carInfo.beep->status == ONLINECAR)
                carInfo.off();
            else if (carInfo.beep->status == ASSIGNCAR)
                carInfo.on();
        }
        if (carInfo.beep->status == WAITINGCAR)
        {
            carInfo.beep->tick ++;
            if(carInfo.beep->tick >= carInfo.beep->times)
            {
                carInfo.toggle();
                carInfo.beep->tick = 0x00;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static u8 carScanKey ( void )
//* 功能        : get caller command
//* 输入参数    : 无
//* 输出参数    : call car id
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static u8 carScanKey ( void )
{
    if (isCallDevice())
    {
        if ((GPIO_PAIN & (PA0)) != (PA0))
            return CARIDST + (sys_info.dev.type - CALLIDST) * 0x02;
        if ((GPIO_PAIN & PA4) != PA4)
            return CARIDST + (sys_info.dev.type - CALLIDST) * 0x02 + 0x01;
    }
    return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleMsEventHandler ( void ) 
//* 功能        : micro_second tciker handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleMsEventHandler ( void ) 
{
    static vu32 tick = 0x00;
    
    carInfo.ms ++;
    if (network_info.appstate == EMBER_JOINED_NETWORK)
    {
        tick ++;
        if (!sea_msgisempty(&send1) && (tick % 800) == 0x00)        
        {
            pmsg_t ptr = sea_getmsg(&send1);
            network_info.unicast(ptr->id, COORDID, ptr->size, ptr->body);
        }
    }
    
    if (isCallDevice())
    {
        if (carInfo.ms >= KEYTIMES)
        {
            u8 key;
            key = carScanKey();
            if (key == carInfo.key && key)
            {
                carInfo.beep->call   = key;
                carInfo.beep->assign = 0x01;
                DBG("\r\n[KEY]carInfo.beep>caller's vehicle type %d", carInfo.beep->call);
                set_lampmode(LAMP_FORCE);
                carInfo.key = 0x00;
            }    
            else
                carInfo.key = key;
            carInfo.ms = 0x00;
        }
    }
    else if (isCarDevice())
    {
        if (carInfo.ms >= carInfo.delay)
        {
            prs485_t ptr = carInfo.get();
               
            if (ptr)
            {   
                rs485sendstring(ptr);
                carInfo.ms = 0x00;
            }
            else
            {
               #ifdef VEHICLE_RELEASE
                carInfo.turn->cmd = CAR_REQ_MSG;                   
                single_info.toggle(carInfo.turn);
               #endif
            }
        }
#ifndef VEHICLE_RELEASE   
        if ((single_info.time.tick % PLCPERIOD) == 0x00)
        {
            pplc_t ptr = (pplc_t)sample.body;
            ptr->magic = halCommonGetRandom();
            recvPlcStateFrm(&sample);
        }
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void vehicleTimeEventHandler ( void ) 
//* 功能        : second tciker handler
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void vehicleTimeEventHandler ( void ) 
{
    if ((single_info.time.min % sys_info.ctrl.period) == 0x00 && single_info.min != single_info.time.min)   // 5 minute
    {
        single_info.min = single_info.time.min;
        if (network_info.appstate == EMBER_JOINED_NETWORK)
        {    
            if (isCarDevice())
            {
                if (!get_lampmode(LAMP_CHANGE))
                    set_lampmode(LAMP_CHANGE);
#ifndef VEHICLE_RELEASE
                if (carInfo.route.count && carInfo.status != CAR_STOP)
                {
                    pplc_t ptr = (pplc_t)sample.body;
                    ptr->status = carInfo.route.line[carInfo.route.index].action;
                    ptr->card   = carInfo.route.line[carInfo.route.index].id;
                }
#endif
            }
        }
    }
#ifdef CLASH_ENABLE
    else if (isCarDevice() && carInfo.card && carInfo.clash && carInfo.action != CAR_STOP)    // 1 second handler
    {
        if (!get_lampbitmap(carInfo.state, carInfo.card - 0x01) && carInfo.status == CAR_STOP)
        {
            DBG("\r\nthe line is free, vehicle restarting ...");  
            single_info.on(NULL);
            single_info.toggle(carInfo.turn);
            carInfo.plc.fail &= ~MEET_CLASH;
            carInfo.clash = 0x00;
        }
    }
#endif
    
    if (carInfo.action == CAR_STOP && carInfo.waiting)
        carInfo.waiting --;    
    else if (carInfo.waiting)
        carInfo.waiting = 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : static void frmcopystate ( u8 * body, pvehicle_t ptr )
//* 功能        : received frame from plc
//* 输入参数    : u8 * body, pvehicle_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
static void frmcopystate ( u8 * body, pvehicle_t ptr )
{
    ptr->status    = body[0];
    ptr->obstacle  = body[1];
    ptr->obstacle ? (ptr->fail |= MEET_OBSTACLE) : (ptr->fail &= ~MEET_OBSTACLE);
    ptr->card      = body[2];
    ptr->magic     = body[7] | ((u16)(body[8]) << 0x08);
    ptr->index     = carInfo.route.index;
    ptr->count     = carInfo.route.count;
#ifdef CARSTATEDIFF    
    switch (ptr->status)
    {
        case CAR_LEFT:
        case CARGOLEFT:
            ptr->status = CAR_LEFT;
            break;
        case CAR_RIGHT:
        case CARGORIGHT:
            ptr->status = CAR_RIGHT;
            break;
        case CAR_STOP:
        case CARSTOP:
            ptr->status = CAR_STOP;
            break;
        case CAR_RUN:
        case CARRUN:
            ptr->status = CAR_RUN;
            break;
        case CAR_STRAIGHT:
        case CARGOSTRAIGHT:
            ptr->status = CAR_STRAIGHT;
            break;
        default:
            ptr->status = CAR_RUN;
            break;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void recvPlcStateFrm ( pframe_t ptr )
//* 功能        : received frame from plc
//* 输入参数    : pframe_t ptr
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void recvPlcStateFrm ( pframe_t ptr )
{
    if (!isCarDevice() || ptr->len < 0x05 || !ptr->body[0x02] || !ptr->body[0x00])
        return;

    carInfo.recv ++;
    if (ptr->body[1] || plc.card != ptr->body[2] || plc.status != ptr->body[0])
    {
        frmcopystate(ptr->body, &plc);
        sea_memcpy(&carInfo.plc, &plc, sizeof(vehicle_t));
        if (plc.status == CAR_RUN)
        {
            single_info.on(NULL);   
            if (carInfo.action == CAR_STOP)
                carInfo.action = CAR_RUN;
        }
        set_lampmode(LAMP_UPDATE);
    }
    else
    {
        if (carInfo.recv >= RECVTIMES)      // one secend, 50ms plc send report once
        {
            frmcopystate(ptr->body, &plc);
            sea_memcpy(&carInfo.plc, &plc, sizeof(vehicle_t));
            set_lampmode(LAMP_CHANGE);
            carInfo.recv = 0x00;
        }
#ifdef CALMAGIC_ENABLE        
        else
        {
            sea_memcpy(carInfo.map, &carInfo.map[0x01], (MAGICSIZE - 0x01) * sizeof(magic_t));
            carInfo.magic((ptr->body[7] | ((u16)(ptr->body[8]) << 0x08)), &carInfo.map[MAGICSIZE - 0x01]);
            plc.magic = 0x00;
            plc.speed = 0x00;
            for (u8 i = 0x00; i < MAGICSIZE; i ++)
            {
                plc.magic += carInfo.map[i].cnt;
                plc.speed += carInfo.map[i].dir;
            }
        }
#endif        
    }
}

#endif //SENSOR_APP

/////////////////////////////////////////////////////////////////////////////////////

