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
//* ������      : void sea_SendKeyRptViaGprs(void)
//* ����        : ͨ��GPRSģ�飬��webserver����Key Rpt
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_SendKeyRptViaGprs(void);

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_SendFulRptViaGprs(void)
//* ����        : ͨ��GPRSģ�飬��webserver����Ful Rpt
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_SendFulRptViaGprs(void);



#endif  //SEND_VIA_GPRS
////////////////////////////////////////////////////////////////////////
#endif // __SERVER_GPRS_H__