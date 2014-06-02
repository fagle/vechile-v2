#ifndef __ARC_H__
#define __ARC_H__

////////////////////////////////////////////////////////////////////////////////////
//
#include "config.h" 


#define ARC_CTRL_AUTO                  0x00
#define ARC_CTRL_NETWORK               0x01     
#define ARC_CTRL_KEY                   0x02

typedef struct
{
    u16 airid;         //≥ı ºªØ≈‰÷√
    u8 temper;
    u8 lumin;
    u8 cur;
    u8 redsensor1;
    u8 redsensor2;
    u8 redsensor3;
  
}arc_sensorvalue_t, *parc_sensorvalue_t;

typedef struct
{
    u16 airid;
    u8 ctrlmode;
    u8 open;
    u8 mode;
    u8 temper;
    u8 windlevel;
}arc_ctrl_t, *parc_ctrl_t;


extern arc_sensorvalue_t arc_sensorvalue;
extern arc_ctrl_t     arc_ctrl;

////////////////////////////////////////////////////////////////////////////////////
//
#endif   // __ARC_H__
