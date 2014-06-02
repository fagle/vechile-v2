#include "arc.h"
#include "cmd.h"
#include "sht11.h"  // wkl
#include "remote-db.h"
#include "remote-func.h"
#include "remote-iic.h"

//时时上报信息
arc_cur_t    arc_cur;


/*#pragma pack(1)     //按一16字对齐,不加按32字对齐
typedef struct{
    u32 temper;
    u16 mode;
    u32 wind;
}newtest;
#pragma pack()
newtest test1;*/

//两个队列
arc_ctrl_t     ctrlcmd_NRT;       //非实时命令
u8 ctrlcmd_NRTnum; 

arc_ctrl_t     ctrlcmd_RT;        //实时命令
u8 ctrlcmd_RTnum;


arc_ctrl_t     lastexec_arc_ctrl;
arc_ctrl_t     coord_arc_ctrl;    //for ARC_CD
u32 ARC_sec = 0;

u8 air_status;
u8 air_detect_restart;

//atc:auto control模块
arc_ctrl_t     atc_ctrlcmd1;      //ctrlcmd for have people
arc_ctrl_t     atc_ctrlCmd2;      //ctrlcmd for no people
u16 atc_closetime;

//时间模块
   
sst_time_t sst_time;          //时间模块
u8 sst_state;   //初始化为不可用

u16 sst_lackreporttime;

//power模块
u16 pwr_opentime;//7:00
u16 pwr_lasttime;
u8 pwr_ctrlmode;  // 0 auto 1 network
u8 pwr_autostate; // 0 close 1 open 2 unknow
u8 pwr_netstate;
u8 pwr_state;

//temper mode模块 
u8 tm_modedata[12] = { 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 5, 5};
u8 tm_tempdata[12] = {20,20,20,26,26,26,26,26,26,26,20,20};
                     //1  2  3  4  5  6  7  8  9 10 11 12月 

u8 tm_tmchange;
u8 tm_ctrlmode;
u8 tm_autotemper;
u8 tm_automode;
u8 tm_nettemper;
u8 tm_netmode;
//全局变量（大）
u8 arc_state[26];

u8 readtag = 0; 
u32 nowtick1; //for start
u32 nowtick2; //for first data 0
u32 nowtick3; //for first data 1

u16 task_stackremain[6] = {256,256,256,256,256,0};

pir_store_t pir_store[5];
u8 pir_store_in;


void ARC_Configration(void)
{
    u8 i;
    //PIR init
    GPIO_InitTypeDef    GPIO_ARCStructure;  
  
    GPIO_ARCStructure.GPIO_Pin   = GPIO_Pin_11;               
    GPIO_ARCStructure.GPIO_Speed = GPIO_Speed_50MHz;		  
    GPIO_ARCStructure.GPIO_Mode  = GPIO_Mode_IPD;          
    GPIO_Init(GPIOC, &GPIO_ARCStructure);
    
    GPIO_ARCStructure.GPIO_Pin   = GPIO_Pin_12;               
    GPIO_ARCStructure.GPIO_Speed = GPIO_Speed_50MHz;		  
    GPIO_ARCStructure.GPIO_Mode  = GPIO_Mode_IPD;          
    GPIO_Init(GPIOC, &GPIO_ARCStructure); 
    
    GPIO_ARCStructure.GPIO_Pin   = GPIO_Pin_13;               
    GPIO_ARCStructure.GPIO_Speed = GPIO_Speed_50MHz;		  
    GPIO_ARCStructure.GPIO_Mode  = GPIO_Mode_IPD;          
    GPIO_Init(GPIOC, &GPIO_ARCStructure); 
  
    GPIO_ARCStructure.GPIO_Pin   = GPIO_Pin_8 ;
    GPIO_ARCStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_ARCStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(GPIOC, &GPIO_ARCStructure);

    //sst 初始化
    sst_state = 0;   //初始化为不可用
    pwr_opentime = dyn_info.atc.opentime;//7:00
    pwr_lasttime = dyn_info.atc.lasttime;
    sst_lackreporttime = 60;
    
    //pwr 初始化
    pwr_ctrlmode = 0;  // 0 auto 1 network 其他 unknow
    pwr_autostate = 255; // 0 close 1 open 其他 unknow
    pwr_netstate = 255;
    pwr_state = 1;
    GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);   //开为reset

    //tm 初始化
    tm_tmchange = 0;
    tm_ctrlmode = 0;
    tm_autotemper = 0xff;
    tm_automode = 0xff;
    tm_nettemper = 0xff;
    tm_netmode = 0xff;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
    for(i = 0;i < 12;i++)
        tm_modedata[i] = dyn_info.atc.tm_modedata[i];
    for(i = 0;i < 12;i++)
        tm_tempdata[i] = dyn_info.atc.tm_tempdata[i];
    
    arc_cur.airid = dyn_info.atc.id;  //以它作为空调号的基准
    arc_cur.temper = 127; //初始化为无效 
    arc_cur.lumin = 0xff;
    arc_cur.humi = 0xff;
    arc_cur.cur = 0;
    arc_cur.PIR1 = PIR_NONE;
    arc_cur.PIR2 = PIR_NONE;
    arc_cur.PIR3 = PIR_NONE;
    arc_cur.ctrlcmd1open = UNKNOW;
    arc_cur.mcur_count = 0;
    arc_cur.valid_mcur_count = 0;
    
    coord_arc_ctrl.airid = 610;
    coord_arc_ctrl.ctrlmode = ARC_CTRL_NETWORK;
    coord_arc_ctrl.open = UNKNOW;
    coord_arc_ctrl.mode = 5;
    coord_arc_ctrl.temper = 25;
    coord_arc_ctrl.windlevel = 1;
    
    ctrlcmd_NRT.airid = 610;           //排队的命令
    ctrlcmd_NRT.ctrlmode = ARC_CTRL_UNKNOW;
    ctrlcmd_NRT.open = UNKNOW;
    ctrlcmd_NRT.mode = 0;
    ctrlcmd_NRT.temper = 0;
    ctrlcmd_NRT.windlevel = 0;
    
    ctrlcmd_NRTnum = 0;        //in queue num
    
    
    ctrlcmd_RT.airid = 610;           //实时命令
    ctrlcmd_RT.ctrlmode = ARC_CTRL_UNKNOW;
    ctrlcmd_RT.open = UNKNOW;
    ctrlcmd_RT.mode = 0;
    ctrlcmd_RT.temper = 0;
    ctrlcmd_RT.windlevel = 0;
    
    ctrlcmd_RTnum = 0;
    
    
    atc_ctrlcmd1.airid = 610;          //记录自动控制命令 初始化为可用
    atc_ctrlcmd1.ctrlmode = ARC_CTRL_AUTO;
    atc_ctrlcmd1.open = KEEP_AIR;
    atc_ctrlcmd1.mode = dyn_info.atc.mode;
    atc_ctrlcmd1.temper = dyn_info.atc.temper;
    atc_ctrlcmd1.windlevel = 2;
    
    atc_closetime = dyn_info.atc.closetime;   //sec
    
    
    lastexec_arc_ctrl.airid = 610;      //最后执行的命令
    lastexec_arc_ctrl.ctrlmode = ARC_CTRL_UNKNOW;
    lastexec_arc_ctrl.open = UNKNOW;
    lastexec_arc_ctrl.mode = 0;
    lastexec_arc_ctrl.temper = 0;
    lastexec_arc_ctrl.windlevel = 0;
    
    air_status = AIR_STATUS_UNKNOW;
    air_detect_restart = 0;
    
    memset(&sst_time, 0x00, sizeof(sst_time_t));
    
    pir_store_in = 0;
    for(i = 0;i < 5;i ++)
    {
        pir_store[i].hour = 0;
        pir_store[i].min = 0;
        pir_store[i].sec = 0;
        pir_store[i].pir_stateall = 0;
    }
}






void tm_tick()
{
    static u8 last_tm_ctrlmode = 0xff;
    
    static u8 last_tm_automode = 0xff;
    static u8 last_tm_autotemper = 0xff;
    static u8 last_tm_netmode = 0xff;
    static u8 last_tm_nettemper = 0xff;
    
    if(last_tm_ctrlmode != tm_ctrlmode ||       //可以一直执行，也可以有改变再执行
       last_tm_automode != tm_automode ||
       last_tm_autotemper != tm_autotemper ||
       last_tm_netmode != tm_netmode ||
       last_tm_nettemper != tm_nettemper)
    {
        last_tm_ctrlmode = tm_ctrlmode;
        last_tm_automode = tm_automode;
        last_tm_autotemper = tm_autotemper;
        last_tm_netmode = tm_netmode;
        last_tm_nettemper = tm_nettemper;
       
       // 0 auto 1 net
        if(tm_ctrlmode == 0)
        {
            if(tm_automode >= 1 && tm_automode <= 5
               && tm_autotemper >= 16 && tm_autotemper <= 30)
            {
                atc_ctrlcmd1.mode = tm_automode;
                atc_ctrlcmd1.temper = tm_autotemper;
                sea_printf("\n[TM]new atc_ctrlcmd1,mode = %d,temper = %d",atc_ctrlcmd1.mode,atc_ctrlcmd1.temper);
            }
        }
        else if(tm_ctrlmode == 1)
        {
            if(tm_netmode >= 1 && tm_netmode <= 5
               && tm_nettemper >= 16 && tm_nettemper <= 30)
            {
                atc_ctrlcmd1.mode = tm_netmode;
                atc_ctrlcmd1.temper = tm_nettemper;
                sea_printf("\n[TM]new atc_ctrlcmd1,mode = %d,temper = %d",atc_ctrlcmd1.mode,atc_ctrlcmd1.temper);
            }
        }
        else
        {
            sea_printf("\n[TM]tm_ctrlmode error");
        }
        
        if(dyn_info.atc.mode != atc_ctrlcmd1.mode || dyn_info.atc.temper != atc_ctrlcmd1.temper)
        {
            dyn_info.atc.mode = atc_ctrlcmd1.mode;
            dyn_info.atc.temper = atc_ctrlcmd1.temper;
            sea_printf("\n[FLASH]update atc_mode to %d,atc_temper to %d",dyn_info.atc.mode,dyn_info.atc.temper);
            sea_updatesysinfo();
        }
    }
}

void pwr_tick()
{
    static u8 last_pwr_ctrlmode = 0xff;
    static u8 last_pwr_autostate = 0xff;
    static u8 last_pwr_netstate = 0xff;
    static u8 pwr_statelast = 0xff; //跟初始化一样
    
    if(last_pwr_ctrlmode != pwr_ctrlmode ||
       last_pwr_autostate != pwr_autostate ||
       last_pwr_netstate != pwr_netstate)
    {
        last_pwr_ctrlmode = pwr_ctrlmode;
        last_pwr_autostate = pwr_autostate;
        last_pwr_netstate = pwr_netstate;
        
        
    
        sea_printf("\n[PWR]current pwr_mode = %d",pwr_ctrlmode);
        sea_printf("\n[PWR]current auto = %d,net = %d",pwr_autostate,pwr_netstate);
        if(pwr_ctrlmode == 0)        // auto
        {
            if(pwr_autostate <= 1)
                pwr_state = pwr_autostate;
        }
        else if(pwr_ctrlmode == 1)   // network
        {
            if(pwr_netstate <= 1)
                pwr_state = pwr_netstate;
        }
        else
        {
            sea_printf("\n[PWR]unknow pwr_ctrlmode");
        }
        
        sea_printf("\n[PWR]pwr_state = %d,last_state = %d",pwr_state,pwr_statelast);
        
        if(pwr_statelast != pwr_state)
        {
            pwr_statelast = pwr_state;
            
            if(pwr_state == 0)
            {
                sea_printf("\n[PWR]power off");
                GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
            }
            else if(pwr_state == 1)
            {
                sea_printf("\n[PWR]power on");
                GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
            }
            else
            {
                sea_printf("\n[PWR]pwr_state error");
            }
        }
    }
}

void getPIR()
{
    u8 PIR1,PIR2,PIR3;
    
    static u32 PIR1Low = 0;
    static u32 PIR1High = 0;
    static u32 PIR2Low = 0;
    static u32 PIR2High = 0;
    static u32 PIR3Low = 0;
    static u32 PIR3High = 0;
    
    static u8 PIR1Status_now = PIR_UNKNOW;         // 0-low 1-high 2-pwm 3-unknow
    static u8 PIR1Status_last = PIR_UNKNOW;
    static u8 PIR2Status_now = PIR_UNKNOW;        
    static u8 PIR2Status_last = PIR_UNKNOW;
    static u8 PIR3Status_now = PIR_UNKNOW;         
    static u8 PIR3Status_last = PIR_UNKNOW;
    
    
    static u32 PIR_counter = 0;
    
    
    PIR_counter++;    // 1~60000 
    
    
    //PIR1
      //初始化延时10s
      //检测变化
    PIR1 = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11);   
    if(PIR1 > 0)
        PIR1High++;
    else 
        PIR1Low++;
    
    //PIR2
    PIR2 = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12);
    if(PIR2 > 0)
        PIR2High++;
    else 
        PIR2Low++;
    
    //PIR3
    PIR3 = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13);
    if(PIR3 > 0)
        PIR3High++;
    else 
        PIR3Low++;
    
    
    if(PIR_counter >= 40000)  //大约1s
    {
        PIR_counter = 0;
        
#ifdef ARC_DEBUG_PRINTF
        sea_printf("\nPIR3High = %d,PIR3Low = %d",PIR3High,PIR3Low);
#endif
        
        PIR1Status_last = PIR1Status_now;   //last status
        PIR2Status_last = PIR2Status_now;
        PIR3Status_last = PIR3Status_now;
        
        if(PIR1High > PIR1Low)              //now status
        {
            if(PIR1Low > 200)          // 60000/30   占空比30：1
                PIR1Status_now = PIR_PWM;
            else
                PIR1Status_now = PIR_HIGH;
        }
        else
        {
            if(PIR1High > 200)
                PIR1Status_now = PIR_PWM;
            else
                PIR1Status_now = PIR_LOW;
        }
        
        if(PIR2High > PIR2Low)              
        {
            if(PIR2Low > 200)       
                PIR2Status_now = PIR_PWM;
            else
                PIR2Status_now = PIR_HIGH;
        }
        else
        {
            if(PIR2High > 200)
                PIR2Status_now = PIR_PWM;
            else
                PIR2Status_now = PIR_LOW;
        }
        
        if(PIR3High > PIR3Low)              
        {
            if(PIR3Low > 200)         
                PIR3Status_now = PIR_PWM;
            else
                PIR3Status_now = PIR_HIGH;
        }
        else
        {
            if(PIR3High > 200)
                PIR3Status_now = PIR_PWM;
            else
                PIR3Status_now = PIR_LOW;
        }
        
        if(PIR1Status_last == PIR1Status_now)
        {
            if(PIR1Status_now == PIR_LOW)
                arc_cur.PIR1 = PIR_NONE;
            else if(PIR1Status_now == PIR_HIGH)
                arc_cur.PIR1 = PIR_MANIN;
            else if(PIR1Status_now == PIR_PWM)
                arc_cur.PIR1 = PIR_MANOUT;
        }
        
        if(PIR2Status_last == PIR2Status_now)
        {
            if(PIR2Status_now == PIR_LOW)
                arc_cur.PIR2 = PIR_NONE;
            else if(PIR2Status_now == PIR_HIGH)
                arc_cur.PIR2 = PIR_MANIN;
            else if(PIR2Status_now == PIR_PWM)
                arc_cur.PIR2 = PIR_MANOUT;
        }
        
        if(PIR3Status_last == PIR3Status_now)
        {
            if(PIR3Status_now == PIR_LOW)
                arc_cur.PIR3 = PIR_NONE;
            else if(PIR3Status_now == PIR_HIGH)
                arc_cur.PIR3 = PIR_MANIN;
            else if(PIR3Status_now == PIR_PWM)
                arc_cur.PIR3 = PIR_MANOUT;
        }
        
        PIR1Low = 0;
        PIR1High = 0;
        PIR2Low = 0;
        PIR2High = 0;
        PIR3Low = 0;
        PIR3High = 0;
    }
}
void getCur()
{
    u16 curValue;
    
    curValue = ADC_ConvertedValue();
#ifdef ARC_DEBUG_PRINTF
    sea_printf("\ncurValue = %d",curValue);
#endif
    
    arc_cur.cur = curValue;
    
    
    arc_cur.cmdcount = ctrlcmd_NRTnum;
    arc_cur.cmd_open = ctrlcmd_NRT.open;
    arc_cur.lastexec_open = lastexec_arc_ctrl.open;
}




void Send_ctrlCmd(arc_ctrl_t ctrlcmd)
{  
    //确保参数一定可靠
    if(ctrlcmd.temper >= 16 && ctrlcmd.temper <= 30
       && ctrlcmd.windlevel >= 1 && ctrlcmd.windlevel <= 4
       && ctrlcmd.mode >= 1 && ctrlcmd.mode <= 5)                     //检查命令参数是否有效
    {}
    else 
    {
        sea_printf("\n[ARCCMD]temper,wind,mode is out of range");
        ARC_Report_Para(ctrlcmd.temper,ctrlcmd.windlevel,ctrlcmd.mode); //parameter error
        return;
    }
  
    if(ctrlcmd.ctrlmode == ARC_CTRL_HD || ctrlcmd.ctrlmode == ARC_CTRL_AUTO)
    {
        ctrlcmd_NRTnum = 0;
        ctrlcmd_RTnum = 1;
        
        ctrlcmd_RT.airid = ctrlcmd.airid;
        ctrlcmd_RT.ctrlmode = ctrlcmd.ctrlmode;
        ctrlcmd_RT.open = ctrlcmd.open;
        ctrlcmd_RT.mode = ctrlcmd.mode;
        ctrlcmd_RT.temper = ctrlcmd.temper;
        ctrlcmd_RT.windlevel = ctrlcmd.windlevel;
        
        sea_printf("\n[ARCCMD]HD AUTO cmd, open = %d",ctrlcmd.open);
    }
    
    if(ctrlcmd.ctrlmode == ARC_CTRL_NETWORK)
    {
        ctrlcmd_NRTnum = 1;
        ctrlcmd_NRT.airid = ctrlcmd.airid;
        ctrlcmd_NRT.ctrlmode = ctrlcmd.ctrlmode;
        ctrlcmd_NRT.open = ctrlcmd.open;
        ctrlcmd_NRT.mode = ctrlcmd.mode;
        ctrlcmd_NRT.temper = ctrlcmd.temper;
        ctrlcmd_NRT.windlevel = ctrlcmd.windlevel;
        
        sea_printf("\n[ARCCMD]NET cmd, open = %d",ctrlcmd.open);
    }
}

void atc_tick()
{
    u8 pir_stateall = PIR_NONE;
    static u8 pir_stateall_last = PIR_MANOUT;
    
    static u16 atc_peoin_delay = 0; 
    
    static u8 pir_statefn = PIR_NONE;// 0 无人 1有人 2不工作
    static u8 pir_statefn_last = PIR_NONE;
    
    static u16 pir_statefn0_sec = 0;
    static u16 pir_statefn1_sec = 0;
    static u16 pir_statefn2_sec = 0;
    
    static u16 mcur_count = 0;
    static u16 valid_mcur_count = 0;    
    
    if(arc_cur.PIR1 == PIR_MANIN || arc_cur.PIR2 == PIR_MANIN || arc_cur.PIR3 == PIR_MANIN)
    {
        pir_stateall = PIR_MANIN;
    }
    else if(arc_cur.PIR1 == PIR_MANOUT || arc_cur.PIR2 == PIR_MANOUT || arc_cur.PIR3 == PIR_MANOUT)
    {
        pir_stateall = PIR_MANOUT;
    }
    else
    {
        pir_stateall = PIR_NONE;
    }
    
    
    
#ifdef ARC_DEBUG_PRINTF
    sea_printf("\npirs = %d",pir_stateall);
#endif
    
    switch(pir_stateall) //输入pir_stateall 0 1 2 3 输出pir_statefn 0 1 2
    {  
        case PIR_MANOUT:      
        {
            if(atc_peoin_delay > 0)
            {
                atc_peoin_delay --;
            }
            else
            {
                pir_statefn = PIR_MANOUT;
            }
            
            break;
        }
        case PIR_MANIN:      
        {   
            pir_statefn = PIR_MANIN;
            atc_peoin_delay = atc_closetime;    //再次进入有效
                
            break;
        }
        case PIR_NONE:      
        {
            if(atc_peoin_delay > 0)
            {
                atc_peoin_delay --;
            }
            else
            {
                pir_statefn = PIR_NONE;        
            }
              
            break;
        }
        default:
        {
            sea_printf("\ngoto default");
            break;
        }
    }
    
    arc_cur.roomstatus = pir_statefn;
    
    if(pir_statefn_last != pir_statefn)
    {
        pir_statefn_last = pir_statefn;
        pir_statefn0_sec = 0;
        pir_statefn1_sec = 0;
        pir_statefn2_sec = 0;
        
        sea_printf("\n[ROOM]pir_statefn change to %d",pir_statefn);
    }
    
    switch(pir_statefn)
    {
        case PIR_MANOUT:  //无人
        {
            if(pir_statefn0_sec < 10000)  //pir_statefn0_sec 无人控制的核心
                pir_statefn0_sec ++;
               
            if(pir_statefn0_sec == 1)//无人控制的初始化，只做一次
            {
                sea_printf("\npeople out of room,initial");
                
                atc_ctrlcmd1.open = KEEP_AIR;
                
                mcur_count = 0;     //检测连续
                valid_mcur_count = 0;
            }
            else if(pir_statefn0_sec == 2 || pir_statefn0_sec == 3 || pir_statefn0_sec == 4)//部分初始化，
            {
                sea_printf("\n[ATC-NOONE]wait 3s");

                mcur_count = 0;     
                valid_mcur_count = 0;
            }
            else if(pir_statefn0_sec >= 7) //前5秒不工作,开始下一次检测"未关状态"
            {
                sea_printf("\n[ATC-NOONE]mcur_count %d,valid_mcur_count %d",mcur_count,valid_mcur_count);
                if(arc_cur.cur > 100)
                {
                    if(mcur_count < 200)
                        mcur_count ++;
                }
                else
                {
                    mcur_count = 0;
                }
                
                if(mcur_count >= 7)
                {
                    if(valid_mcur_count < 500)
                        valid_mcur_count ++;
                }
                
                if(valid_mcur_count > 180 || air_status == AIR_STATUS_OPEN)
                {
                    pir_statefn0_sec = 1; //初始化
                    mcur_count = 0;     
                    valid_mcur_count = 0;
                    
                    arc_ctrl_t autoctrl;
                    autoctrl.airid = atc_ctrlcmd1.airid;
                    autoctrl.ctrlmode = ARC_CTRL_AUTO;
                    autoctrl.open = CLOSE_AIR;
                    autoctrl.mode = atc_ctrlcmd1.mode;
                    autoctrl.temper = atc_ctrlcmd1.temper;
                    autoctrl.windlevel = atc_ctrlcmd1.windlevel;
                        
                    sea_printf("\nsend ctrlcmd,close");
                    
                    Send_ctrlCmd(autoctrl);
                    
                    
                    atc_ctrlcmd1.open = OPEN_AIR;  //KEEP_AIR OPEN_AIR之间切换
                }
            }
            break;
        }
        case PIR_MANIN:  //有人
        {
            if(pir_statefn1_sec < 10000)           
                pir_statefn1_sec ++;
            
            if(pir_statefn1_sec == 1)             //有人之后 以最快的速度发一条命令 否则不发
            {
                sea_printf("\npeople go in to room");
                
                if(air_status == AIR_STATUS_CLOSE)  //CLOSE 或 UNKNOW 执行 OPEN 不执行
                {
                    arc_ctrl_t autoctrl;
                    autoctrl.airid = atc_ctrlcmd1.airid;
                    autoctrl.ctrlmode = ARC_CTRL_AUTO;
                    autoctrl.open = atc_ctrlcmd1.open;
                    autoctrl.mode = atc_ctrlcmd1.mode;
                    autoctrl.temper = atc_ctrlcmd1.temper;
                    autoctrl.windlevel = atc_ctrlcmd1.windlevel;
                    
                    sea_printf("\nsend ctrlcmd,open = %d",atc_ctrlcmd1.open);
                    
                    Send_ctrlCmd(autoctrl);
                }
                else if(air_status == AIR_STATUS_UNKNOW)   //开关是不同命令
                {
                    arc_ctrl_t autoctrl;
                    autoctrl.airid = atc_ctrlcmd1.airid;
                    autoctrl.ctrlmode = ARC_CTRL_AUTO;
                    autoctrl.open = atc_ctrlcmd1.open;
                    autoctrl.mode = atc_ctrlcmd1.mode;
                    autoctrl.temper = atc_ctrlcmd1.temper;
                    autoctrl.windlevel = atc_ctrlcmd1.windlevel;
                    
                    sea_printf("\nsend ctrlcmd,open = %d",atc_ctrlcmd1.open);
                    
                    Send_ctrlCmd(autoctrl);
                }
                
                atc_ctrlcmd1.open = KEEP_AIR;  //wkl 打上这个标签表示有待验证
            }
            
            break;
        }
        case PIR_NONE:  //未工作
        {
            if(pir_statefn2_sec < 10000)
                pir_statefn2_sec ++;   
            break;
        }
    }
    
    //监视区
    arc_cur.mcur_count = mcur_count;
    arc_cur.valid_mcur_count = valid_mcur_count;
    arc_cur.ctrlcmd1open = atc_ctrlcmd1.open;
    
    if(pir_stateall_last != pir_stateall)
    {
        pir_stateall_last = pir_stateall;
        pir_store[pir_store_in].hour = sst_time.hour;
        pir_store[pir_store_in].min = sst_time.min;
        pir_store[pir_store_in].sec = sst_time.sec;
        pir_store[pir_store_in].pir_stateall = pir_stateall;
        pir_store_in = (pir_store_in + 1)%4;
    }
    pir_store[4].hour = sst_time.hour;
    pir_store[4].min = sst_time.min;
    pir_store[4].sec = sst_time.sec;
    pir_store[4].pir_stateall = pir_stateall;
}


void ARC_getAirState(void) //严格检测状态
{
    static u16 H_counter = 0;
    static u16 L_counter = 0;
    
    
    
    if(air_detect_restart > 0)      //reset and delay
    {
        air_detect_restart --;
        H_counter = 0;
        L_counter = 0;
        return;
    }
    
      
    if(arc_cur.cur > 250)     //统计连续的高电流、低电流个数
    {
        if(H_counter < 10000)
            H_counter ++;
        L_counter = 0;
    }
    else if(arc_cur.cur < 100)   //change 100 
    {
        if(L_counter < 10000)
            L_counter ++;
        H_counter = 0;
    }
    else
    {
        H_counter = 0;
        L_counter = 0;
    }
      
    if(H_counter >= 3)
    {
        air_status = AIR_STATUS_OPEN;
    }
    else if(L_counter >= 180)    // if 3min is enough to distinguish closemode and sleepmode 可以考虑改成240 300
    {
        air_status = AIR_STATUS_CLOSE;
    }
    else
    {
        air_status = AIR_STATUS_UNKNOW;
    }
    
    //sea_printf("\nL_counter = %d,H_counter = %d",L_counter,H_counter);
    arc_cur.Hcur = H_counter;
    arc_cur.Lcur = L_counter;
    arc_cur.airstatus = air_status;
    
}



void ARC_cmdExec(void)  //包括HD命令执行 NET_AUTO命令执行 命令校准三部份 每次只执行一部份
{
#ifdef ARC_DEBUG_PRINTF
    sea_printf("\nair_status = %d",air_status);
#endif
    
    if(ctrlcmd_RTnum != 0)  //实时控制命令
    {
        ctrlcmd_RTnum = 0;   //取命令
     
        if(ctrlcmd_RT.open == SWITCH_AIR)   //this is a switch，以后不用
        { 
            remote_openair(ctrlcmd_RT.temper,ctrlcmd_RT.windlevel,ctrlcmd_RT.mode);     //保证3个参数正确
            air_status = AIR_STATUS_UNKNOW;     //清空空调状态
            air_detect_restart = 2;
        }
        else if(ctrlcmd_RT.open == OPEN_AIR)
        {
            remote_openair(ctrlcmd_RT.temper,ctrlcmd_RT.windlevel,ctrlcmd_RT.mode);     //保证3个参数正确
            air_status = AIR_STATUS_UNKNOW;     //清空空调状态
            air_detect_restart = 2;
        }
        else if(ctrlcmd_RT.open == CLOSE_AIR)
        {
            remote_closeair(ctrlcmd_RT.temper,ctrlcmd_RT.windlevel,ctrlcmd_RT.mode);     //保证3个参数正确
            air_status = AIR_STATUS_UNKNOW;     //清空空调状态
            air_detect_restart = 2;
        }
        else if(ctrlcmd_RT.open == CHANGE_AIR)
        {
            remote_changeair(ctrlcmd_RT.temper,ctrlcmd_RT.windlevel,ctrlcmd_RT.mode); 
            air_status = AIR_STATUS_UNKNOW;     //清空空调状态
            air_detect_restart = 2;
        }
        else if(ctrlcmd_RT.open == KEEP_AIR)
        {
        }
        else
        {
            //上报未知命令 命令执行失败
            return;
        }
        
        lastexec_arc_ctrl.airid = ctrlcmd_RT.airid;      //保留执行状态
        lastexec_arc_ctrl.ctrlmode = ctrlcmd_RT.ctrlmode;
        lastexec_arc_ctrl.open = ctrlcmd_RT.open;
        lastexec_arc_ctrl.mode = ctrlcmd_RT.mode;
        lastexec_arc_ctrl.temper = ctrlcmd_RT.temper;
        lastexec_arc_ctrl.windlevel = ctrlcmd_RT.windlevel;
        
    }
    else if(ctrlcmd_NRTnum != 0)
    {
        if(air_status == AIR_STATUS_OPEN || air_status == AIR_STATUS_CLOSE)
        {
            ctrlcmd_NRTnum = 0;    //取命令
            
            if(ctrlcmd_NRT.open == OPEN_AIR && air_status == AIR_STATUS_OPEN)
            {
                //remote_changeair(ctrlcmd_NRT.temper,ctrlcmd_NRT.windlevel,ctrlcmd_NRT.mode);
                //air_status = AIR_STATUS_UNKNOW;
                //air_detect_restart = 2;
                
                if(ctrlcmd_NRT.temper == lastexec_arc_ctrl.temper 
                   && ctrlcmd_NRT.windlevel == lastexec_arc_ctrl.windlevel
                   && ctrlcmd_NRT.mode == lastexec_arc_ctrl.mode)
                {
                      
                }
                else
                {
                    remote_changeair(ctrlcmd_NRT.temper,ctrlcmd_NRT.windlevel,ctrlcmd_NRT.mode);
                    air_status = AIR_STATUS_UNKNOW;
                    air_detect_restart = 2;
                }
            }
            else if(ctrlcmd_NRT.open == OPEN_AIR && air_status == AIR_STATUS_CLOSE)
            {
                remote_openair(ctrlcmd_NRT.temper,ctrlcmd_NRT.windlevel,ctrlcmd_NRT.mode); 
                air_status = AIR_STATUS_UNKNOW;
                air_detect_restart = 2;
             }
            else if(ctrlcmd_NRT.open == CLOSE_AIR && air_status == AIR_STATUS_OPEN)
            {
                remote_closeair(ctrlcmd_NRT.temper,ctrlcmd_NRT.windlevel,ctrlcmd_NRT.mode);
                air_status = AIR_STATUS_UNKNOW;
                air_detect_restart = 2;
            }
            else if(ctrlcmd_NRT.open == CLOSE_AIR && air_status == AIR_STATUS_CLOSE)
            {
                // 
            }
            else if(ctrlcmd_NRT.open == SWITCH_AIR)
            {
                remote_openair(ctrlcmd_NRT.temper,ctrlcmd_NRT.windlevel,ctrlcmd_NRT.mode); 
                air_status = AIR_STATUS_UNKNOW;
                air_detect_restart = 2;
            }
            else if(ctrlcmd_NRT.open == KEEP_AIR)
            {
                //return;
            }
            else 
            {
                //上报未知命令
                return;
            }
            lastexec_arc_ctrl.airid = ctrlcmd_NRT.airid;
            lastexec_arc_ctrl.ctrlmode = ctrlcmd_NRT.ctrlmode;
            lastexec_arc_ctrl.open = ctrlcmd_NRT.open;
            lastexec_arc_ctrl.mode = ctrlcmd_NRT.mode;
            lastexec_arc_ctrl.temper = ctrlcmd_NRT.temper;
            lastexec_arc_ctrl.windlevel = ctrlcmd_NRT.windlevel;  
        }
    }
    else
    {
        /*if(lastexec_arc_ctrl.ctrlmode != ARC_CTRL_HD)   //HD 命令不校准
        {
            if(lastexec_arc_ctrl.open == OPEN_AIR && air_status == AIR_STATUS_CLOSE)
            { //可以加delay         
                remote_openair(lastexec_arc_ctrl.temper,lastexec_arc_ctrl.windlevel,lastexec_arc_ctrl.mode); 
                air_status = AIR_STATUS_UNKNOW;
                air_detect_restart = 2;
            }
            if(lastexec_arc_ctrl.open == CLOSE_AIR && air_status == AIR_STATUS_OPEN)
            {
                remote_closeair(lastexec_arc_ctrl.temper,lastexec_arc_ctrl.windlevel,lastexec_arc_ctrl.mode);
                air_status = AIR_STATUS_UNKNOW;
                air_detect_restart = 2;
            }
        }*/
    }
    
}



void ARC_Tick(void)
{
    static volatile long time_count = 0;
    
    static u16 temp_sec = 0;
    //static u16 report_sec = 0;
    
    time_count ++;
    ARC_sec = sst_time.sec;
    if(time_count >= 100000)
    {
        time_count = 0;
        temp_sec ++;
        //report_sec ++;
    }

  
#ifdef ARC_CD
    static volatile long count = 0;
    
    count++;
    if(count >= 80000)
    {   
        //sea_printf("ARC_Tick");
        count = 0;
        
    }  
    
    
#else
    
    
    sst_tick();
    pwr_tick();
    tm_tick();
    
    getPIR();   //loop 算占空比
    
    
    
    static u32 ARC_sec_last = 0;
    
    if(ARC_sec_last != ARC_sec)   //根据模拟时间 大致为1s
    {
        ARC_sec_last = ARC_sec;
        getCur();              // 1s
        ARC_getAirState();     // 1s
        
        atc_tick();     //根据热释电产生自动命令,独立模块  1s
        ARC_cmdExec();         //执行空调开关命令, 1s
        //outputtest3();
        
        if(ARC_sec % 3 == 0 )
        {
            wifi_ReportFrame(ARC_REPORT_CURINFO, sizeof(arc_cur_t), 0, (u8*)&arc_cur);
            FRAME_SendFrame(ARC_REPORT_CURINFO,sizeof(arc_cur_t),0,(u8*)&arc_cur);
        }   

        task_stackremain[0] = uxTaskGetStackHighWaterMark(NULL);
    }
    
#endif
}




//sst模块
sst_time_t sst_getsystime(void)
{
    return sst_time;
}

void sst_setsystime(psst_time_t psettime)
{
    sea_printf("\n[SST]set new time");
    sst_time.year = psettime->year;
    sst_time.mon = psettime->mon;
    sst_time.day = psettime->day;
    sst_time.hour = psettime->hour;
    sst_time.min = psettime->min;
    sst_time.sec = psettime->sec;
    
    sst_time.realmin = sst_time.hour * 60 + sst_time.min;
    
    sst_state = 1;
}

void sst_tick()  //loop调用
{   
    static u8 last_sec = 255;
    static u8 last_mon = 255;
    
    static u16 last_realmin = 255;
    static u16 last_opentime = 1440;
    static u16 last_lasttime = 1440;

    static u16 timelack_count = 60 - 10;  // 10s wait for w108 join in network

    static u8 last_hour = 255;

    //pwr autostate tick
    if(sst_state == 1)  //time is valuable
    {
        //realmin pwr
        if(last_realmin != sst_time.realmin ||
           last_opentime != pwr_opentime || 
           last_lasttime != pwr_lasttime)   // 1 min 1 call
        {
            last_realmin = sst_time.realmin; 
            last_opentime = pwr_opentime;
            last_lasttime = pwr_lasttime;
                
            //向电流输出
            u16 pastmins;   //相对于 pwr_opentime
            
            if(last_realmin >= pwr_opentime)
            {
                pastmins = last_realmin - pwr_opentime;
            }
            else
            {
                pastmins = last_realmin + 1440 - pwr_opentime;
            }
            
            sea_printf("\n[SST_PWR]nowmins = %d,pwr_opentime = %d",last_realmin,pwr_opentime);
            sea_printf("\n[SST_PWR]pastmins = %d,pwr_lasttime = %d",pastmins,pwr_lasttime);

            if(pastmins < pwr_lasttime)
            {
                pwr_autostate = 1;
                //pwr_update();
                //开
            }
            else
            {
                pwr_autostate = 0;
                //pwr_update();
                //关
            }
        }
        
        //tm
        if(last_mon != sst_time.mon || tm_tmchange != 0)    
        {
            last_mon = sst_time.mon;
            tm_tmchange = 0;      
            
            if(last_mon >= 1 && last_mon <= 12)
            {
                tm_automode = tm_modedata[last_mon - 1];
                tm_autotemper = tm_tempdata[last_mon - 1];
                sea_printf("\n[SST_TM]new mon %d,tm_automode %d,tm_autotemper %d",last_mon,
                           tm_automode,tm_autotemper);
            }
        } 
        
        //每日初始化模块
        if(last_hour != sst_time.hour)
        {
            last_hour = sst_time.hour;
            if(last_hour == 0)
            {
                atc_ctrlcmd1.open = KEEP_AIR; 
                sea_printf("[SST]every day clear atc_cmd1.open %d",atc_ctrlcmd1.open );
            }
        }
    }
    if(sst_state == 0)
    {
        if(last_sec != sst_time.sec)   //1 sec 1 call
        {
            last_sec = sst_time.sec;
            timelack_count ++;
            
            if(timelack_count > sst_lackreporttime) // sst_lackreporttime
            {
                timelack_count = 0;
                sea_printf("\nsend ARC_REQUEST_TIME");
                wifi_ReportFrame(ARC_REQUEST_TIME,2,0,(u8*)&arc_cur.airid);
                FRAME_SendFrame(ARC_REQUEST_TIME,2,0,(u8*)&arc_cur.airid);
                
            }
        }   
    }
}


