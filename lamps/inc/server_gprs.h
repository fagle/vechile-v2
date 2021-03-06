#ifndef __SERVER_GPRS_H__
#define __SERVER_GPRS_H__

#include "config.h"

#ifdef SEND_VIA_GPRS
////////////////////////////////////////////////////////////////////////
//constants and globals




////////////////////////////////////////////////////////////////////////
// public functions

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_SendKeyRptViaGprs(void)
//* 功能        : 通过GPRS模块，向webserver报告Key Rpt
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_SendKeyRptViaGprs(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_SendFulRptViaGprs(void)
//* 功能        : 通过GPRS模块，向webserver报告Ful Rpt
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_SendFulRptViaGprs(void);



#endif  //SEND_VIA_GPRS
////////////////////////////////////////////////////////////////////////
#endif // __SERVER_GPRS_H__