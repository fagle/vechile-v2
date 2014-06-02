//
// communication.h
// define the sink and sensor communication function
// @Author:Íõ½¨»ª
// copyright 2013 by HDU Embeded Lab.
// All rights reserved.
//

#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

/*include*/
#include "stm32f10x_type.h"
#include "ember-types.h"
#include "lamp.h"
#include "network.h"

/*define*/
#define MAXSENSORSENDTIME 8 /*s*/

/*extern*/
extern single_t single_info;



//
// sensorSinkCOmTick
// MAXSENSORSENDTIME second tick,sensor and sink communication 
// if it fails to send ,then MAXSENSORSENDTIME / 2
//
void sensorSinkCOMTick(void);

//
// sensorSinkCOMSuccess
// sensor and sink communicate success
// @param status : the send status
// @return void
//
void sensorSinkCOMSuccess(EmberStatus status);

#endif //__COMMUNICATION_H

