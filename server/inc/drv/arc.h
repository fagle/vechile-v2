
#ifndef __ARC_H__
#define __ARC_H__

///////////////////////////////////////////////////////////////////////////////////
//
#include "config.h" 
#include "network.h"

typedef enum
{
    ARC_CTRL_AUTO = 0x00,
    ARC_CTRL_NETWORK,
    ARC_CTRL_KEY,
    ARC_CTRL_HD,
    ARC_CTRL_UNKNOW,
}ARCCtrlMode_TypeDef;

typedef enum
{
    PIR_LOW = 0x00,
    PIR_HIGH,
    PIR_PWM,
    PIR_UNKNOW,
}PIRVolt_TypeDef;

typedef enum
{
    AIR_STATUS_CLOSE = 0x00,
    AIR_STATUS_OPEN,
    AIR_STATUS_UNKNOW,
}AIRStatus_TypeDef;

typedef enum
{
    CLOSE_AIR = 0x00,
    OPEN_AIR,
    SWITCH_AIR,
    CHANGE_AIR,
    KEEP_AIR,
    UNKNOW,
}AIROpen_TypeDef;

typedef enum
{ 
    PIR_MANOUT = 0x00,
    PIR_MANIN,
    PIR_NONE,
}PIRStatus_TypeDef;

typedef enum
{
    ARC_MODE_ERROR = 0xf0,
    ARC_CUR_ERROR,
    ARC_REMOTE_ERROR,
    ARC_STATUES_ERROR,
    ARC_PARA_ERROR,
}ARCERROR_TypeDef;



typedef struct
{
    u16 airid;
    u8 ctrlmode;
    u8 open;
    u8 mode;
    u8 temper;
    u8 windlevel;
}arc_ctrl_t, *parc_ctrl_t;


typedef struct  //sst模块
{
    u16  year;
    u8   mon;
    u8   day;
    u8   hour;
    u8   min;
    u8   sec;
    u16  realmin;    // realmin = hour * 60 + min;
    u32  tick;
} sst_time_t, *psst_time_t;


extern arc_ctrl_t     coord_arc_ctrl;

extern arc_ctrl_t     atc_ctrlcmd1;
extern u16 atc_closetime;

extern u8 air_status;
extern u8 air_detect_restart;
extern u8 ctrlcmd_NRTnum;


//测试用
typedef struct      
{
    u16 airid;         //初始化配置
    s8 temper;
    u8 lumin;
    u8 humi;
    
    u16 cur;
    u16 Hcur;
    u16 Lcur;
    u8 airstatus;
    
    u8 PIR1;
    u8 PIR2;
    u8 PIR3;
    u8 roomstatus;
    u16 mcur_count;
    u16 valid_mcur_count;
    
    u8 cmdcount;
    u8 cmd_open;
    u8 lastexec_open;
    u8 ctrlcmd1open;
    
}arc_cur_t,*parc_cur_t;


typedef struct
{
    u8 hour;
    u8 min;
    u8 sec;
    u8 pir_stateall;
}pir_store_t, *ppir_store_t;


extern arc_cur_t    arc_cur;

extern u8 sst_state;   //初始化为不可用
extern sst_time_t sst_time;


extern u16 pwr_opentime;
extern u16 pwr_lasttime;

extern u8 pwr_ctrlmode;
extern u8 pwr_autostate; 
extern u8 pwr_netstate;
extern u8 pwr_state;

extern u8 tm_tempdata[12];
extern u8 tm_modedata[12];


extern u8 tm_tmchange;
extern u8 tm_ctrlmode;
extern u8 tm_autotemper;
extern u8 tm_automode;
extern u8 tm_nettemper;
extern u8 tm_netmode;
extern u8 arc_state[26];

extern u8 readtag; 

extern u32 nowtick1; //for start
extern u32 nowtick2; //for first data 0
extern u32 nowtick3;

extern u16 task_stackremain[6];

extern pir_store_t pir_store[5];
extern u8 pir_store_in;



void Send_ctrlCmd(arc_ctrl_t ctrlcmd);
sst_time_t sst_getsystime(void);
void sst_setsystime(psst_time_t psettime);
void sst_tick();
void ARC_Tick();

///////////////////////////////////////////////////////////////////////////////////////
#endif  // __ARC_H__